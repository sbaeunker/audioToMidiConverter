#include <vector>
#include "midi.h"

#ifndef __MIDI_PARSER_H__
#define __MIDI_PARSER_H__

using namespace std;
typedef unsigned char uchar;

const uchar FIRST_NOTE_INDEX = 21;
const unsigned NOTE_COUNT = 88; // number of notes playable by a piano

const uint32_t DEFAULT_TEMPO = 500 * 1000; // microseconds for one quarter note / beat
const uint16_t DEFAULT_PPQ = 480;          // parts per quarter note

const int NOTE_SWITCH_THRESHOLD = 4; // don't switch on too similar notes ... less midi Messages
const int MIN_VOLUME = 5;            //don't play too silent notes ... less midi Messages

class MIDIParser
{
private:
  uint32_t tempo;
  uint16_t ppq;
  uchar minVolume;
  uchar noteSwitchThreshold;
  int maxNoteCount;
  struct comp;

  vector<int> getLargest(vector<int> rowData, vector<uchar> noteStatus, int maxNotes);
  MIDITrack getMidiTrack(vector<vector<int>> data, bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0);

public:
  unsigned noteCount;   // blame/TODO: only public for demo/testing
  uchar firstNoteIndex; // blame/TODO: only public for demo/testing

  MIDIParser(uint32_t tempo, uint16_t ppq, int maxNoteCount, uchar minVolume, uchar noteSwitchThreshold);
  MIDIParser(uint32_t tempo, uint16_t ppq, int maxNoteCount);
  MIDIParser(uint32_t tempo, uint16_t ppq);
  MIDIParser();

  MIDIFile getMidiFile(short **midiTable, int frames, unsigned char program = 0, unsigned char channel = 0);
  MIDIFile getMidiFile(vector<vector<int>> midiTable, bool getTempoFromData = false, unsigned char program = 0, unsigned char channel = 0);
};

#endif
