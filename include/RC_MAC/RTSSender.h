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

#ifndef INCLUDED_RC_MAC_RTSSENDER_H
#define INCLUDED_RC_MAC_RTSSENDER_H

#include <RC_MAC/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace RC_MAC {
    enum RTS_Sender_State{
      S_RTS_RESET,
      S_RTS_RECEIVE_DATA,
      S_RTS_CAD,
      S_RTS_WAIT_DATA,
      S_RTS_WAIT_CTS,
      S_RTS_send_RTS,
      S_RTS_SLEEP,
      S_RTS_TO_RECEIVE1,
      S_RTS_RECEIVE1,
      S_RTS_TO_RECEIVE2,
      S_RTS_RECEIVE2,
      S_RTS_Send_Data,
      S_RTS_RECEIVE_Slot,
      S_RTS_RECEIVE_Class_C,
      S_RTS_BEACON
    };
    enum RTS_Class_Type{
      M_RTS_CLASSA,
      M_RTS_CLASSB,
      M_RTS_CLASSC
    };
    /*!
     * \brief <+description of block+>
     * \ingroup RC_MAC
     *
     */
    class RC_MAC_API RTSSender : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<RTSSender> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of RC_MAC::RTSSender.
       *
       * To avoid accidental use of raw pointers, RC_MAC::RTSSender's
       * constructor is in a private implementation
       * class. RC_MAC::RTSSender::make is the public interface for
       * creating new instances.
       */
      static sptr make(uint32_t sf,uint32_t bw,int classType,uint32_t NodeId,uint32_t sampRate);
    };

  } // namespace RC_MAC
} // namespace gr

#endif /* INCLUDED_RC_MAC_RTSSENDER_H */

