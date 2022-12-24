#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir="/home/zjhao/gr-RC_MAC/python"
export GR_CONF_CONTROLPORT_ON=False
export PATH="/home/zjhao/gr-RC_MAC/build/python":$PATH
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/home/zjhao/gr-RC_MAC/build/swig:$PYTHONPATH
/usr/bin/python3.6 /home/zjhao/gr-RC_MAC/python/qa_ReceiveToken.py 
