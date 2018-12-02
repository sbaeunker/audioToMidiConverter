#include <string>
#include <vector>
#include <array>
#include <sstream>   //istringstream
#include <iostream>  // cout
#include <fstream>   // filestream
#include <algorithm> //partial_sort
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

vector<int> MIDIParser::getLargest(vector<int> nextVolumes, vector<uchar> lastVolumes, int noteCount)
{
    size_t length = nextVolumes.size() < lastVolumes.size() ? nextVolumes.size() : lastVolumes.size();
    if (noteCount > length)
        noteCount = length;

    vector<int> diffs(length); // list of absolute differences between to-be and as-is volume
    for (unsigned i = 0; i < length; ++i)
    {
        diffs[i] = abs(nextVolumes[i] - lastVolumes[i]); //absolute differences
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
            cout << "Invalid CSV row detected. Rows must contain 1 + " << noteCount << " entries (not " << rowData.size() << ")!" << endl;
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

// legacy helper method to get MIDI file from csv input
// inspired by: https://waterprogramming.wordpress.com/2017/08/20/reading-csv-files-in-c/
vector<vector<int>> MIDIParser::getMidiDataFromCsv(const char *filename, char delimiter)
{
    vector<vector<int>> data;
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

            data.push_back(record);
        }
    }
    if (!inputFile.eof())
    {
        cerr << "Could not read file " << filename << "\n";
    }
    return data;
}

MIDIFile MIDIParser::getMidiFile(vector<vector<int>> midiTable, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    MIDITrack track = getMidiTrack(midiTable, getTempoFromData, program, channel);

    MIDIFile file{ppq};
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

// legacy method to get MIDI file from csv input
MIDIFile MIDIParser::getMidiFileFromCsv(const char *filename, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    cout << "getting MIDI file from csv file " << filename << endl;
    vector<vector<int>> data = getMidiDataFromCsv(filename);
    return getMidiFile(data, getTempoFromData, program, channel);
}
