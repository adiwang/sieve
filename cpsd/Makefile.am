noinst_PROGRAMS = cpsd
PYLON_ROOT ?= /usr/local/pylon5
cpsd_SOURCES = cpsd.cpp
cpsd_CPPFLAGS = $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
cpsd_LDADD = $(top_srcdir)/common/tcpclient.o $(top_srcdir)/common/log.o $(top_srcdir)/pb/netmessage.pb.o $(top_srcdir)/common/configfile.o 
cpsd_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/lib -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto -lprotobuf $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath) $(shell $(PYLON_ROOT)/bin/pylon-config --libs)
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include --std=c++0x
export INCLUDES
