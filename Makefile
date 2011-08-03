CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=-I.
LIBS=-lmingw32 -lws2_32 -leay32 -lssleay32
CPPFLAGS = $(INCLUDES) -g -Wall -O2
MAKE=make

.PHONY: all sockets proxy engine clean

all: test-sockets.exe proxychain.exe

sockets:
	$(MAKE) -C sockets

proxy: sockets
	$(MAKE) -C proxy

engine: sockets
	$(MAKE) -C engine

# Linking
test-sockets.exe: sockets test-sockets.o
	$(CPP) -o $@ test-sockets.o -L. -lsockets $(LIBS)

proxychain.exe: sockets proxy proxychain.o
	$(CPP) -o $@ proxychain.o -L. -lproxy -lsockets $(LIBS)

clean:
	$(RM) libsockets.a libproxy.a
	$(RM) test-sockets.o proxychain.o
	$(MAKE) -C sockets clean
	$(MAKE) -C proxy clean
	$(MAKE) -C engine clean
