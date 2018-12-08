#include <vector>
#include <iostream>
#include <string>

#include "lib/midi.h"
#include "lib/midi-parser.h"
#include "lib/frequencyApprox.h"

/* DEFAULT PARAMETER */
const int WINDOW_SIZE = 1000;
const int WINDOW_DISTANCE = 200;
const int ZERO_PADDING = 0;
const int MAX_NOTES = 88;		   // limit midi messages per time unit to enable playing it with midi hardware interface
const uint32_t TEMPO = 300 * 1000; // microseconds for one quarter note / beat (default = 500 ms = 500 * 1000 us)
const uint16_t PPQ = 10;		   // parts per quarter note -> with default settings (quarter note = 500 ms) PPQ *2(!) = parts per second

using namespace std;
typedef unsigned char uchar;

MIDIFile demo()
{
	const uint32_t pauseUs = TEMPO / 2;

	vector<vector<int>> testVector = {
		{0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0},

		{2 * pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0},
		{2 * pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},

		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0},

		{4 * pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},

		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, 0, 0},

		{4 * pauseUs, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0},

		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0},

		{2 * pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0},
		{2 * pauseUs, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0},

		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0},
		{pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0},
		{pauseUs, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0},

		{4 * pauseUs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

	MIDIParser temp{TEMPO, PPQ};
	temp.noteCount = 22;
	temp.firstNoteIndex = 60;
	return temp.getMidiFile(testVector, false, 4);
}

int main(int argc, char *argv[])
{
	// for demo / testing purposes only
	demo().saveAs("./output/demo.mid");

	// input validation
	if (argc < 2)
	{
		cerr << "At least one input argument is expected. Please provide a file path..." << endl;
		return -1;
	}
	if (argc > 8)
	{
		cerr << "Too many input arguments (" << argc << ")" << endl;
		return -1;
	}

	string inputFilepath = "";
	int windowSize = WINDOW_SIZE;
	int windowDistance = WINDOW_DISTANCE;
	int zeroPadding = ZERO_PADDING;
	int maxNotes = MAX_NOTES;
	uchar minVolume = MIN_VOLUME;
	uchar noteSwitchThreshold = NOTE_SWITCH_THRESHOLD;

	// read args
	for (int i = 1; i < argc; ++i)
	{
		char *value = argv[i];
		switch (i)
		{
		case 1:
			inputFilepath = string(value);
			break;
		case 2:
			windowSize = atoi(value);
			break;
		case 3:
			windowDistance = atoi(value);
			break;
		case 4:
			zeroPadding = atoi(value);
			break;
		case 5:
			maxNotes = atoi(value);
			break;
		case 6:
			minVolume = atoi(value);
			break;
		case 7:
			noteSwitchThreshold = atoi(value);
			break;
		}
	}
	cout << "generating: " << inputFilepath << "\nwindowSize " << to_string(windowSize) << ", windowDistance " << to_string(windowDistance);
	cout << ", zeroPadding " << to_string(zeroPadding) << ", maxNotes " << to_string(maxNotes);
	cout << ", minVolume " << to_string(minVolume) << ", noteSwitchThreshold " << to_string(noteSwitchThreshold) << endl;

	// read audio file, perform fast-fourier-transformation (FFT) and aggregate results to MIDI compatible format
	short **midiTable;
	FrequencyApprox approx{};
	int frames, midiTempo;
	midiTable = approx.toMIDI(inputFilepath.c_str(), windowSize, windowDistance, zeroPadding, midiTempo, frames);
	cout << "read audio file, performed FFT and aggregated MIDI data" << endl;

	// parse data to MIDI object
	uint32_t tempo = midiTempo;
	MIDIParser parser{tempo, PPQ, maxNotes, minVolume, noteSwitchThreshold};
	MIDIFile midiFile = parser.getMidiFile(midiTable, frames);
	cout << "generated MIDI file" << endl;

	// save as MIDI file
	size_t slashIndex = inputFilepath.find_last_of("/\\");
	size_t dotIndex = inputFilepath.find_last_of(".");
	// get file name without parent directories and file extension
	string inputFilename = inputFilepath.substr(slashIndex + 1, dotIndex - slashIndex - 1);
	// add parameter information to file names
	string filenameSuffix1 = "-" + to_string(windowSize) + "-" + to_string(windowDistance) + "-" + to_string(zeroPadding);
	string filenameSuffix2 = "-" + to_string(maxNotes) + "-" + to_string(minVolume) + "-" + to_string(noteSwitchThreshold);
	
	string outputFilename = "./output/" + inputFilename + filenameSuffix1 + filenameSuffix2 + ".mid";
	midiFile.saveAs(outputFilename.c_str());
	cout << "saved MIDI file" << endl;
}
