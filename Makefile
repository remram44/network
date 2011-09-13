CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=-I.
#LIBS=-lssl -lpthread
LIBS=-lmingw32 -lws2_32 -leay32 -lssleay32 -lpthread
CPPFLAGS = $(INCLUDES) -g -Wall -O2
MAKE=make

.PHONY: all libs tests sockets proxy engine clean

all: libs proxychain.exe tests
libs: sockets proxy engine
tests: test-sockets.exe pong.exe

# Order-only dependencies
# The phony order-only dependencies will be built, but will not force targets
# depending on the libraries
libsockets.a: | sockets
libproxy.a: | proxy
libnetwork.a: | engine

# Subdirectories
sockets:
	$(MAKE) -C sockets

proxy: libsockets.a
	$(MAKE) -C proxy

engine: libsockets.a
	$(MAKE) -C engine

pong.exe: libnetwork.a
	$(MAKE) -C pong

# Linking
test-sockets.exe: libsockets.a test-sockets.o
	$(CPP) -o $@ test-sockets.o -L. -lsockets $(LIBS)

proxychain.exe: libsockets.a libproxy.a proxychain.o
	$(CPP) -o $@ proxychain.o -L. -lproxy -lsockets $(LIBS)

clean:
	$(RM) libsockets.a libproxy.a libnetwork.a
	$(RM) test-sockets.o proxychain.o
	$(MAKE) -C sockets clean
	$(MAKE) -C proxy clean
	$(MAKE) -C engine clean
	$(MAKE) -C pong clean
