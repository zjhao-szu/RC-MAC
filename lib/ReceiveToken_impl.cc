/* -*- c++ -*- */
/*
 * Copyright 2022 zjhao.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "ReceiveToken_impl.h"

namespace gr {
  namespace RC_MAC {

    ReceiveToken::sptr
    ReceiveToken::make(uint8_t  sf,uint32_t samp_rate,uint32_t bw)
    {
      return gnuradio::get_initial_sptr
        (new ReceiveToken_impl(sf,samp_rate,bw));
    }


    /*
     * The private constructor
     */
    ReceiveToken_impl::ReceiveToken_impl(uint8_t  sf,uint32_t samp_rate,uint32_t bw)
      : gr::block("ReceiveToken",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0)),
      m_sf(sf),
      m_samp_rate(samp_rate),
      m_bw(bw) 
    {
      m_number_of_bins = (uint32_t) (1u<<m_sf);
      m_oversample_rate = m_samp_rate / m_bw;
      m_samples_per_symbol = (uint32_t)(m_number_of_bins * m_oversample_rate);
      m_fft_size = m_samples_per_symbol;
      m_downchirp.resize(m_samples_per_symbol);
      m_downchirp_ifreq.resize(m_samples_per_symbol);
      m_upchirp.resize(m_samples_per_symbol);
      m_upchirp_ifreq.resize(m_samples_per_symbol);

      build_chirps(&m_upchirp[0],&m_downchirp[0],m_sf,m_oversample_rate);
      instantaneous_frequency(&m_downchirp[0], &m_downchirp_ifreq[0], m_samples_per_symbol);
      instantaneous_frequency(&m_upchirp[0],   &m_upchirp_ifreq[0],   m_samples_per_symbol);
     
      m_up = 8;
      premab_symb_cnt = 0;

      m_outPort = pmt::mp("outPort");
      message_port_register_out(m_outPort);
      // set_msg_handler(m_outPort, boost::bind(&CTSReceiveToken_impl::receiveRTS, this, _1));

      
      m_fft = new fft::fft_complex(m_fft_size, true, 1);
      m_corr_fails = 0;

    }

    /*
     * Our virtual destructor.
     */
    ReceiveToken_impl::~ReceiveToken_impl()
    {
    }

     void 
    ReceiveToken_impl::instantaneous_frequency(const gr_complex * in_samples,float * out_freq,const uint32_t windowSize){
      if (windowSize < 2u) {
          std::cerr << "[LoRa Decoder] WARNING : window size < 2 !" << std::endl;
          return;
      }

      /* instantaneous_phase */
      for (uint32_t i = 1u; i < windowSize; i++) {
          const float iphase_1 = std::arg(in_samples[i - 1]);
                float iphase_2 = std::arg(in_samples[i]);

          // Unwrapped loops from liquid_unwrap_phase
          while ( (iphase_2 - iphase_1) >  M_PI ) iphase_2 -= 2.0f*M_PI;
          while ( (iphase_2 - iphase_1) < -M_PI ) iphase_2 += 2.0f*M_PI;

          out_freq[i - 1] = iphase_2 - iphase_1;
      }

      // Make sure there is no strong gradient if this value is accessed by mistake
      out_freq[windowSize - 1] = out_freq[windowSize - 2];
    }
    
    void 
    ReceiveToken_impl::instantaneous_phase(const gr_complex *in_samples, float *out_iphase, const uint32_t windowSize){
      out_iphase[0] = std::arg(in_samples[0]);

      for (uint32_t i = 1u; i < windowSize; i++) {
          out_iphase[i] = std::arg(in_samples[i]);
          // = the same as atan2(imag(in_samples[i]),real(in_samples[i]));

          // Unwrapped loops from liquid_unwrap_phase
          while ( (out_iphase[i] - out_iphase[i-1]) >  M_PI ) out_iphase[i] -= 2.0f*M_PI;
          while ( (out_iphase[i] - out_iphase[i-1]) < -M_PI ) out_iphase[i] += 2.0f*M_PI;
      }
    }
    
    float
    ReceiveToken_impl::cross_correlate_ifreq_fast(const float *samples_ifreq, const float *ideal_chirp, const uint32_t window){
      float result = 0;
      volk_32f_x2_dot_prod_32f(&result, samples_ifreq, ideal_chirp, window);
      return result;
    } 
    
    float
    ReceiveToken_impl::cross_correlate_fast(const gr_complex *samples, const gr_complex *ideal_chirp, const uint32_t window){
      gr_complex result = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&result, samples, ideal_chirp, window);
      return abs(result);
    } 
    
    float
    ReceiveToken_impl::cross_correlate(const gr_complex *samples_1, const gr_complex *samples_2, const uint32_t window){
      float result = 0.0f;

      for (uint32_t i = 0u; i < window; i++) {
          result += std::real(samples_1[i] * std::conj(samples_2[i]));
      }

      result /= (float)window;

      return result;
    }
    
    float
    ReceiveToken_impl::cross_correlate_ifreq(const float *samples_ifreq, const std::vector<float>& ideal_chirp, const uint32_t to_idx){
      float result = 0.0f;

      const float average   = std::accumulate(samples_ifreq  , samples_ifreq + to_idx, 0.0f) / (float)(to_idx);
      const float chirp_avg = std::accumulate(&ideal_chirp[0], &ideal_chirp[to_idx]  , 0.0f) / (float)(to_idx);
      const float sd        =  stddev(samples_ifreq   , to_idx, average)
                              * stddev(&ideal_chirp[0] , to_idx, chirp_avg);

      for (uint32_t i = 0u; i < to_idx; i++) {
          result += (samples_ifreq[i] - average) * (ideal_chirp[i] - chirp_avg) / sd;
      }

      result /= (float)(to_idx);

      return result;
    }
    
    float 
    ReceiveToken_impl::stddev(const float *values, const uint32_t len, const float mean){
      float variance = 0.0f;

      for (uint32_t i = 0u; i < len; i++) {
          const float temp = values[i] - mean;
          variance += temp * temp;
      }

      variance /= (float)len;
      return std::sqrt(variance);
    }

    float
    ReceiveToken_impl::detect_upchirp(const gr_complex *samples, const uint32_t window, int32_t *index){
      float samples_ifreq[window*2];
      instantaneous_frequency(samples, samples_ifreq, window*2);

      return sliding_norm_cross_correlate_upchirp(samples_ifreq, window, index);
    };
    
    float
    ReceiveToken_impl::detect_downchirp(const gr_complex *samples, const uint32_t window){
      float samples_ifreq[window];
      instantaneous_frequency(samples, samples_ifreq, window);

      return cross_correlate_ifreq(samples_ifreq, m_downchirp_ifreq, window - 1u);
    }
    float 
    ReceiveToken_impl::detect_preamble_autocorr(const gr_complex *samples, const uint32_t window){
      const gr_complex* chirp1 = samples;
      const gr_complex* chirp2 = samples + m_samples_per_symbol;
      float magsq_chirp1[window];
      float magsq_chirp2[window];
      float energy_chirp1 = 0;
      float energy_chirp2 = 0;
      float autocorr = 0;
      gr_complex dot_product;

      volk_32fc_x2_conjugate_dot_prod_32fc(&dot_product, chirp1, chirp2, window);
      volk_32fc_magnitude_squared_32f(magsq_chirp1, chirp1, window);
      volk_32fc_magnitude_squared_32f(magsq_chirp2, chirp2, window);
      volk_32f_accumulator_s32f(&energy_chirp1, magsq_chirp1, window);
      volk_32f_accumulator_s32f(&energy_chirp2, magsq_chirp2, window);
      autocorr = abs(dot_product / gr_complex(sqrt(energy_chirp1 * energy_chirp2), 0));
      return autocorr;
    }
    float
    ReceiveToken_impl::sliding_norm_cross_correlate_upchirp(const float *samples_ifreq, const uint32_t window, int32_t *index){
      float max_correlation = 0;

      // Cross correlate
      for (uint32_t i = 0; i < window; i++) {
          const float max_corr = cross_correlate_ifreq_fast(samples_ifreq + i, &m_upchirp_ifreq[0], window - 1u);

          if (max_corr > max_correlation) {
              *index = i;
              max_correlation = max_corr;
          }
      }

      return max_correlation;
    }


    uint32_t
    ReceiveToken_impl::argmax_32f(float *fft_result, float *max_val_p)
    {
      float mag   = abs(fft_result[0]);
      float max_val = mag;
      uint32_t   max_idx = 0;

      for (uint32_t i = 0; i < m_fft_size; i++)
      {
        mag = abs(fft_result[i]);
        if (mag > max_val)
        {
          max_idx = i;
          max_val = mag;
        }
      }

      *max_val_p = max_val;
      return max_idx;
    }

    uint32_t
    ReceiveToken_impl::search_fft_peak(const lv_32fc_t *fft_result,float *buffer1, float *max_val_p){
      uint32_t max_idx = 0;
      *max_val_p = 0;
      
      // fft result magnitude summation
      volk_32fc_magnitude_32f(buffer1, fft_result, m_fft_size);

      // Take argmax of returned FFT (similar to MFSK demod)
      max_idx = argmax_32f(buffer1, max_val_p);
    
      return max_idx;
    }

    void
    ReceiveToken_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    ReceiveToken_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int num_consumed = 0;
      switch (m_state)
      {
        case STATE_TOKEN_DETECT_PREMBLE:{
          float correlation = detect_preamble_autocorr(in, m_samples_per_symbol);

          if (correlation >= 0.90f) {
              std::cout<<"detect Preamble"<<std::endl;
              m_state = STATE_TOKEN_SYNC;
              break;
          }
          num_consumed = m_samples_per_symbol;
          // consume_each(d_samples_per_symbol);

          /* code */
          break;
        }
        case STATE_TOKEN_SYNC:{
          int i = 0;
          detect_upchirp(in, m_samples_per_symbol, &i);

          // consume_each(i);
          num_consumed = i;
          m_state = STATE_TOKEN_FIND_SFD;
          break;
        }
        case STATE_TOKEN_FIND_SFD:{
          const float c = detect_downchirp(in, m_samples_per_symbol);

          if (c > 0.96f) {
            std::cout<<"detect Downchirp"<<std::endl;
            m_state = STATE_TOKEN_PAUSE;
          } else {
              if(c < -0.96f) { //检测到了upchirp
                m_corr_fails++;
              }

              if (m_corr_fails > 8u) {
                  m_state = STATE_TOKEN_DETECT_PREMBLE;
              }
          }
          num_consumed =  (int32_t)m_samples_per_symbol;
        
          break;
        }
        case STATE_TOKEN_PAUSE:{
          m_state = STATE_TOKEN_READ_TOKEN;
          num_consumed = m_samples_per_symbol + m_samples_per_symbol / 4;
          break;
        }
        case STATE_TOKEN_READ_TOKEN:{
          const float c = detect_downchirp(in, m_samples_per_symbol);
          if(c > 0.96f){
            std::cout<<"receive RTS/CTS packet!!!"<<std::endl;
            uint32_t max_idx        = 0;
            float    max_val        = 0;

            gr_complex *down_block = (gr_complex *)volk_malloc(m_fft_size*sizeof(gr_complex), volk_get_alignment());
            float *fft_res_mag = (float*)volk_malloc(m_fft_size*sizeof(float), volk_get_alignment());
          

            volk_32fc_x2_multiply_32fc(down_block, in, &m_upchirp[0], m_samples_per_symbol);

            memset(m_fft->get_inbuf(),            0, m_fft_size *sizeof(gr_complex));
            memcpy(m_fft->get_inbuf(), &down_block[0], m_samples_per_symbol*sizeof(gr_complex));
            m_fft->execute();

            max_idx = search_fft_peak(m_fft->get_outbuf(), fft_res_mag, &max_val);

            std::cout<<"downchirp token is: "<< max_idx << std::endl;

            pmt::pmt_t tokenMsg = pmt::from_uint64(max_idx);
            message_port_pub(m_outPort,tokenMsg);
            num_consumed = m_samples_per_symbol;
            volk_free(down_block);
            volk_free(fft_res_mag);
          }
          break;
        }
      
        default:
          break;
        }



      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace RC_MAC */
} /* namespace gr */

