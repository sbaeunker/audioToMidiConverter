#include <string>
#include <vector>
#include <array>
#include <sstream>  //istringstream
#include <iostream> // cout
#include <fstream>  // filestream
#include <list>
#include "midi-parser.h"

#define NO_KEYS 88

const uint32_t DEFAULT_TEMPO = 500 * 1000; // microseconds for one quarter note / beat
const uint16_t DEFAULT_PPQ = 480;          // parts per quarter note

MIDIParser::MIDIParser(uint32_t tempo, uint16_t ppq) : tempo(tempo), ppq(ppq), noteCount(NOTE_COUNT), firstNoteIndex(FIRST_NOTE_INDEX)
{
}

MIDIParser::MIDIParser() : MIDIParser(DEFAULT_TEMPO, DEFAULT_PPQ)
{
}

// in Anlehnung an: https://waterprogramming.wordpress.com/2017/08/20/reading-csv-files-in-c/
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
MIDITrack MIDIParser::getMidiTrack(short ** midiTable,int frames, unsigned char program, unsigned char channel){
	std::cout << "PPQ: " << std::to_string(ppq) << " microS pro Q: " << std::to_string(tempo) << endl;
	MIDITrack track{tempo};
	track.programChange(channel, program);
	int noteSwitchThreshold = 4; // don't switch on too similar notes ... less midi Messages
	int minVelocity = 3; //don't play too silent notes ... less midi Messages
    vector<int> noteStatus;
    noteStatus.assign(noteCount, 0);
    uchar velocity;
    for(int frame = 0; frame < frames; frame++){
		for(int note = 0; note<NO_KEYS; note++){			
			if (abs(noteStatus[note] - midiTable[frame][note])>noteSwitchThreshold){
				track.noteOff(channel, firstNoteIndex + note);
				noteStatus[note] = 0;
				if(midiTable[frame][note]>minVelocity){
					track.noteOn(channel, firstNoteIndex + note, midiTable[frame][note]);
					noteStatus[note] = midiTable[frame][note];
				}
			}
		}
		track.addDelay(ppq);
	}   
    return track;
}

MIDITrack MIDIParser::getMidiTrack(bool getTempoFromData, unsigned char program, unsigned char channel)
{
    if (getTempoFromData && rawData.size() > 2 && rawData[1].size() > 1)
        tempo = rawData[1][0];


    MIDITrack track{tempo};
    track.programChange(channel, program);

	int noteSwitchThreshold = 5;
    vector<int> noteStatus;
    noteStatus.assign(noteCount, 0);

	uchar velocity;
	uint16_t messageCount = 0;
	unsigned int delayCount = 0;

	//std::cout << "PPQ: " << std::to_string(ppq) << " microS pro Q: " << std::to_string(tempo) << " ";

	int minVelocity = 16;

    for (unsigned i = 0; i < rawData.size(); ++i)	
    {
        vector<int> rowData = rawData[i];

        if (rowData.size() != noteCount + 1)
        {
            std::cout << "Invalid CSV row detected. Rows must contain 1 + " << noteCount << " entries (not " << rowData.size() << ")!" << endl;
        }
        else
        {
            // step 1: add delay / time offset to previous step
            uint32_t delay = rowData[0];             // delay in us
			//delayCount += delay;
            unsigned ppqDelay = ppq * delay / (tempo); // transform delay in us to delay in PPQ

            

            // step 2: switch all previously switched on notes off
            for (unsigned note = 0; note < rowData.size() - 1; ++note)
            {
				
					velocity = rowData[note + 1] & 0xFF;
					if (abs(noteStatus[note] - velocity)>noteSwitchThreshold) // noteOff, if it was activated in the previous step!
					{
						track.noteOff(channel, firstNoteIndex + note);
						messageCount++;
						noteStatus[note] = 0;						
					}
				
				if (note % 4 == 0) {
					messageCount++;
					delayCount += ppqDelay / (4* 11);
					track.addDelay(ppqDelay / (4* 11)); //spread playing across multiple ticks -> more delay messages
				}
				
            }
			
            // step 3: switch current notes on
            for (unsigned note = 0; note < rowData.size() - 1; ++note)
            {
                velocity = rowData[note + 1] & 0xFF;
				
					if (abs(noteStatus[note] - velocity)>noteSwitchThreshold && velocity > minVelocity) //play notes when: Diff greater Threshold and greater minVelocity
					{
						track.noteOn(channel, firstNoteIndex + note, velocity);
						messageCount++;
						noteStatus[note] = velocity;
					}
								
				if (note % 4 == 0) {
					messageCount++;
					delayCount += delay / (4* 11);
					track.addDelay(ppqDelay / (4* 11)); //spread playing across multiple ticks -> more delay messages
				}
            }
        }
    }
	double bitRate = 24.0 *messageCount / (delayCount / 1000000); //assuming 24 bit messages
	cout << "bitRate: " << std::to_string(bitRate) << "bit/s" << endl;
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

MIDIFile MIDIParser::getMidiFile(const char *filename, bool getTempoFromData, unsigned char program, unsigned char channel)
{
    loadRawDataFromCsv(filename);
	cout << filename<< endl;
    return getMidiFile(getTempoFromData, program, channel);
}
