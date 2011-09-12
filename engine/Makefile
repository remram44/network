CPP=g++
#RM=rm -f
RM=del /F
AR=ar rcs
INCLUDES=-I..
CPPFLAGS=$(INCLUDES) -g -Wall -O2

.PHONY: all clean

all: ../libnetwork.a

# Builds the static library
../libnetwork.a: Net.o NetClient.o NetServer.o CVar.o
	$(AR) ../libnetwork.a Net.o NetClient.o NetServer.o CVar.o

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o

Net.o: Net.cpp Net.h
NetClient.o: NetClient.cpp NetClient.h Net.h CVar.h
NetServer.o: NetServer.cpp
CVar.o: CVar.cpp CVar.h Net.h
