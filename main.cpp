#include <vector>
#include <iostream>

#include "midi.h"
#include "midi-parser.h"

#include "frequencyApprox.h"




using namespace std;
typedef unsigned char uchar;


int main(int argc, char* argv[])
{
	if(argc < 1) 
	{
		cerr << "At least one input Argument expected. Specify the File to Convert";
	}else 
	{		
		FrequencyApprox approx{};
		approx.toMIDI(argv[1], 1000, 1000, 0);

	}		
}
