#include <vector>
#include "midi.h"

#ifndef __MIDI_PARSER_H__
#define __MIDI_PARSER_H__

using namespace std;

const uchar FIRST_NOTE_INDEX = 21; // 60; // TODO: set firstNote to 21
const unsigned NOTE_COUNT = 88;    // 10;    // number of notes playable by a piano -> TODO: set to 88

class MIDIParser
{
private:
  uint32_t tempo;
  uint16_t ppq;

public:
  unsigned noteCount;
  uchar firstNoteIndex;
  vector<vector<int>> rawData;

  MIDIParser(uint32_t tempo, uint16_t);
  MIDIParser();

  //void storeMax();

  void loadRawDataFromCsv(const char *filename, char delimiter = ',');

  MIDITrack getMidiTrack(bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0, bool holding = false);

  MIDIFile getMidiFile(bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0, bool holding = false);

  MIDIFile getMidiFile(const char *filename, bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0, bool holding = true);
};

#endif