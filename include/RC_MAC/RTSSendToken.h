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

#ifndef INCLUDED_RC_MAC_RTSSENDTOKEN_H
#define INCLUDED_RC_MAC_RTSSENDTOKEN_H

#include <RC_MAC/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace RC_MAC {
    enum STATE_RTS_TOKEN{
      STATE_RTS_RESET,
      STATE_RTS_SEND_TOKEN
    };
    /*!
     * \brief <+description of block+>
     * \ingroup RC_MAC
     *
     */
    class RC_MAC_API RTSSendToken : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<RTSSendToken> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of RC_MAC::RTSSendToken.
       *
       * To avoid accidental use of raw pointers, RC_MAC::RTSSendToken's
       * constructor is in a private implementation
       * class. RC_MAC::RTSSendToken::make is the public interface for
       * creating new instances.
       */
      static sptr make(uint8_t sf, uint32_t samp_rate, uint32_t bw,uint32_t nodeToken);
    };

  } // namespace RC_MAC
} // namespace gr

#endif /* INCLUDED_RC_MAC_RTSSENDTOKEN_H */

