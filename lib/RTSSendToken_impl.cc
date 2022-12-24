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
#include "RTSSendToken_impl.h"

namespace gr {
  namespace RC_MAC {

    RTSSendToken::sptr
    RTSSendToken::make(uint8_t sf, uint32_t samp_rate, uint32_t bw,uint32_t nodeToken)
    {
      return gnuradio::get_initial_sptr
        (new RTSSendToken_impl(sf,samp_rate,bw,nodeToken));
    }


    /*
     * The private constructor
     */
    RTSSendToken_impl::RTSSendToken_impl(uint8_t sf, uint32_t samp_rate, uint32_t bw,uint32_t nodeToken)
      : gr::block("RTSSendToken",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
      m_sf(sf),
      m_samp_rate(samp_rate),
      m_bw(bw),
      m_nodeToken(nodeToken)
    {
      m_number_of_bins = (uint32_t)(1u << m_sf);
      m_oversample_rate = m_samp_rate / m_bw;
      m_samples_per_symbol = (uint32_t)(m_number_of_bins * m_oversample_rate);

      m_downchirp.resize(m_samples_per_symbol);
      m_upchirp.resize(m_samples_per_symbol);

      build_chirps(&m_upchirp[0],&m_downchirp[0],m_sf,m_oversample_rate);

      m_up = 8;
      preamb_symb_cnt = 0;

      m_receiveRTSTokenPort = pmt::mp("RTSToken");
      message_port_register_in(m_receiveRTSTokenPort);
      set_msg_handler(m_receiveRTSTokenPort, boost::bind(&RTSSendToken_impl::receiveRTSToken, this, _1));
      m_state = STATE_RTS_RESET;

      m_outParaPort = pmt::mp("outPara");
      message_port_register_out(m_outParaPort);

      m_RTSTime = 0;
    }

    /*
     * Our virtual destructor.
     */
    RTSSendToken_impl::~RTSSendToken_impl()
    {
    }

    void
    RTSSendToken_impl::receiveRTSToken(pmt::pmt_t msg){
      m_state = STATE_RTS_SEND_TOKEN;
    }

    double
    RTSSendToken_impl::getTimeval(const struct timeval startTime,const struct timeval endTime){
      double stime =  startTime.tv_sec * 1000 + startTime.tv_usec / 1000;
      double etime = endTime.tv_sec * 1000 + endTime.tv_usec/1000;
      return etime - stime;
    }

    void
    RTSSendToken_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    RTSSendToken_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex*out = (gr_complex *) output_items[0];
      
      if(m_state == STATE_RTS_RESET){
        consume_each (0);
        return 0;
      }else{
        m_up = 8;
        preamb_symb_cnt = 0;
      }
      gettimeofday(&m_startRTSTime,NULL);
      int output_offset = 1;
      preamb_symb_cnt = 0;
      for (int i = 0; i < noutput_items / m_samples_per_symbol; i++)
      {
          if (preamb_symb_cnt < m_up + 3) //should output preamble part
          {
              if (preamb_symb_cnt < m_up)
              { //upchirps
                  memcpy(&out[output_offset], &m_upchirp[0], m_samples_per_symbol * sizeof(gr_complex));
              }
              else if (preamb_symb_cnt < m_up + 2) //2.25 downchirps
                  memcpy(&out[output_offset], &m_downchirp[0], m_samples_per_symbol * sizeof(gr_complex));
              else if (preamb_symb_cnt == m_up + 2)
              {
                  memcpy(&out[output_offset], &m_downchirp[0], m_samples_per_symbol / 4 * sizeof(gr_complex));
                  //correct offset dur to quarter of downchirp
                  output_offset -= 3 * m_samples_per_symbol / 4;                  
              }
              output_offset += m_samples_per_symbol;
              preamb_symb_cnt++;
          }
      }
      std::cout<<output_offset<<std::endl;
      std::cout<<noutput_items<<std::endl;
      if(output_offset > noutput_items){
        std::cout<<"添加的一次性内容超过了gnuradio最多项数"<<std::endl;
      }else{
        build_downchirp(&out[output_offset], m_nodeToken, m_sf,m_oversample_rate);
        
        output_offset += m_samples_per_symbol; 
        // std::cout<<"output Token"<<std::endl;
      }

      gettimeofday(&m_endRTSTime,NULL);
      m_RTSTime = getTimeval(m_startRTSTime,m_endRTSTime);
      m_state = STATE_RTS_RESET;
     
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (0);

      // Tell runtime system how many output items we produced.
      return output_offset;
    }

  } /* namespace RC_MAC */
} /* namespace gr */

