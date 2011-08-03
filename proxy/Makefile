CPP=g++
#RM=rm -f
RM=del /F
AR=ar rcs
INCLUDES=-I..
CPPFLAGS=$(INCLUDES) -g -Wall -O2

.PHONY: all clean

all: ../libproxy.a

# Builds the static library
../libproxy.a: Proxy.o Socks4Server.o HTTPServer.o Forwarder.o Socks4Client.o HTTPClient.o Delayer.o
	$(AR) ../libproxy.a Proxy.o Socks4Server.o HTTPServer.o Forwarder.o Socks4Client.o HTTPClient.o Delayer.o

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o

Proxy.o: Proxy.cpp Proxy.h ../sockets/Socket.h
Socks4Server.o: Socks4Server.cpp Socks4Server.h Proxy.h \
 ../sockets/Socket.h ProxyServer.h
HTTPServer.o: HTTPServer.cpp HTTPServer.h Proxy.h ../sockets/Socket.h \
 ProxyServer.h
Forwarder.o: Forwarder.cpp Forwarder.h Proxy.h ../sockets/Socket.h \
 ProxyServer.h
Socks4Client.o: Socks4Client.cpp Socks4Client.h Proxy.h \
 ../sockets/Socket.h
HTTPClient.o: HTTPClient.cpp HTTPClient.h Proxy.h ../sockets/Socket.h
Delayer.o: Delayer.cpp Delayer.h Proxy.h ../sockets/Socket.h \
 ../sockets/DelayedStream.h ../sockets/Socket.h
