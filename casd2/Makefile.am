noinst_PROGRAMS = casd
casd_SOURCES = casd.cpp rediscb.cpp channel.cpp pyloader.cpp
casd_LDADD = $(top_srcdir)/common/tcpserver.o $(top_srcdir)/common/log.o $(top_srcdir)/pb/netmessage.pb.o $(top_srcdir)/common/configfile.o $(top_srcdir)/common/json/json_value.o $(top_srcdir)/common/json/json_reader.o $(top_srcdir)/common/json/json_writer.o
casd_LDFLAGS = -L/usr/local/lib -L$(top_srcdir)/lib -L/usr/lib64/python2.7 -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto -lprotobuf -lhiredis -lpython2.7
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include -I$(top_srcdir)/include/hiredis -I/usr/include/python2.7 --std=c++0x
export INCLUDES
