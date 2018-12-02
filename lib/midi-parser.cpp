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

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq) : tempo(tempo), ppq(ppq), noteCount(NOTE_COUNT), firstNoteIndex(FIRST_NOTE_INDEX)
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

vector<int> MIDIParser::getLargest(vector<int> midiRow, vector<uchar> playedNotes, int maxNotes)
{
    size_t length = midiRow.size() < playedNotes.size() ? midiRow.size() : playedNotes.size();

    cout << length << endl;

    vector<int> v(length);

    cout << length << endl;
    for (unsigned i = 0; i < length; ++i)
    {
        v[i] = abs(midiRow[i] - playedNotes[i]); //absolute differences
    }
    cout << length << endl;

    vector<int> vx(length);
    for (unsigned i = 0; i < length; ++i)
    {
        vx[i] = i; // fill with indexes
    }

    partial_sort(vx.begin(), vx.begin() + maxNotes, vx.end(), comp(v)); // sort vx after largest values in v
    return vx;                                                          // vx[0..maxNotes] contains index of keys to play
}

vector<int> MIDIParser::getLargest(short *midiRow, vector<uchar> playedNotes, int maxNotes)
{
    vector<int> v(midiRow, midiRow + NOTE_COUNT);
    for (int i = 0; i < v.size(); ++i)
    {
        v[i] = abs(v[i] - playedNotes[i]); //absolute differences
    }
    vector<int> vx;
    vx.resize(v.size());
    for (int i = 0; i < v.size(); ++i)
        vx[i] = i;                                               //fill with indexes
    partial_sort(vx.begin(), vx.begin() + 5, vx.end(), comp(v)); //sort vx after largest values in v
    return vx;                                                   //vx[0..maxNotes] contains index of keys to play
}

MIDITrack MIDIParser::getMidiTrack(short **midiTable, int frames, int maxNotes, int noteSwitchThreshold, int minVolume, unsigned char program, unsigned char channel)
{
    MIDITrack track{tempo};
    track.programChange(channel, program);

    vector<uchar> noteStatus;
    noteStatus.assign(noteCount, 0);

    vector<int> largestDiffs; //stores the most important notes to play
    largestDiffs.assign(maxNotes, 0);

    for (unsigned frame = 0; frame < frames; ++frame)
    {
        short *row = midiTable[frame];
        vector<int> row2(row, row + noteCount);
        cout << "test " << row2.size() << endl;
        largestDiffs = getLargest(row, noteStatus, maxNotes);
        for (unsigned i = 0; i < maxNotes; ++i) //only play most differing keys
        {
            int noteIndex = largestDiffs[i];
            // int noteIndex = i;
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

    int noteSwitchThreshold = 5; // blame
    int minVolume = 16;          // blame

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
                if (abs(noteStatus[note] - volume) >= noteSwitchThreshold || volume < minVolume)
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
