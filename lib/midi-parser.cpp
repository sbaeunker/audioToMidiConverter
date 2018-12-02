#include <string>
#include <vector>
#include <array>
#include <sstream>   //istringstream
#include <iostream>  // cout
#include <fstream>   // filestream
#include <algorithm> //partial_sort
#include "midi-parser.h"

using namespace std;

const uint32_t DEFAULT_TEMPO = 500 * 1000; // microseconds for one quarter note / beat
const uint16_t DEFAULT_PPQ = 480;          // parts per quarter note

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq, uchar minVolume, uchar noteSwitchThreshold)
    : tempo(tempo), ppq(ppq),
      minVolume(minVolume), noteSwitchThreshold(noteSwitchThreshold),
      noteCount(NOTE_COUNT), firstNoteIndex(FIRST_NOTE_INDEX)
{
}

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq) : MIDIParser(tempo, ppq, MIN_VOLUME, NOTE_SWITCH_THRESHOLD)
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

vector<int> MIDIParser::getLargest(vector<int> nextVolumes, vector<uchar> lastVolumes, int maxNoteCount)
{
    size_t length = nextVolumes.size() < lastVolumes.size() ? nextVolumes.size() : lastVolumes.size();
    if (maxNoteCount > length)
        maxNoteCount = length;

    vector<int> diffs(length); // list of absolute differences between to-be and as-is volume
    for (unsigned i = 0; i < length; ++i)
    {
        diffs[i] = abs(nextVolumes[i] - lastVolumes[i]); //absolute differences
    }

    vector<int> temp(length); // simple array of 1,2,3,4,...,maxNoteCount
    for (unsigned i = 0; i < length; ++i)
    {
        temp[i] = i;
    }

    // sort temp after largest values in v
    partial_sort(temp.begin(), temp.begin() + maxNoteCount, temp.end(), comp(diffs));
    return temp; // temp[0..maxNoteCount] contains index of keys to play
}

MIDITrack MIDIParser::getMidiTrack(short **midiTable, int frames, int maxNoteCount, unsigned char program, unsigned char channel)
{
    MIDITrack track{tempo};
    track.programChange(channel, program);

    vector<uchar> noteStatus;
    noteStatus.assign(noteCount, 0);

    vector<int> largestDiffs; //stores the most important notes to play
    largestDiffs.assign(maxNoteCount, 0);

    for (unsigned frame = 0; frame < frames; ++frame)
    {
        vector<int> row(midiTable[frame], midiTable[frame] + noteCount);
        largestDiffs = getLargest(row, noteStatus, maxNoteCount);
        for (unsigned i = 0; i < maxNoteCount; ++i) //only play most differing keys
        {
            int noteIndex = largestDiffs[i];
            int volume = row[noteIndex];
            int lastVolume = noteStatus[noteIndex];
            if (abs(volume - lastVolume) >= noteSwitchThreshold)
            {
                track.noteOff(channel, firstNoteIndex + noteIndex); //first turn note off
                noteStatus[noteIndex] = 0;
                if (volume >= minVolume)
                {
                    track.noteOn(channel, firstNoteIndex + noteIndex, volume);
                    noteStatus[noteIndex] = volume;
                }
            }
        }
        track.addDelay(ppq); // blame
    }
    return track;
}

MIDITrack MIDIParser::getMidiTrack(bool getTempoFromData, unsigned char program, unsigned char channel)
{
    if (getTempoFromData && rawData.size() > 2 && rawData[1].size() > 1)
        tempo = rawData[1][0];

    MIDITrack track{tempo};
    track.programChange(channel, program);

    vector<uchar> noteStatus;
    noteStatus.assign(noteCount, 0);

    for (unsigned i = 0; i < rawData.size(); ++i)
    {
        vector<int> rowData = rawData[i];

        if (rowData.size() != noteCount + 1)
        {
            cout << "Invalid CSV row detected. Rows must contain 1 + " << noteCount << " entries (not " << rowData.size() << ")!" << endl;
        }
        else
        {
            uint32_t delay = rowData[0];               // delay in us
            unsigned ppqDelay = ppq * delay / (tempo); // transform delay in us to delay in PPQ
            track.addDelay(ppqDelay);

            for (unsigned note = 0; note < rowData.size() - 1; ++note)
            {
                uchar volume = rowData[note + 1] & 0xFF;
                // noteOff, if it will change (diff to previous value > threshold) or volume too low
                if (abs(noteStatus[note] - volume) >= noteSwitchThreshold)
                {
                    track.noteOff(channel, firstNoteIndex + note);
                    noteStatus[note] = 0;

                    if (volume >= minVolume) // noteOn, if volume > minVolume
                    {
                        track.noteOn(channel, firstNoteIndex + note, volume);
                        noteStatus[note] = volume;
                    }
                }
            }
        }
    }
    return track;
}

MIDIFile MIDIParser::getMidiFile(short **midiTable, int frames, int maxNoteCount, unsigned char program, unsigned char channel)
{
    MIDIFile file{ppq};
    MIDITrack track = getMidiTrack(midiTable, frames, maxNoteCount, program, channel);

    file.addTrack(track);
    file.generate();

    return file;
}

MIDIFile MIDIParser::getMidiFile(bool getTempoFromData, unsigned char program, unsigned char channel)
{
    MIDIFile file{ppq};
    MIDITrack track = getMidiTrack(getTempoFromData, program, channel);

    file.addTrack(track);
    file.generate();

    return file;
}

// legacy helper method to get MIDI file from csv input
// inspired by: https://waterprogramming.wordpress.com/2017/08/20/reading-csv-files-in-c/
void MIDIParser::loadRawDataFromCsv(const char *filename, char delimiter)
{
    rawData.clear();
    ifstream inputFile{filename};
    int lineIndex = 0;

    while (inputFile)
    {
        ++lineIndex;
        string line;
        if (!getline(inputFile, line))
            break;
        if (line[0] != '#')
        {
            istringstream lineStream{line};
            vector<int> record;

            while (lineStream)
            {
                string element;
                if (!getline(lineStream, element, delimiter))
                    break;
                try
                {
                    record.push_back(stoi(element));
                }
                catch (const invalid_argument e)
                {
                    cout << "NaN found in file " << filename << " line " << lineIndex << endl;
                    e.what();
                }
            }

            rawData.push_back(record);
        }
    }

    if (!inputFile.eof())
    {
        cerr << "Could not read file " << filename << "\n";
    }
}

// legacy method to get MIDI file from csv input
MIDIFile MIDIParser::getMidiFile(const char *filename, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    cout << "getting MIDI file from csv file " << filename << endl;
    loadRawDataFromCsv(filename);
    return getMidiFile(getTempoFromData, program, channel);
}
