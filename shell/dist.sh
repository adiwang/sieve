#!/bin/bash

SRC_DIR=/home/hzfy/workspace/cpp/sieve
DST_DIR=/home/hzfy/dist

#1. stop applications
#2. copy files
/bin/cp -f $SRC_DIR/casd2/casd $DST_DIR/casd/casd
/bin/cp -f $SRC_DIR/cpsd2/cpsd $DST_DIR/cpsd/cpsd
#3. start applications
#$DST_DIR/casd/casd $DST_DIR/conf/casd.conf
#$DST_DIR/cpsd/cpsd $DST_DIR/conf/cpsd.conf
