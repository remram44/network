CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=
LIBS=-lmingw32 -lws2_32 -leay32 -lssleay32
CPPFLAGS = $(INCLUDE) -g -Wall -O2
MAKE=make

.PHONY: all sockets engine clean

all: test-sockets.exe

sockets:
	$(MAKE) -C sockets

engine: sockets
	$(MAKE) -C engine

# Linking
test-sockets.exe: sockets test-sockets.o
	$(CPP) -o $@ test-sockets.o -L. -lsockets $(LIBS)

clean:
	$(RM) libsockets.a
	$(RM) test-sockets.o
	$(MAKE) -C sockets clean
	$(MAKE) -C engine clean
