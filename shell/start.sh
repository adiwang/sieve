#!/bin/bash

ROOT_DIR=/home/hzfy/dist

echo "starting casd ..."
#$ROOT_DIR/casd/casd $ROOT_DIR/conf/casd.conf > /dev/null 2>&1 &
$ROOT_DIR/casd/casd $ROOT_DIR/conf/casd.conf > casd.out 2>&1 &
echo "wait 5 seconds ..."
sleep 5
echo "starting cpsd ..."
#$ROOT_DIR/cpsd/cpsd $ROOT_DIR/conf/cpsd.conf > /dev/null 2>&1 &
$ROOT_DIR/cpsd/cpsd $ROOT_DIR/conf/cpsd.conf > cpsd.out 2>&1 &
