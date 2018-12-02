#include <vector>
#include "midi.h"

#ifndef __MIDI_PARSER_H__
#define __MIDI_PARSER_H__

using namespace std;
typedef unsigned char uchar;

const uchar FIRST_NOTE_INDEX = 21;
const unsigned NOTE_COUNT = 88;      // number of notes playable by a piano
const int NOTE_SWITCH_THRESHOLD = 4; // don't switch on too similar notes ... less midi Messages
const int MIN_VOLUME = 5;            //don't play too silent notes ... less midi Messages

class MIDIParser
{
private:
  uint32_t tempo;
  uint16_t ppq;
  uchar minVolume;
  uchar noteSwitchThreshold;
  struct comp;

  vector<int> getLargest(vector<int> midiRow, vector<uchar> playedNotes, int maxNotes);

public:
  unsigned noteCount;   // blame/TODO: only public for demo/testing
  uchar firstNoteIndex; // blame/TODO: only public for demo/testing
  vector<vector<int>> rawData;

  MIDIParser(uint32_t tempo, uint16_t ppq, uchar minVolume, uchar noteSwitchThreshold);
  MIDIParser(uint32_t tempo, uint16_t ppq);
  MIDIParser();

  MIDITrack getMidiTrack(short **midiTable, int frames, int maxNoteCount = NOTE_COUNT, unsigned char program = 0, unsigned char channel = 0);
  MIDIFile getMidiFile(short **midiTable, int frames, int maxNoteCount = NOTE_COUNT, unsigned char program = 0, unsigned char channel = 0);

  MIDITrack getMidiTrack(bool getTempoFromData = false, int maxNoteCount = NOTE_COUNT, unsigned char program = 0, unsigned char channel = 0);
  MIDIFile getMidiFile(bool getTempoFromData = false, int maxNoteCount = NOTE_COUNT, unsigned char program = 0, unsigned char channel = 0);

  // legacy methods to handle CSV file input
  void loadRawDataFromCsv(const char *filename, char delimiter = ',');
  MIDIFile getMidiFile(const char *filename, bool getTempoFromData = false, int maxNoteCount = NOTE_COUNT, unsigned char program = 0, unsigned char channel = 0);
};

#endif
