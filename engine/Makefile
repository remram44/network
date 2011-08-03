CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=-I..
#LIBS=
LIBS=-lmingw32 -lws2_32
CPPFLAGS=$(INCLUDES) -g -Wall -O2

.PHONY: all clean

all:

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o
