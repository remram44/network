CPP=g++
#RM=rm -f
RM=del /F
AR=ar rcs
INCLUDES=
CPPFLAGS = $(INCLUDE) -g -Wall -O2

.PHONY: all clean

all: ../libproxy.a

# Builds the static library
../libproxy.a: Proxy.o Socks4Server.o HTTPServer.o Forwarder.o Socks4Client.o HTTPClient.o
	$(AR) ../libproxy.a Proxy.o Socks4Server.o HTTPServer.o Forwarder.o Socks4Client.o HTTPClient.o

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o
