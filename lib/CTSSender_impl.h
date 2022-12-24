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

#ifndef INCLUDED_RC_MAC_CTSSENDER_IMPL_H
#define INCLUDED_RC_MAC_CTSSENDER_IMPL_H

#include <RC_MAC/CTSSender.h>
#include <queue>
#include <gnuradio/fft/fft.h>
#include <volk/volk.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <time.h>
namespace gr {
  namespace RC_MAC {

    class CTSSender_impl : public CTSSender
    {
     private:
      //CTS 状态机接口
      CTS_Sender_State m_state;
      //模块数据出入接口
      pmt::pmt_t m_receive_data_port;
      pmt::pmt_t m_receive_RTS_port; 
      pmt::pmt_t m_out_data_port;
      pmt::pmt_t m_out_CTS_port;

      //lora 必要特征：sf、采样率、带宽
      uint32_t m_sf;
      uint32_t m_sampRate;
      uint32_t m_bw;

      //cad function
      uint32_t m_oversample_rate;
      uint32_t m_number_of_bins;
      uint32_t m_samples_per_symbol;
      int m_cad_count ;
      uint32_t m_fft_size;
      std::vector<gr_complex> m_upchirp,m_downchirp;
      gr::fft::fft_complex * m_fft;
      bool m_cad_detect;
      uint32_t searchFFTPeek(const lv_32fc_t * fft_result,float * max_value,float * fft_res_mag);
      unsigned int pmod(int x,unsigned int n);
      uint32_t argmax(float * fft_result,float * max_value);
      uint32_t m_SleepWindowCount;

      //网关状态---是否正在接收数据
      bool m_isconnected;
      int m_waitTime; //缓冲窗口，用于延迟作用
      int m_NodeIdConnected;
      int m_durationConnected;
      CTS_Class_Type m_classType;

      //每个slot之间的时间间隔
      uint32_t m_slotIntervalTime;

      //beacon 同步时间
      bool m_start; //判断是不是首次启动，首次启动需要立马发送beacon如果是B类的话
      std::string beaconMsg;
      uint32_t m_beaconIntervalTime;
      
      // std::vector<pair<int,int>> m_nodeIds; //ID和duration的结合 
      // std::queue<std::pair<int,int>> m_nodeIdDurations; //
      std::queue<int> m_sendNodeIds;

      void receiveDataSolve(pmt::pmt_t msg);
      void receiveRTSSolve(pmt::pmt_t msg);
      bool isNumeric(std::string const &str);
     public:
      CTSSender_impl(uint8_t sf,uint32_t bw,uint32_t sampRate,int classType);
      ~CTSSender_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

    };

  } // namespace RC_MAC
} // namespace gr

#endif /* INCLUDED_RC_MAC_CTSSENDER_IMPL_H */

