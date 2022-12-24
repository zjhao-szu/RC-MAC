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

#ifndef INCLUDED_RC_MAC_RECEIVETOKEN_IMPL_H
#define INCLUDED_RC_MAC_RECEIVETOKEN_IMPL_H

#include <RC_MAC/ReceiveToken.h>
#include <vector>
#include <string>
#include <volk/volk.h>
#include <gnuradio/fft/fft.h>
#include <queue>
#include <complex>
#include <cmath>
#include <cstdlib>
#include "utilities.h"

namespace gr {
  namespace RC_MAC {

    class ReceiveToken_impl : public ReceiveToken
    {
     private:
      uint8_t m_sf;
      uint32_t m_samp_rate;
      uint32_t m_bw;

      uint32_t m_number_of_bins;
      uint32_t m_fft_size;
      uint32_t m_samples_per_symbol;
      std::vector<uint16_t> m_sync_words;    

      int m_oversample_rate;
      int m_up;
      int premab_symb_cnt;
  
      // variable used to perform the FFT demodulation
      std::vector<gr_complex> m_upchirp;   ///< Reference upchirp
      std::vector<gr_complex> m_downchirp; ///< Reference downchirp
      std::vector<gr_complex> m_dechirped; ///< Dechirped symbol
      std::vector<gr_complex> m_fftVector;       ///< Result of the FFT
      std::vector<float>      m_upchirp_ifreq; 
      std::vector<float>      m_downchirp_ifreq;

      uint32_t m_result;
      pmt::pmt_t m_outPort;
      RECEIVE_TOKEN m_state;

      fft::fft_complex   *m_fft;
      std::vector<float> m_window;
      float              m_beta;
      int m_corr_fails;

      uint32_t search_fft_peak(const lv_32fc_t *fft_result,float *buffer1,  float *max_val_p);
      uint32_t argmax_32f(float *fft_result, float *max_val_p);


      void  instantaneous_frequency(const gr_complex * in_samples,float * out_freq,const uint32_t windowSize);
      void  instantaneous_phase(const gr_complex *in_samples, float *out_iphase, const uint32_t windowSize);
      float  cross_correlate_ifreq_fast(const float *samples_ifreq, const float *ideal_chirp, const uint32_t window);
      float  cross_correlate_fast(const gr_complex *samples, const gr_complex *ideal_chirp, const uint32_t window);
      float  cross_correlate(const gr_complex *samples_1, const gr_complex *samples_2, const uint32_t window);
      float  cross_correlate_ifreq(const float *samples_ifreq, const std::vector<float>& ideal_chirp, const uint32_t to_idx);

      float  detect_upchirp(const gr_complex *samples, const uint32_t window, int32_t *index);
      float  detect_downchirp(const gr_complex *samples, const uint32_t window);
      float  detect_preamble_autocorr(const gr_complex *samples, const uint32_t window);
      float  sliding_norm_cross_correlate_upchirp(const float *samples_ifreq, const uint32_t window, int32_t *index);
      float  stddev(const float * values, const uint32_t len,const float mean);

     public:
      ReceiveToken_impl(uint8_t  sf,uint32_t samp_rate,uint32_t bw);
      ~ReceiveToken_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

    };

  } // namespace RC_MAC
} // namespace gr

#endif /* INCLUDED_RC_MAC_RECEIVETOKEN_IMPL_H */

