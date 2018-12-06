#include <string>
#include <vector>
#include <iostream>  // cout
#include <algorithm> // partial_sort

#include "midi-parser.h"

using namespace std;

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq, int maxNoteCount, uchar minVolume, uchar noteSwitchThreshold)
    : tempo(tempo), ppq(ppq), maxNoteCount(maxNoteCount),
      minVolume(minVolume), noteSwitchThreshold(noteSwitchThreshold),
      noteCount(NOTE_COUNT), firstNoteIndex(FIRST_NOTE_INDEX)
{
}

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq, int maxNoteCount) : MIDIParser(tempo, ppq, maxNoteCount, MIN_VOLUME, NOTE_SWITCH_THRESHOLD)
{
}

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq) : MIDIParser(tempo, ppq, NOTE_COUNT)
{
}

MIDIParser::MIDIParser() : MIDIParser(DEFAULT_TEMPO, DEFAULT_PPQ)
{
}

struct MIDIParser::comp
{
    comp(const vector<int> &v) : _v(v) {}
    bool operator()(int a, int b) { return _v[a] > _v[b]; }
    const vector<int> &_v;
};

vector<int> MIDIParser::getLargest(vector<int> rowData, vector<uchar> noteStatus, int noteCount)
{
    // NOTE: rowData contains tempo information at index 0 -> note information start at index 1!
    size_t length = rowData.size() - 1 < noteStatus.size() ? rowData.size() - 1 : noteStatus.size();
    if (noteCount > length)
        noteCount = length;

    vector<int> diffs(length); // list of absolute differences between to-be and as-is volume
    for (unsigned i = 0; i < length; ++i)
    {
        diffs[i] = abs(rowData[i + 1] - noteStatus[i]); //absolute differences
    }

    vector<int> temp(length); // simple array of 1,2,3,4,...,noteCount
    for (unsigned i = 0; i < length; ++i)
    {
        temp[i] = i;
    }

    // sort temp after largest values in v
    partial_sort(temp.begin(), temp.begin() + noteCount, temp.end(), comp(diffs));
    return temp; // temp[0..noteCount] contains index of keys to play
}

MIDITrack MIDIParser::getMidiTrack(vector<vector<int>> data, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    if (getTempoFromData && data.size() > 2 && data[1].size() > 1)
        tempo = data[1][0];

    MIDITrack track{tempo};
    track.programChange(channel, program);

    vector<uchar> noteStatus;
    noteStatus.assign(noteCount, 0);

    vector<int> largestDiffs(maxNoteCount); //stores the most important notes to play

    for (unsigned i = 0; i < data.size(); ++i)
    {
        vector<int> rowData = data[i];

        if (rowData.size() != noteCount + 1)
        {
            cerr << "Invalid CSV row detected. Rows must contain 1 + " << noteCount << " entries (not " << rowData.size() << ")!" << endl;
        }
        else
        {
            uint32_t delay = rowData[0];               // delay in us
            unsigned ppqDelay = ppq * delay / (tempo); // transform delay in us to delay in PPQ
            track.addDelay(ppqDelay);

            largestDiffs = getLargest(rowData, noteStatus, maxNoteCount);

            for (unsigned k = 0; k < rowData.size() - 1 && k < maxNoteCount; ++k)
            {
                int noteIndex = largestDiffs[k];
                uchar volume = rowData[noteIndex + 1] & 0xFF;
                uchar lastVolume = noteStatus[noteIndex];
                // noteOff, if it will change (diff to previous value > threshold) or volume too low
                if (abs(lastVolume - volume) >= noteSwitchThreshold)
                {
                    track.noteOff(channel, firstNoteIndex + noteIndex);
                    noteStatus[noteIndex] = 0;

                    if (volume >= minVolume) // noteOn, if volume > minVolume
                    {
                        track.noteOn(channel, firstNoteIndex + noteIndex, volume);
                        noteStatus[noteIndex] = volume;
                    }
                }
            }
        }
    }

    return track;
}

MIDIFile MIDIParser::getMidiFile(vector<vector<int>> midiTable, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    MIDITrack track = getMidiTrack(midiTable, getTempoFromData, program, channel);

    MIDIFile file{ppq, 0}; // because we only add 1(!) track, MIDI file format 0 is ok
    file.addTrack(track);
    file.generate();

    return file;
}

MIDIFile MIDIParser::getMidiFile(short **midiTable, int frames, unsigned char program, unsigned char channel)
{
    vector<vector<int>> data;
    // parse short** to vector<vector<int>> and insert tempo information
    for (unsigned frame = 0; frame < frames; ++frame)
    {
        vector<int> row(midiTable[frame], midiTable[frame] + noteCount);
        // insert delay at the beginning
        row.insert(row.begin(), tempo); // blame/TODO: check delay parameter
        data.push_back(row);
    }
    return getMidiFile(data, false, program, channel);
}
