#include <vector>
#include <iostream>
#include "midi.h"
#include "midi-parser.h"

#include "frequencyApprox.h"




using namespace std;
typedef unsigned char uchar;


int main(int argc, char* argv[])
{
	char * filename;
	int windowSize =1000;
	int windowDistance =200;
	int zeroPadding =0;
	int maxNotes = 88; //limit midi messages per time unit to enable playing it with midi hardware interface
	int noteSwitchThreshold = 4; // don't switch on too similar notes ... less midi Messages
	int minVelocity = 5; //don't play too silent notes ... less midi Messages
	bool writeCSV = true;
	if(argc < 1){
		cerr << "At least one input Argument expected. Specify the File to Convert" << endl;
		return -1;
	}else if(argc > 7){
		cerr << "Too many Input Arguments" << endl;
		return -1;
	}else{
	if(argc >= 1){//TODO: switch case for readability
		filename =  argv[1];
	}
	if(argc >= 2){
		windowSize = atoi(argv[2]);
	}
	if(argc >= 3){
		windowDistance = atoi(argv[3]);	
	}
	if(argc >= 4){
		zeroPadding = atoi(argv[4]);
	}
	if(argc >= 5){
		maxNotes = atoi(argv[5]);
	}
	//writeCSV = argv[6];	
	}
	std::cout << "generating: " << argv[1] << " ,windowSize " << std::to_string(windowSize) << " ,windowDistance " << std::to_string(windowDistance) ;
	std::cout << ", zeroPadding TBD "<< ", maxNotes " << std::to_string(maxNotes) << std::endl;
	short ** midiTable;
	FrequencyApprox approx{};
	int midiTempo, frames;
	midiTable = approx.toMIDI(argv[1], windowSize, windowDistance, zeroPadding, writeCSV, midiTempo , frames);
	uint16_t ppq =460;
	MIDIParser parse{midiTempo, ppq};
	MIDITrack track = parse.getMidiTrack(midiTable,frames,maxNotes, noteSwitchThreshold, minVelocity, 0,0);
	MIDIFile result;
	result.addTrack(track);
    result.generate();
	std::string file = std::string(filename);	
	result.saveAs(file.replace(file.find(".wav"),4,".mid").c_str());
			
}
