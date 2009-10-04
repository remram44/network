CPP=g++
#RM=rm -f
RM=del /F
AR=ar rcs
INCLUDES=
CPPFLAGS = $(INCLUDE) -g -Wall -O2

.PHONY: all clean

all: ../libsockets.a

# Builds the static library
../libsockets.a: Socket.o SSLSocket.o
	$(AR) ../libsockets.a Socket.o SSLSocket.o

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o

Socket.o: Socket.h
SSLSocket.o: SSLSocket.h Socket.h
