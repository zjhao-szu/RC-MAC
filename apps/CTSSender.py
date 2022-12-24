#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: CTSSender
# Author: zjhao
# GNU Radio version: v3.8.5.0-6-g57bd109d

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from RC_MAC_PHY_Sender import RC_MAC_PHY_Sender  # grc-generated hier_block
from gnuradio import gr
from gnuradio.filter import firdes
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
from rc_mac_phy import rc_mac_phy  # grc-generated hier_block
import RC_MAC


class CTSSender(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "CTSSender")

        ##################################################
        # Variables
        ##################################################
        self.sf = sf = 7
        self.samp_rate = samp_rate = 250000
        self.frequency = frequency = 433000000
        self.classType = classType = 1
        self.bw = bw = 250000

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(("serial=32419E7", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0.set_center_freq(frequency, 0)
        self.uhd_usrp_source_0.set_gain(70, 0)
        self.uhd_usrp_source_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0.set_bandwidth(bw, 0)
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        # No synchronization enforced.
        self.uhd_usrp_sink_0_0 = uhd.usrp_sink(
            ",".join(("serial=32419E7", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
            'frame_len',
        )
        self.uhd_usrp_sink_0_0.set_subdev_spec('A:B', 0)
        self.uhd_usrp_sink_0_0.set_center_freq(433000000, 0)
        self.uhd_usrp_sink_0_0.set_gain(90, 0)
        self.uhd_usrp_sink_0_0.set_antenna('TX/RX', 0)
        self.uhd_usrp_sink_0_0.set_bandwidth(250000, 0)
        self.uhd_usrp_sink_0_0.set_samp_rate(samp_rate)
        # No synchronization enforced.
        self.rc_mac_phy_0 = rc_mac_phy()
        self.RC_MAC_PHY_Sender_0_0 = RC_MAC_PHY_Sender()
        self.RC_MAC_CTSSender_1 = RC_MAC.CTSSender(sf, bw, samp_rate, 1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.RC_MAC_CTSSender_1, 'OutData'), (self.RC_MAC_PHY_Sender_0_0, 'in'))
        self.msg_connect((self.rc_mac_phy_0, 'out'), (self.RC_MAC_CTSSender_1, 'receiveData'))
        self.connect((self.RC_MAC_PHY_Sender_0_0, 0), (self.uhd_usrp_sink_0_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.RC_MAC_CTSSender_1, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.rc_mac_phy_0, 0))


    def get_sf(self):
        return self.sf

    def set_sf(self, sf):
        self.sf = sf

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_sink_0_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)

    def get_frequency(self):
        return self.frequency

    def set_frequency(self, frequency):
        self.frequency = frequency
        self.uhd_usrp_source_0.set_center_freq(self.frequency, 0)

    def get_classType(self):
        return self.classType

    def set_classType(self, classType):
        self.classType = classType

    def get_bw(self):
        return self.bw

    def set_bw(self, bw):
        self.bw = bw
        self.uhd_usrp_source_0.set_bandwidth(self.bw, 0)





def main(top_block_cls=CTSSender, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
