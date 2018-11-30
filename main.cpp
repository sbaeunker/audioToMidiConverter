#include <vector>
#include <stdio.h>
//#include <windows.h>
//#include <tchar.h>

#include <iostream>

#include "midi.h"
#include "midi-parser.h"


using namespace std;
typedef unsigned char uchar;

const uint32_t TEMPO = 45351; // 100 * 1000; // microseconds for one quarter note / beat (default = 500 ms = 500 * 1000 us)
const uint16_t PPQ = 960;      // parts per quarter note -> with default settings (quarter note = 500 ms) PPQ *2(!) = parts per second

MIDIFile demo1()
{
    MIDIFile demo{PPQ};

    MIDITrack track1{TEMPO}, track2{TEMPO};

    track2.programChange(2, 52); // track 2 = choir aah

    vector<uchar> notes = {
        60, 62, 64, 65, 67, 67, // c d e f g g
        69, 69, 69, 69, 67,     // a a a a g
        69, 69, 69, 69, 67,     // a a a a g
        65, 65, 65, 65, 64, 64, // f f f f e e
        62, 62, 62, 62, 60};    // d d d d c
    vector<unsigned> pauses = {
        PPQ, PPQ, PPQ, PPQ, 2 * PPQ, 2 * PPQ,
        PPQ, PPQ, PPQ, PPQ, 4 * PPQ,
        PPQ, PPQ, PPQ, PPQ, 4 * PPQ,
        PPQ, PPQ, PPQ, PPQ, 2 * PPQ, 2 * PPQ,
        PPQ, PPQ, PPQ, PPQ, 4 * PPQ};

    for (unsigned i = 0; i < notes.size() && i < pauses.size(); ++i)
    {
        track1.noteOn(0, notes[i], 0x7F);
        track1.addDelay(pauses[i]);
        track1.noteOff(0, notes[i]);

        track2.noteOn(1, notes[i], i < 10 ? 0 : 0x7F);
        track2.addDelay(pauses[i]);
        track2.noteOff(1, notes[i]);
    }

    demo.addTrack(track1);
    demo.addTrack(track2);

    demo.generate();

    return demo;
}
/*
void rwFile(WIN32_FIND_DATA FindFileData, string inputFolder,string outputFolder) {
	string fN(FindFileData.cFileName);
	MIDIParser temp{ TEMPO, PPQ };
	MIDIFile file3 = temp.getMidiFile((inputFolder + fN).c_str(), true, 0, 0, true);
	fN = fN.substr(0, fN.length() - 3);
	fN.append("mid");
	file3.saveAs((outputFolder + fN).c_str());
	//std::cout << fN.c_str() << endl;
}
*/

/**
 * g++ -Wall main.cpp lib/midi.cpp lib/midi-parser.cpp -o main
 * ./main 
 */
int main()
{
	
	/*
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		

		//_tprintf(TEXT("Target file is %s\n"), "E:\\Downloads\\Quellcode\\Quellcode");
		std::string inputFolder ("E:\\Downloads\\Quellcode\\Quellcode\\output\\ClavinovaTest\\");
		std::string outputFolder("E:\\Downloads\\Quellcode\\Quellcode\\output\\ClavinovaTest\\");
		hFind = FindFirstFile( (inputFolder+ "*.csv").c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("FindFirstFile failed (%d)\n", GetLastError());
			return 0;
		}
		else {
			rwFile(FindFileData, inputFolder, outputFolder);
		}
		
		while (FindNextFile(hFind, &FindFileData) != 0) {
			rwFile(FindFileData, inputFolder, outputFolder);
		}
		FindClose(hFind);
		int i;
		cin >> i;
		return 0;
   */
	/*
    MIDIParser temp{TEMPO, PPQ};
    MIDIFile file3 = temp.getMidiFile("C:\\Users\\Stefan\\Documents\\MATLAB\\Treiberentwicklung\\SprachetTest1CSV\\fs44100_ampRoot2_w4000_d4000_gewNone_fbSum_dfLog_phaseNone.csv", 4); // optional 4th parameter: program (e. g. 52 for choir aah)
    file3.saveAs("C:\\Users\\Stefan\\Desktop\\test.mid");
	*/
	return 0;
		
}
