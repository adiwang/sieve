noinst_PROGRAMS = casd
casd_SOURCES = casd.cpp rediscb.cpp channel.cpp
casd_LDADD = $(top_srcdir)/common/tcpserver.o $(top_srcdir)/common/log.o $(top_srcdir)/pb/netmessage.pb.o $(top_srcdir)/common/configfile.o
casd_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/lib -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto -lprotobuf -lhiredis
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include -I$(top_srcdir)/include/hiredis --std=c++0x
export INCLUDES
