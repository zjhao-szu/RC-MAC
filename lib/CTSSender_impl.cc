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
#include "CTSSender_impl.h"


#define SLOTINTERVALTIME 1000
#define BEACONINTERVALTIME 1500

namespace gr {
  namespace RC_MAC {

    CTSSender::sptr
    CTSSender::make(uint8_t sf,uint32_t bw,uint32_t sampRate,int classType)
    {
      return gnuradio::get_initial_sptr
        (new CTSSender_impl(sf,bw,sampRate,classType));
    }


    /*
     * The private constructor
     */
    CTSSender_impl::CTSSender_impl(uint8_t sf,uint32_t bw,uint32_t sampRate,int classType)
      : gr::block("CTSSender",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0)),
              m_sf(sf),
              m_bw(bw),
              m_sampRate(sampRate)
    {
      //自动机状态切换
      m_state  = S_CTS_Reset;

      //port选择
      m_receive_data_port = pmt::mp("receiveData");
      m_receive_RTS_port = pmt::mp("receiveRTS");
      m_out_data_port = pmt::mp("OutData");
      m_out_CTS_port = pmt::mp("OutCTS");

      message_port_register_in(m_receive_data_port);
      message_port_register_in(m_receive_RTS_port);
      message_port_register_out(m_out_data_port);
      message_port_register_out(m_out_CTS_port);

      set_msg_handler(m_receive_data_port,boost::bind(&CTSSender_impl::receiveDataSolve,this,_1));
      set_msg_handler(m_receive_RTS_port,boost::bind(&CTSSender_impl::receiveRTSSolve,this,_1));
      
      //网关节点初始状态
      m_isconnected = false;
      m_NodeIdConnected = -1;
      m_classType = CTS_ClassA;
      
      //ping slot 时间间隔
      // m_slotIntervalTime = SLOTINTERVALTIME;
      // m_beaconIntervalTime = BEACONINTERVALTIME;
      m_waitTime = 10;
      
      if(classType == 0){
        m_classType = CTS_ClassA;
      }else if(classType == 1){
        m_classType = CTS_ClassB;
      }else if(classType == 2){
        m_classType = CTS_ClassC;
      } 

      m_number_of_bins = (uint32_t)(1u << m_sf); //根据sf决定每个sysbol 占用多少个bins
      m_oversample_rate = m_sampRate / m_bw; //根据 采样率 和 带宽 决定比例
      m_samples_per_symbol = (uint32_t) (m_number_of_bins * m_oversample_rate); 
      m_fft_size = m_samples_per_symbol; //fft大小一般跟symbol采样率一致
      m_fft = new fft::fft_complex(m_fft_size,true,1);

      m_upchirp.resize(m_samples_per_symbol);
      m_downchirp.resize(m_samples_per_symbol);
      for(int i = 0; i < m_samples_per_symbol;i++){
          double phase = M_PI / m_oversample_rate * (i - i*i / (float) m_samples_per_symbol);
          m_downchirp[i] = gr_complex(std::polar(1.0,phase));
          m_upchirp[i] = gr_complex(std::polar(1.0,-1*phase));
      }
      m_cad_count = 4;
    }

