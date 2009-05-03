CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=
#LIBS=
LIBS=-lmingw32 -lws2_32
CPPFLAGS = $(INCLUDE) -g -Wall -O2

.PHONY: all clean

all: test-sockets.exe

# Linking
test-sockets.exe: test-sockets.o Socket.o
	$(CPP) -o $@ test-sockets.o Socket.o $(LIBS)

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o
