#include <vector>
#include "midi.h"

#ifndef __MIDI_PARSER_H__
#define __MIDI_PARSER_H__

using namespace std;
typedef unsigned char uchar;

const uchar FIRST_NOTE_INDEX = 21;
const unsigned NOTE_COUNT = 88; // number of notes playable by a piano

class MIDIParser
{
private:
  uint32_t tempo;
  uint16_t ppq;
  struct comp;

  vector<int> getLargest(vector<int> midiRow, vector<uchar> playedNotes, int maxNotes);

public:
  unsigned noteCount;
  uchar firstNoteIndex;
  vector<vector<int>> rawData;

  MIDIParser(uint32_t tempo, uint16_t);
  MIDIParser();
  
  vector<int> getLargest(short *midiRow, vector<uchar> playedNotes, int maxNotes);
  MIDITrack getMidiTrack(short **midiTable, int frames, int noteSwitchThreshold, int minVelocity, int maxNotes, unsigned char program = 0, unsigned char channel = 0);

  MIDITrack getMidiTrack(bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0);

  MIDIFile getMidiFile(bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0);

  // legacy methods to handle CSV file input
  void loadRawDataFromCsv(const char *filename, char delimiter = ',');
  MIDIFile getMidiFile(const char *filename, bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0);
};

#endif
