#include <string>
#include <vector>
#include <array>
#include <sstream>  //istringstream
#include <iostream> // cout
#include <fstream>  // ifstream
#include <list>
#include "midi-parser.h"

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
/*
MIDITrack MIDIParser::getMidiTrack(bool getTempoFromData, unsigned char program, unsigned char channel, bool holding)
{
	if (getTempoFromData && rawData.size() > 2 && rawData[1].size() > 1)
		tempo = rawData[1][0];

	int tempoScaling = 1;
	MIDITrack track{ tempoScaling*tempo };
	track.programChange(channel, program);

	int noteSwitchThreshold = 5;
	int maxParallelNotes = 40;
	std::list<int> currentParallelNoteVelocity(maxParallelNotes, 0);
	std::list<int> currentParallelNotes(maxParallelNotes, 0);
	int minVelocity = 16;

	vector<int> noteStatus;
	noteStatus.assign(noteCount, 0);

	uchar velocity;
	uint16_t messageCount = 0;
	unsigned int delayCount = 0;

	
	std::cout << "PPQ: " << std::to_string(ppq) << " microS pro Q: " << std::to_string(tempo) << " ";

	for (unsigned i = 0; i < rawData.size(); ++i) //read each line
	{
		vector<int> rowData = rawData[i];

		currentParallelNotes.assign(maxParallelNotes, 0);
		currentParallelNoteVelocity.assign(maxParallelNotes, 0);

		if (rowData.size() != noteCount + 1)
		{
			std::cout << "Invalid CSV row detected. Rows must contain 1 + " << noteCount << " entries (not " << rowData.size() << ")!" << endl;
		}
		else
		{
			
			
			//find the largest upcoming note changes
			for (unsigned note = 0; note < rowData.size() - 1; ++note)
			{
				velocity = rowData[note + 1] & 0xFF;
				
				for (std::list<int>::iterator it = currentParallelNoteVelocity.begin(); it != currentParallelNoteVelocity.end(); it++) //find the largest changes
				{	
					if (abs(noteStatus[note] - velocity) > *it ) {
							currentParallelNoteVelocity.insert(it, velocity);
							std::list<int>::iterator ind = currentParallelNotes.begin();
							std::advance(ind, std::distance(currentParallelNoteVelocity.begin(), it));
							currentParallelNotes.insert(ind, note);
							currentParallelNoteVelocity.pop_back();
							currentParallelNotes.pop_back();
							break;
					}
				}			
			}

			// step 1: add delay / time offset to previous step
			uint32_t delay = rowData[0];             // delay in us	 //delayCount += delay;
			unsigned ppqDelay = ppq * delay / (tempoScaling * tempo); // transform delay in us to delay in PPQ
			delayCount += delay;
			for (std::list<int>::iterator it = currentParallelNotes.begin(); it != currentParallelNotes.end(); it++) //iterate through previously found changes
			{

				
				track.noteOff(channel, firstNoteIndex + *it); //all notes off
				messageCount++;
				noteStatus[*it] = 0;
				std::list<int>::iterator ind = currentParallelNoteVelocity.begin();
				std::advance(ind, std::distance(currentParallelNotes.begin(), it));
				if (*ind > minVelocity) //important notes back on
				{
					std::cout << " n: "  <<std::to_string(*it) << " v: " << std::to_string(*ind);
					messageCount++;
					track.noteOn(channel, firstNoteIndex + *it, *ind);
					noteStatus[*it] = *ind;
				}
				
			}
			std::cout << endl;
		}
		
	}
	double bitRate = 24.0 *messageCount / (delayCount / 1000000); //assuming 24 bit messages
	std::cout << "bitRate: " << std::to_string(bitRate) << "bit/s" << endl;
	return track;

}

*/





MIDITrack MIDIParser::getMidiTrack(bool getTempoFromData, unsigned char program, unsigned char channel, bool holding)
{
    if (getTempoFromData && rawData.size() > 2 && rawData[1].size() > 1)
        tempo = rawData[1][0];

	int tempoScaling = 1;
    MIDITrack track{tempoScaling*tempo};
    track.programChange(channel, program);

	int noteSwitchThreshold = 5;
    vector<int> noteStatus;
    noteStatus.assign(noteCount, false);

	uchar velocity;
	uint16_t messageCount = 0;
	unsigned int delayCount = 0;

	/*
	unsigned long preDelay = 28000000; //preDelay in us to lower Average Bitrate
	track.addDelay(ppq* preDelay / (tempoScaling * tempo));
	delayCount += preDelay;
	*/
	
	std::cout << "PPQ: " << std::to_string(ppq) << " microS pro Q: " << std::to_string(tempo) << " ";

	int maxParallelNotes = 10;
	int currentParallelNotes = 0;
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
            unsigned ppqDelay = ppq * delay / (tempoScaling * tempo); // transform delay in us to delay in PPQ

            

            // step 2: switch all previously switched on notes off
            for (unsigned note = 0; note < rowData.size() - 1; ++note)
            {
				if (holding) { //only play notes on threshold, otherwise hold the note
					velocity = rowData[note + 1] & 0xFF;
					if (abs(noteStatus[note] - velocity)>noteSwitchThreshold) // noteOff, if it was activated in the previous step!
					{
						track.noteOff(channel, firstNoteIndex + note);
						messageCount++;
						currentParallelNotes++;
						noteStatus[note] = 0;
						/*
						if (currentParallelNotes > maxParallelNotes) {
							currentParallelNotes = 0;
							break;
						}*/
						
					}
				}else if (noteStatus[note]) // noteOff, if it was activated in the previous step!
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
				if (holding) { //only play notes on threshold, otherwise hold the note
					if (abs(noteStatus[note] - velocity)>noteSwitchThreshold && velocity > minVelocity) //play notes when: Diff greater Threshold and greater minVelocity
					{
						track.noteOn(channel, firstNoteIndex + note, velocity);
						messageCount++;
						noteStatus[note] = velocity;
					}
				}else if (velocity)
                {
                    track.noteOn(channel, firstNoteIndex + note, velocity);
					messageCount++;
                    noteStatus[note] =velocity;
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

MIDIFile MIDIParser::getMidiFile(bool getTempoFromData, unsigned char program, unsigned char channel, bool holding)
{
    MIDIFile file{ppq};
    MIDITrack track = getMidiTrack(getTempoFromData, program, channel, holding);

    file.addTrack(track);
    file.generate();

    return file;
}

MIDIFile MIDIParser::getMidiFile(const char *filename, bool getTempoFromData, unsigned char program, unsigned char channel, bool holding)
{
    loadRawDataFromCsv(filename);
	cout << filename;
	if (holding) cout << " holding";
	cout << endl;
    return getMidiFile(getTempoFromData, program, channel, holding);
}