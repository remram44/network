CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=-I.
#LIBS=-lssl -lpthread
LIBS=-lmingw32 -lws2_32 -leay32 -lssleay32 -lpthread
CPPFLAGS = $(INCLUDES) -g -Wall -O2
MAKE=make

.PHONY: all libs tests sockets proxy engine clean

all: libs tests proxychain.exe
libs: sockets proxy engine
tests: test-sockets.exe pong.exe
sockets: libsockets.a
proxy: libproxy.a
engine: libnetwork.a

libsockets.a:
	$(MAKE) -C sockets

libproxy.a: libsockets.a
	$(MAKE) -C proxy

libnetwork.a: libsockets.a
	$(MAKE) -C engine

pong.exe: engine
	$(MAKE) -C pong

# Linking
test-sockets.exe: libsockets.a test-sockets.o
	$(CPP) -o $@ test-sockets.o -L. -lsockets $(LIBS)

proxychain.exe: libsockets.a proxy proxychain.o
	$(CPP) -o $@ proxychain.o -L. -lproxy -lsockets $(LIBS)

clean:
	$(RM) libsockets.a libproxy.a libnetwork.a
	$(RM) test-sockets.o proxychain.o
	$(MAKE) -C sockets clean
	$(MAKE) -C proxy clean
	$(MAKE) -C engine clean
	$(MAKE) -C pong clean
