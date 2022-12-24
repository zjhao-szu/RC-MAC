/* -*- c++ -*- */

#define RC_MAC_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "RC_MAC_swig_doc.i"

%{
#include "RC_MAC/RTSSendToken.h"
#include "RC_MAC/ReceiveToken.h"
#include "RC_MAC/RTSSender.h"
#include "RC_MAC/CTSSender.h"
%}

%include "RC_MAC/RTSSendToken.h"
GR_SWIG_BLOCK_MAGIC2(RC_MAC, RTSSendToken);
%include "RC_MAC/ReceiveToken.h"
GR_SWIG_BLOCK_MAGIC2(RC_MAC, ReceiveToken);
%include "RC_MAC/RTSSender.h"
GR_SWIG_BLOCK_MAGIC2(RC_MAC, RTSSender);
%include "RC_MAC/CTSSender.h"
GR_SWIG_BLOCK_MAGIC2(RC_MAC, CTSSender);
