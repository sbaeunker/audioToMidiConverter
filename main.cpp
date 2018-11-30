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
	if(argc < 1){
		cerr << "At least one input Argument expected. Specify the File to Convert";
	}else if(argc > 5){
		cerr << "Too many Input Arguments";
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
							
	}
	//std::cout << std::to_string(windowSize) << std::endl;
	FrequencyApprox approx{};
	approx.toMIDI(argv[1], windowSize, windowDistance, zeroPadding);

			
}
