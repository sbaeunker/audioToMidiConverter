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
	bool writeCSV = true;
	if(argc < 1){
		cerr << "At least one input Argument expected. Specify the File to Convert" << endl;
		return -1;
	}else if(argc > 6){
		cerr << "Too many Input Arguments" << endl;
		return -1;
	}else{
	if(argc >= 1){
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
		//writeCSV = atoi(argv[4]);
	}			
	}
	std::cout << "generating: " << argv[1] << " ,windowSize " << std::to_string(windowSize) << " ,windowDistance " << std::to_string(windowDistance) << endl;
	short ** midiTable;
	FrequencyApprox approx{};
	int midiTempo, frames;
	midiTable = approx.toMIDI(argv[1], windowSize, windowDistance, zeroPadding, writeCSV, midiTempo , frames);
	uint16_t ppq =460;
	MIDIParser parse{midiTempo, ppq};
	MIDITrack track = parse.getMidiTrack(midiTable,frames, 0,0);
	MIDIFile result;
	result.addTrack(track);
    result.generate();
	std::string file = std::string(filename);	
	result.saveAs(file.replace(file.find(".wav"),4,".mid").c_str());
			
}
