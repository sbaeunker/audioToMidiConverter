#include <vector>
#include <iostream>
#include <string>

#include "lib/midi.h"
#include "lib/midi-parser.h"
#include "lib/frequencyApprox.h"

using namespace std;
typedef unsigned char uchar;

int main(int argc, char *argv[])
{
	string inputFilepath = "";
	int windowSize = 1000;
	int windowDistance = 200;
	int zeroPadding = 0;
	int maxNotes = 88;			 //limit midi messages per time unit to enable playing it with midi hardware interface
	int noteSwitchThreshold = 4; // don't switch on too similar notes ... less midi Messages
	int minVelocity = 5;		 //don't play too silent notes ... less midi Messages
	bool writeCSV = true;
	if (argc < 2)
	{
		cerr << "At least one input Argument expected. Specify the File to Convert" << endl;
		return -1;
	}
	else if (argc > 7)
	{
		cerr << "Too many Input Arguments" << endl;
		return -1;
	}
	else
	{
		if (argc >= 1)
		{ //TODO: switch case for readability
			inputFilepath = string(argv[1]);
		}
		if (argc >= 2)
		{
			windowSize = atoi(argv[2]);
		}
		if (argc >= 3)
		{
			windowDistance = atoi(argv[3]);
		}
		if (argc >= 4)
		{
			zeroPadding = atoi(argv[4]);
		}
		if (argc >= 5)
		{
			maxNotes = atoi(argv[5]);
		}
		//writeCSV = argv[6];
	}
	std::cout << "generating: " << inputFilepath << " ,windowSize " << std::to_string(windowSize) << " ,windowDistance " << std::to_string(windowDistance);
	std::cout << ", zeroPadding TBD "
			  << ", maxNotes " << std::to_string(maxNotes) << std::endl;

	short **midiTable;
	FrequencyApprox approx{};
	int frames;
	int midiTempo;
	midiTable = approx.toMIDI(argv[1], windowSize, windowDistance, zeroPadding, writeCSV, midiTempo, frames);

	std::cout << "save as MIDI file" << std::endl;
	uint16_t ppq = 460;
	uint32_t tempo = midiTempo;
	MIDIParser parse{tempo, ppq};
	MIDITrack track = parse.getMidiTrack(midiTable, frames, maxNotes, noteSwitchThreshold, minVelocity, 0, 0);

	MIDIFile result;
	result.addTrack(track);
	result.generate();

	// save as MIDI file
	std::cout << "save as MIDI file" << std::endl;
	size_t tempIndex = inputFilepath.find_last_of("/\\");
	string inputFilename = inputFilepath.substr(tempIndex + 1);
	string outputFilename = "./output/" + inputFilename.substr(0, inputFilename.length() - 4) + ".mid";
	result.saveAs(outputFilename.c_str());
}
