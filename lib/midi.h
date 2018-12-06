
#include <vector>

#ifndef __MIDI_H__
#define __MIDI_H__

using namespace std;

typedef unsigned char uchar;

/**
 * Basically a wrapper for a byte vector with protected helper methods to easily append numbers, strings, etc.
 */
class MIDIBase : public vector<uchar>
{
  protected:
    void pushBackNumber(size_t data, size_t numberOfBytes);

    void pushBackString(const char *s);

    template <typename... Args>
    void pushBackBytes(uchar value1, Args... moreValues);
    void pushBackBytes();
};

/* Represents a single MIDI Track */
class MIDITrack : public MIDIBase
{
  private:
    unsigned long long delay;
    uint32_t tempo;

    template <typename... Args>
    void addEventRaw(Args... data);

    void addEvent(uchar status, uchar data1);
    void addEvent(uchar status, uchar data1, uchar data2);

    void addTimeOffset(unsigned long long t);

    void flush();

  public:
    MIDITrack(uint32_t tempo);
    MIDITrack();

    // Methods for indicating how much time elapses:
    void addDelay(unsigned long long amount);

    void noteOff(uchar channel, uchar note, uchar velocity = 0);
    void noteOn(uchar channel, uchar note, uchar velocity);
    void afterTouchPoly(uchar channel, uchar note, uchar pressure);
    void controlChange(uchar channel, uchar key, uchar value);
    void programChange(uchar channel, uchar patch);
    void afterTouchChannel(uchar channel, uchar pressure);
    void pitchWheel(uchar channel, uchar pitchLSB, uchar pitchMSB);

    template <typename... Args>
    void addMetaEvent(uchar metatype, uchar length, Args... args);

    // TODO: implement SysEx endpoints?
};

/* Represents a MIDI file */
class MIDIFile : public MIDIBase
{
  private:
    uint16_t ppq;
    uint16_t type;

  protected:
    vector<MIDITrack> tracks;

  public:
    MIDIFile(uint16_t ppq, uint16_t type);
    MIDIFile(uint16_t ppq);
    MIDIFile();

    void addTrack(MIDITrack track);

    void generate();

    void saveAs(const char *filePath = "midi-parser.mid");
};

#endif