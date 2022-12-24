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

#ifndef INCLUDED_RC_MAC_RTSSENDTOKEN_IMPL_H
#define INCLUDED_RC_MAC_RTSSENDTOKEN_IMPL_H

#include <RC_MAC/RTSSendToken.h>
#include <vector>
#include <time.h>
#include <ctime>
#include "utilities.h"
namespace gr {
  namespace RC_MAC {

    class RTSSendToken_impl : public RTSSendToken
    {
     private:
      uint8_t m_sf;
      uint32_t m_samp_rate;
      uint32_t m_bw;
      uint32_t m_number_of_bins;
      uint32_t m_samples_per_symbol;   
      uint32_t m_nodeToken;

      int m_oversample_rate;
      int m_inter_frame_padding;
      int m_frame_len;

      std::vector<gr_complex> m_upchirp;
      std::vector<gr_complex> m_downchirp;

      uint8_t m_up; //preamble 长度
      int32_t symb_cnt; //发送lora信号的symbol
      uint32_t preamb_symb_cnt; //前导码symbol

      pmt::pmt_t m_receiveRTSTokenPort;
      void receiveRTSToken(pmt::pmt_t msg);
      STATE_RTS_TOKEN m_state;

      pmt::pmt_t m_outParaPort;
      struct timeval m_startRTSTime,m_endRTSTime;
		  double m_RTSTime;
      double getTimeval(const struct timeval startTime,const struct timeval endTime);

     public:
      RTSSendToken_impl(uint8_t sf, uint32_t samp_rate, uint32_t bw,uint32_t nodeToken);
      ~RTSSendToken_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

    };

  } // namespace RC_MAC
} // namespace gr

#endif /* INCLUDED_RC_MAC_RTSSENDTOKEN_IMPL_H */

