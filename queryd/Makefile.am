noinst_PROGRAMS = queryd
queryd_SOURCES = queryd.cpp rediscb.cpp
queryd_LDADD = $(top_srcdir)/common/tcpserver.o $(top_srcdir)/common/log.o $(top_srcdir)/pb/netmessage.pb.o
queryd_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/lib -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto -lprotobuf -lhiredis
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include -I$(top_srcdir)/include/hiredis --std=c++0x
export INCLUDES
