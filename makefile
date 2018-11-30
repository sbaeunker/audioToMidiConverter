CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=main.cpp midi.cpp midi-parser.cpp
OBJS=$(subst .cc,.o,$(SRCS))

all: main

main: $(OBJS)
	$(CXX) $(LDFLAGS) -o main $(OBJS) $(LDLIBS) 

main.o: main.cpp midi.h midi-parser.h

midi.o: midi.h midi.cpp

midi-parser.o: midi-parser.h midi-parser.cpp

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) main