    /*
     * Our virtual destructor.
     */
    CTSSender_impl::~CTSSender_impl()
    {
    }
    bool 
    CTSSender_impl::isNumeric(std::string const &str)
    {
      auto it = std::find_if(str.begin(), str.end(), [](char const &c) {
        return !std::isdigit(c);
      });
      return !str.empty() && it == str.end();
    }
    void
    CTSSender_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }


    /***********************************************************************
     *  该部分接口用于处理接收端口传入的数据-包括RTS数据包 以及 用户 
     * 
     ***********************************************************************/
    void
    CTSSender_impl::receiveRTSSolve(const pmt::pmt_t msg){ //根据接收模块接收的数据打印数据
      uint32_t nodeToken = pmt::to_double(msg);
      m_sendNodeIds.push((int)(nodeToken+1)/2);
      // m_nodeIdDurations.push(std::make_pair<int,int>((int)(nodeToken+1)/2,0));
    }
    void
    CTSSender_impl::receiveDataSolve(pmt::pmt_t msg){ //根据USRP模块接收的信号，就地解析数据
      //do nothing
      std::cout<<"接收到了用户信息:"<<std::endl;
      std::cout<<pmt::symbol_to_string(msg)<<std::endl;
      std::cout<<"current state is "<<m_state<<std::endl;
      if(m_state != S_CTS_Receive) return;
      std::string str = pmt::symbol_to_string(msg);
      
      //message 格式：“Type:RTS,NodeId:10”
      if(str.find("RTS") != std::string::npos){
        int indexId = str.find("NodeId",0);
        int indexColon = str.find(":",indexId);
        int winNodeID;
        if(isNumeric(str.substr(indexColon+1,str.length() - indexColon- 1))){
          winNodeID = std::stoi(str.substr(indexColon+1,str.length() - indexColon- 1));
        }else{
          m_state = S_CTS_Reset;
          return;
        }
       
      
        std::cout<<"收到来自节点NodeId:"<<winNodeID<<std::endl;
        m_sendNodeIds.push(winNodeID);
        m_state = S_CTS_CADDetect;
      }else{
        std::cout<<"************user data message******************"<<std::endl;
        std::cout<<str<<std::endl;
        m_state = S_CTS_Reset;
      }
    }

    //CADDetect Function
    unsigned int 
    CTSSender_impl::pmod(int x,unsigned int n){
      return ((x%n) + n) % n;
    };
    uint32_t 
    CTSSender_impl::argmax(float * fft_result,float * max_value){
        float mag = std::abs(fft_result[0]);
        float max_val = mag;
        uint32_t max_index = 0;
        for(int i = 0; i < m_number_of_bins;i++){
            mag =  std::abs(fft_result[i]);
            if(mag > max_val){
                max_index = i;
                max_val = mag;
            }
        }
        *max_value = max_val;
        return max_index;
    }
    uint32_t 
    CTSSender_impl::searchFFTPeek(const gr_complex *fft_result,float * max_value,float * fft_res_mag){
        volk_32fc_magnitude_32f(fft_res_mag, fft_result, m_fft_size);
        uint32_t max_idx = argmax(fft_res_mag, max_value);
        return max_idx;
    }

    int
    CTSSender_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      
      switch (m_state)
      {
      case S_CTS_Reset:{
        std::cout<<"reset:: to receive State"<<std::endl;
        m_state = S_CTS_Receive;
        // m_durationConnected = -1;
        m_waitTime = 10; //
        m_NodeIdConnected = -1;
        m_isconnected = false;
        break;
      }

      case S_CTS_Send_Data:{
        //ToDo send message
        
        break;
      }

      case S_CTS_Receive:{
        //TODO：一直接收来自外部的数据
        
        break;
      }
      case S_CTS_CADDetect:{
        //TODO：一直接收来自外部的数据
        float maxValue;
        int maxIndex;
        m_cad_detect = false;
        gr_complex * signalBlockBuffer =  (gr_complex *) volk_malloc(m_fft_size * sizeof(gr_complex),volk_get_alignment());
        gr_complex * outbuf =  (gr_complex *) volk_malloc(m_fft_size * sizeof(gr_complex),volk_get_alignment());
        float * fft_res_mag = (float*)volk_malloc(m_fft_size*sizeof(float), volk_get_alignment());

        volk_32fc_x2_multiply_32fc(signalBlockBuffer, in, &m_downchirp[0], m_samples_per_symbol);
        
        memset(m_fft->get_inbuf(), 0,m_fft_size*sizeof(gr_complex));
        memcpy(m_fft->get_inbuf(),&signalBlockBuffer[0],m_samples_per_symbol*sizeof(gr_complex));
        m_fft->execute();
        maxValue = 0;
        maxIndex = searchFFTPeek(m_fft->get_outbuf(),&maxValue,fft_res_mag);
        std::cout<<maxValue<<std::endl;
        if(maxValue >= 0.30){
          m_cad_detect = true;
          m_cad_count -= 1;
        }else{
          // std::cout<<"while noise is"<<maxValue<<std::endl;
          m_cad_count -= 1;
        }

        if(m_cad_detect){
          std::cout<<"CAD DETECTED!!!! Node Need to Sleep!!!!"<<std::endl;
          //SLEEP 会等待二进制退避算法结束休眠
          //reset 则会重置缓冲区的数据，但是我们还没发送数据，所以这里状态不能切换至reset状态
          m_state = S_CTS_WAIT;
          m_SleepWindowCount = 4;
        }
          
        if(m_cad_count == 0 && m_cad_detect == false){
          std::cout<<"==============CAD Detect count is 0 && 没有CAD 被检测============"<<std::endl;
          std::cout<<"==============进入发送CTS阶段===================================="<<std::endl;
          m_state = S_CTS_Send_CTS_ClassA;
        }
        volk_free(signalBlockBuffer);
        volk_free(outbuf);
        volk_free(fft_res_mag);
        break;
      }
      case S_CTS_WAIT:{
        m_SleepWindowCount--;
        if(m_SleepWindowCount == 0){
          m_cad_count = 4;
          m_state = S_CTS_CADDetect;
        }
        break;
      }
      case S_CTS_Send_CTS_ClassA:{
        std::string message = "";
        int winNodeId = 0;
        //当前网关节点是否正在通信，如果是则返回相应的正在沟通的CTS
        
        std::cout<<"=====Class A采用先来先服务策略==="<<std::endl;
        if(m_isconnected){
          //可以选择发或者可以选择发送当前链接的节点
          winNodeId = m_NodeIdConnected;
          std::cout<<"当前网关正在通信,与节点:"<<winNodeId<<std::endl;
        }else{
          winNodeId = m_sendNodeIds.front();
          std::cout<<"当前网关未与人通信,获胜节点为:"<<winNodeId<<std::endl;
        }
        
  
        //class A采用先来先服务策略
        //采用data数据
        message = "type:CTS,NodeId:"+std::to_string(winNodeId);
        pmt::pmt_t pmtmsg = pmt::string_to_symbol(message);
        std::cout<<"sendCTSMessage"<<std::endl;
        message_port_pub(m_out_data_port,pmtmsg);
        m_state = S_CTS_Reset;
        // //采用downchirp
        // pmt::pmt_t CTSToken = pmt::from_uint64(winNodeId);
        // message_port_pub(m_out_CTS_port,CTSToken);
        // m_state = S_CTS_Receive;
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

