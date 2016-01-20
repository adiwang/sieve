noinst_PROGRAMS = testtcpserver testtcpclient
testtcpserver_SOURCES = test_tcpserver.cpp
testtcpclient_SOURCES = test_tcpclient.cpp
testtcpserver_LDADD = $(top_srcdir)/common/tcpserver.o $(top_srcdir)/common/log.o
testtcpserver_LDFLAGS = -L/usr/local/lib -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto
testtcpclient_LDADD = $(top_srcdir)/common/tcpclient.o $(top_srcdir)/common/log.o
testtcpclient_LDFLAGS = -L/usr/local/lib -D_GNU_SOURCE -luv -lpthread -lssl -lcrypto
INCLUDES = -I$(top_srcdir)/include -I/usr/local/include --std=c++0x
export INCLUDES
