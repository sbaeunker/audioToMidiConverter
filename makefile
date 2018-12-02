CC=gcc -std=c++14
CXX=g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)
 
SRCS=main.cpp lib/midi.cpp lib/midi-parser.cpp lib/kissfft/kiss_fftr.c lib/kissfft/kiss_fft.c lib/audiofile/AudioFile.cpp lib/frequencyApprox.cpp
OBJS=$(subst .cc,.o,$(SRCS))

all: main

main: $(OBJS)
	$(CXX) $(LDFLAGS) -o main $(OBJS) $(LDLIBS) 

main.o: main.cpp lib/midi.h lib/midi-parser.h

midi.o: midi.h midi.cpp

midi-parser.o: midi-parser.h midi-parser.cpp

audiofile.o: AudioFile.h AudioFile.cpp

frequencyApprox.o: frequencyApprox.h frequencyApprox.cpp

kiss_fftr.o: kissfft/kiss_fft.h kissfft/_kiss_fft_guts.h kissfft/kiss_fftr.h kissfft/kiss_fftr.c kissfft/kiss_fft.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) main

