#include <fstream>
#include <iostream>
#include <string>

#include "midi.h"

using namespace std;

const uint32_t DEFAULT_TEMPO = 500 * 1000; // microseconds for one quarter note / beat
const uint16_t DEFAULT_PPQ = 480;          // parts per quarter note

/*********** MIDI BASE *******************************/

void MIDIBase::pushBackNumber(size_t data, size_t numberOfBytes)
{
    for (signed i = numberOfBytes - 1; i >= 0; --i) // MSB inserted before LSB -> results in big endian!
    {
        insert(end(), (uchar)(data >> (8 * i)));
    }
}

void MIDIBase::pushBackString(const char *s)
{
    insert(end(), s, s + string(s).length());
}

template <typename... Args>
void MIDIBase::pushBackBytes(uchar value1, Args... moreValues)
{
    push_back(value1);
    pushBackBytes(moreValues...);
}
void MIDIBase::pushBackBytes() {}

/*********** MIDI TRACK *******************************/

MIDITrack::MIDITrack(uint32_t tempo) : MIDIBase(), delay(0), tempo(tempo)
{
    // set tempo
    addMetaEvent(0x51, 3, (tempo >> 16) & 0xFF, (tempo >> 8) & 0xFF, tempo & 0xFF);
}
MIDITrack::MIDITrack() : MIDITrack(DEFAULT_TEMPO) // default tempo = DEFAULT_TEMPO
{
}

/** private methods **/

template <typename... Args>
void MIDITrack::addEventRaw(Args... data)
{
    flush();
    pushBackBytes(data...);
}

void MIDITrack::addEvent(uchar status, uchar data1)
{
    addEventRaw(status, data1 & 0x7F);
}

void MIDITrack::addEvent(uchar status, uchar data1, uchar data2)
{
    addEventRaw(status, data1 & 0x7F, data2 & 0x7F);
}

void MIDITrack::addTimeOffset(unsigned t)
{
    // currently 42 bit offsets can be processes
    // this should be more than enough -> in worst case (tempo = 1 us, ppq = 0x7FFF) this represents a break of 134 seconds
    if (t >> 35)
        push_back(0x80 | ((t >> 35) & 0x7F));
    if (t >> 28)
        push_back(0x80 | ((t >> 28) & 0x7F));
    if (t >> 21)
        push_back(0x80 | ((t >> 21) & 0x7F));
    if (t >> 14)
        push_back(0x80 | ((t >> 14) & 0x7F));
    if (t >> 7)
        push_back(0x80 | ((t >> 7) & 0x7F));
    push_back(((t >> 0) & 0x7F));
}

void MIDITrack::flush()
{
    addTimeOffset(delay);
    delay = 0;
}

/** public methods **/

// Methods for indicating how much time elapses:
void MIDITrack::addDelay(unsigned amount)
{
    delay += amount;
}

void MIDITrack::noteOff(uchar channel, uchar note, uchar velocity)
{
    addEvent(0x80 | channel, note, velocity);
}
void MIDITrack::noteOn(uchar channel, uchar note, uchar velocity)
{
    addEvent(0x90 | channel, note, velocity);
}
void MIDITrack::afterTouchPoly(uchar channel, uchar note, uchar pressure)
{
    addEvent(0xA0 | channel, note, pressure);
}
void MIDITrack::controlChange(uchar channel, uchar key, uchar value)
{
    addEvent(0xB0 | channel, key, value);
}
void MIDITrack::programChange(uchar channel, uchar patch)
{
    addEvent(0xC0 | channel, patch);
}
void MIDITrack::afterTouchChannel(uchar channel, uchar pressure)
{
    addEvent(0xD0 | channel, pressure);
}
void MIDITrack::pitchWheel(uchar channel, uchar pitchLSB, uchar pitchMSB)
{
    addEvent(0xE0 | channel, pitchLSB, pitchMSB);
}

template <typename... Args>
void MIDITrack::addMetaEvent(uchar metatype, uchar length, Args... args)
{
    // TODO: check if meta = 0xFF or 0xF0...0xFF
    // TODO: implement length > 255 (variable length values)
    // TODO: args only 0-127 or 0-255?
    addEventRaw(0xFF, metatype, length, args...);
}

/*********** MIDI FILE *******************************/

MIDIFile::MIDIFile(uint16_t ppq, uint16_t type) : MIDIBase(), ppq(ppq), type(type)
{
}
MIDIFile::MIDIFile(uint16_t ppq) : MIDIFile(ppq, 1) // default: type = 1
{
}
MIDIFile::MIDIFile() : MIDIFile(DEFAULT_PPQ) // default ppq = DEFAULT_PPQ
{
}

void MIDIFile::addTrack(MIDITrack track)
{
    tracks.push_back(track);
}

void MIDIFile::generate()
{
    clear();

    // default MIDI header
    pushBackString("MThd");
    pushBackNumber(6, 4);
    pushBackNumber(type, 2);
    pushBackNumber(tracks.size(), 2);
    pushBackNumber(ppq, 2);

    for (unsigned i = 0; i < tracks.size(); ++i)
    {
        tracks[i].addMetaEvent(0x2F, 0); // mark track end

        pushBackString("MTrk");                            // track marker
        pushBackNumber(tracks[i].size(), 4);               // track length
        insert(end(), tracks[i].begin(), tracks[i].end()); // track data
    }
}

void MIDIFile::saveAs(const char *filePath)
{
    ofstream outfile;
    outfile.open(filePath, ios::out | ios::binary);

    if (!outfile.is_open())
    {
        cerr << "Saving MIDI file failed. Could not open " << filePath << ". Probably a directory does not exist!" << endl;
    }
    else
    {
        outfile.write((char *)&front(), size());
    }

    outfile.close();
}
