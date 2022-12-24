# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Rc Mac Phy Receiver
# Author: zjhao
# GNU Radio version: v3.8.5.0-6-g57bd109d

from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
import lora_sdr





class rc_mac_phy_receiver(gr.hier_block2):
    def __init__(self):
        gr.hier_block2.__init__(
            self, "Rc Mac Phy Receiver",
                gr.io_signature(1, 1, gr.sizeof_gr_complex*1),
                gr.io_signature(0, 0, 0),
        )
        self.message_port_register_hier_out("out")

        ##################################################
        # Variables
        ##################################################
        self.sf = sf = 7
        self.samp_rate = samp_rate = 250e3
        self.pay_len = pay_len = 11
        self.impl_head = impl_head = False
        self.has_crc = has_crc = True
        self.frame_period = frame_period = 3000
        self.cr = cr = 3
        self.center_freq = center_freq = 433e6
        self.bw = bw = 250000

        ##################################################
        # Blocks
        ##################################################
        self.lora_sdr_recordParams_0 = lora_sdr.recordParams()
        self.lora_sdr_readParam_0 = lora_sdr.readParam(sf, samp_rate, bw)
        self.lora_sdr_header_decoder_0 = lora_sdr.header_decoder(impl_head, cr, 50, has_crc)
        self.lora_sdr_hamming_dec_0 = lora_sdr.hamming_dec()
        self.lora_sdr_gray_enc_0 = lora_sdr.gray_enc()
        self.lora_sdr_frame_sync_0 = lora_sdr.frame_sync(bw, bw, sf, impl_head, [0x12])
        self.lora_sdr_fft_demod_0 = lora_sdr.fft_demod( sf, impl_head)
        self.lora_sdr_dewhitening_0 = lora_sdr.dewhitening()
        self.lora_sdr_deinterleaver_0 = lora_sdr.deinterleaver(sf)
        self.lora_sdr_crc_verif_0 = lora_sdr.crc_verif(center_freq, sf)
        self.interp_fir_filter_xxx_0 = filter.interp_fir_filter_ccf(4, (-0.128616616593872,    -0.212206590789194,    -0.180063263231421,    3.89817183251938e-17    ,0.300105438719035    ,0.636619772367581    ,0.900316316157106,    1    ,0.900316316157106,    0.636619772367581,    0.300105438719035,    3.89817183251938e-17,    -0.180063263231421,    -0.212206590789194,    -0.128616616593872))
        self.interp_fir_filter_xxx_0.declare_sample_delay(0)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.lora_sdr_crc_verif_0, 'msg'), (self.lora_sdr_recordParams_0, 'msgIn'))
        self.msg_connect((self.lora_sdr_crc_verif_0, 'msg'), (self, 'out'))
        self.msg_connect((self.lora_sdr_header_decoder_0, 'frame_info'), (self.lora_sdr_frame_sync_0, 'frame_info'))
        self.msg_connect((self.lora_sdr_readParam_0, 'SNROut'), (self.lora_sdr_recordParams_0, 'SNRIN'))
        self.connect((self.interp_fir_filter_xxx_0, 0), (self.lora_sdr_readParam_0, 0))
        self.connect((self.lora_sdr_deinterleaver_0, 0), (self.lora_sdr_hamming_dec_0, 0))
        self.connect((self.lora_sdr_dewhitening_0, 0), (self.lora_sdr_crc_verif_0, 0))
        self.connect((self.lora_sdr_fft_demod_0, 0), (self.lora_sdr_gray_enc_0, 0))
        self.connect((self.lora_sdr_frame_sync_0, 0), (self.lora_sdr_fft_demod_0, 0))
        self.connect((self.lora_sdr_gray_enc_0, 0), (self.lora_sdr_deinterleaver_0, 0))
        self.connect((self.lora_sdr_hamming_dec_0, 0), (self.lora_sdr_header_decoder_0, 0))
        self.connect((self.lora_sdr_header_decoder_0, 0), (self.lora_sdr_dewhitening_0, 0))
        self.connect((self.lora_sdr_readParam_0, 0), (self.lora_sdr_frame_sync_0, 0))
        self.connect((self, 0), (self.interp_fir_filter_xxx_0, 0))


    def get_sf(self):
        return self.sf

    def set_sf(self, sf):
        self.sf = sf

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

    def get_pay_len(self):
        return self.pay_len

    def set_pay_len(self, pay_len):
        self.pay_len = pay_len

    def get_impl_head(self):
        return self.impl_head

    def set_impl_head(self, impl_head):
        self.impl_head = impl_head

    def get_has_crc(self):
        return self.has_crc

    def set_has_crc(self, has_crc):
        self.has_crc = has_crc

    def get_frame_period(self):
        return self.frame_period

    def set_frame_period(self, frame_period):
        self.frame_period = frame_period

    def get_cr(self):
        return self.cr

    def set_cr(self, cr):
        self.cr = cr

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq

    def get_bw(self):
        return self.bw

    def set_bw(self, bw):
        self.bw = bw


