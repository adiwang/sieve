noinst_PROGRAMS = cpsd
PYLON_ROOT ?= /opt/pylon5
HALCON_ROOT ?= /opt/halcon
HALCON_ARCH ?= x64-linux
cpsd_SOURCES = cpsd.cpp leafanalysis.cpp
cpsd_CPPFLAGS = $(shell $(PYLON_ROOT)/bin/pylon-config --cflags) 
cpsd_LDADD = $(top_srcdir)/common/tcpclient.o $(top_srcdir)/common/log.o $(top_srcdir)/pb/netmessage.pb.o $(top_srcdir)/common/configfile.o 
cpsd_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/lib -DDEBUG -DUSE_GIGE -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto -lprotobuf $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath) $(shell $(PYLON_ROOT)/bin/pylon-config --libs) -L$(HALCON_ROOT)/lib/x64-linux -L$(HALCON_ROOT)/lib/$(HALCON_ARCH) -lhalconcpp -lhalcon
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include -I$(HALCON_ROOT)/include -I$(HALCON_ROOT)/include/halconcpp --std=c++0x
export INCLUDES
