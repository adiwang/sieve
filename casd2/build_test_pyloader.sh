#!/bin/bash

g++ test_pyloader.cpp pyloader.cpp ../common/log.cpp -o test_pyloader -I../include -I/usr/include/python2.7 -L/usr/lib/python2.7 -lpython2.7
