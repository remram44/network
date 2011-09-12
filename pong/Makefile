CPP=g++
#RM=rm -f
RM=del /F
INCLUDES=-I..
LIBS=-lmingw32 -lws2_32 -lSDLmain -lSDL
CPPFLAGS=$(INCLUDES) -g -Wall -O2

OBJS=main.o MovingRect.o GameEngine.o ServerEngine.o ClientEngine.o

.PHONY: all clean

all: ../pong.exe

# Linking
../pong.exe: $(OBJS)
	$(CPP) -o $@ $(OBJS) -L.. -lnetwork -lsockets $(LIBS)

# Compile a .cpp into a .o
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# Clean up object files
clean:
	$(RM) *.o

main.o: main.cpp GameEngine.h MovingRect.h ../engine/Net.h \
 ../engine/CVar.h ../engine/Net.h ClientEngine.h ../engine/NetClient.h \
 ../engine/CVar.h ServerEngine.h ../engine/NetServer.h
MovingRect.o: MovingRect.cpp MovingRect.h ../engine/Net.h GameEngine.h \
 ../engine/CVar.h ../engine/Net.h
GameEngine.o: GameEngine.cpp GameEngine.h MovingRect.h ../engine/Net.h \
 ../engine/CVar.h ../engine/Net.h
ServerEngine.o: ServerEngine.cpp ServerEngine.h ../engine/NetServer.h \
 ../engine/Net.h ../engine/CVar.h GameEngine.h MovingRect.h \
 ../engine/Net.h ../engine/CVar.h
ClientEngine.o: ClientEngine.cpp ClientEngine.h GameEngine.h MovingRect.h \
 ../engine/Net.h ../engine/CVar.h ../engine/Net.h ../engine/NetClient.h \
 ../engine/CVar.h
