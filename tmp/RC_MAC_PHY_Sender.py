# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Rc Mac Phy Sender
# GNU Radio version: v3.8.5.0-6-g57bd109d

from gnuradio import blocks
import pmt
from gnuradio import gr
from gnuradio.filter import firdes
import sys
import signal
import lora_sdr





class RC_MAC_PHY_Sender(gr.hier_block2):
    def __init__(self):
        gr.hier_block2.__init__(
            self, "Rc Mac Phy Sender",
                gr.io_signature(0, 0, 0),
                gr.io_signature(1, 1, gr.sizeof_gr_complex*1),
        )
        self.message_port_register_hier_in("in")

        ##################################################
        # Variables
        ##################################################
        self.bw = bw = 250000
        self.sf = sf = 7
        self.samp_rate = samp_rate = bw
        self.impl_head = impl_head = False
        self.has_crc = has_crc = True
        self.frame_period = frame_period = 3000
        self.cr = cr = 3
        self.center_freq = center_freq = 433e6
        self.TX_gain = TX_gain = 60

        ##################################################
        # Blocks
        ##################################################
        self.lora_sdr_whitening_0 = lora_sdr.whitening()
        self.lora_sdr_modulate_0 = lora_sdr.modulate(sf, bw, bw, [8,16])
        self.lora_sdr_modulate_0.set_min_output_buffer(10000000)
        self.lora_sdr_interleaver_0 = lora_sdr.interleaver(cr, sf)
        self.lora_sdr_header_0 = lora_sdr.header(impl_head, has_crc, cr)
        self.lora_sdr_hamming_enc_0 = lora_sdr.hamming_enc(cr, sf)
        self.lora_sdr_gray_decode_0 = lora_sdr.gray_decode(sf)
        self.lora_sdr_add_crc_0 = lora_sdr.add_crc(has_crc)
        self.lora_sdr_SendRecordParam_0 = lora_sdr.SendRecordParam(4)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.dict_add(pmt.make_dict(),pmt.intern("sf"),pmt.from_uint64(sf)), frame_period)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.lora_sdr_gray_decode_0, 'inSF'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.lora_sdr_hamming_enc_0, 'inSF'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.lora_sdr_interleaver_0, 'inSF'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.lora_sdr_modulate_0, 'inSF'))
        self.msg_connect((self.lora_sdr_modulate_0, 'outPara'), (self.lora_sdr_SendRecordParam_0, 'endIn'))
        self.msg_connect((self.lora_sdr_whitening_0, 'paraOut'), (self.lora_sdr_SendRecordParam_0, 'StartIn'))
        self.msg_connect((self, 'in'), (self.lora_sdr_whitening_0, 'msg'))
        self.connect((self.lora_sdr_add_crc_0, 0), (self.lora_sdr_hamming_enc_0, 0))
        self.connect((self.lora_sdr_gray_decode_0, 0), (self.lora_sdr_modulate_0, 0))
        self.connect((self.lora_sdr_hamming_enc_0, 0), (self.lora_sdr_interleaver_0, 0))
        self.connect((self.lora_sdr_header_0, 0), (self.lora_sdr_add_crc_0, 0))
        self.connect((self.lora_sdr_interleaver_0, 0), (self.lora_sdr_gray_decode_0, 0))
        self.connect((self.lora_sdr_modulate_0, 0), (self, 0))
        self.connect((self.lora_sdr_whitening_0, 0), (self.lora_sdr_header_0, 0))


    def get_bw(self):
        return self.bw

    def set_bw(self, bw):
        self.bw = bw
        self.set_samp_rate(self.bw)

    def get_sf(self):
        return self.sf

    def set_sf(self, sf):
        self.sf = sf
        self.blocks_message_strobe_0_0.set_msg(pmt.dict_add(pmt.make_dict(),pmt.intern("sf"),pmt.from_uint64(self.sf)))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

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
        self.blocks_message_strobe_0_0.set_period(self.frame_period)

    def get_cr(self):
        return self.cr

    def set_cr(self, cr):
        self.cr = cr

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq

    def get_TX_gain(self):
        return self.TX_gain

    def set_TX_gain(self, TX_gain):
        self.TX_gain = TX_gain


