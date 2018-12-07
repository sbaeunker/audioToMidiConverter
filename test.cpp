#include <vector>
#include <iostream>
#include <string>

#include "lib/midi.h"
#include "lib/midi-parser.h"
#include "lib/frequencyApprox.h"

const uint16_t PPQ = 10;

int main(int argc, char *argv[])
{
    // input validation
    if (argc < 2)
    {
        cerr << "At least one input argument is expected. Please provide a file path..." << endl;
        return -1;
    }
    if (argc > 2)
    {
        cerr << "Too many input arguments (" << argc << ")" << endl;
        return -1;
    }

    string inputFilepath = string(argv[1]);
    size_t slashIndex = inputFilepath.find_last_of("/\\");
    size_t dotIndex = inputFilepath.find_last_of(".");
    // get file name without parent directories and file extension
    string inputFilename = inputFilepath.substr(slashIndex + 1, dotIndex - slashIndex - 1);

    vector<int> windowSizesList = {500, 1000};
    vector<int> windowDistancesList = {200, 500};
    vector<int> zeroPaddingsList = {0, 500};
    vector<int> maxNotesList = {8, 20, 88};
    vector<uchar> minVolumesList = {0, 5, 10};
    vector<uchar> noteSwitchThresholdsList = {0, 5, 10};

    for (int windowSize : windowSizesList)
    {
        for (int windowDistance : windowDistancesList)
        {
            for (int zeroPadding : zeroPaddingsList)
            {
                for (int maxNotes : maxNotesList)
                {
                    for (uchar minVolume : minVolumesList)
                    {
                        for (uchar noteSwitchThreshold : noteSwitchThresholdsList)
                        {
                            FrequencyApprox approx{};
                            int sampleSize, sampleRate;
                            // initially read audio file
                            float *samples = approx.loadAudiofile(inputFilepath.c_str(), sampleSize, sampleRate);

                            // perform fast-fourier-transformation (FFT) and aggregate results to MIDI compatible format
                            int frames, midiTempo;
                            short **midiTable = approx.toMIDI(samples, sampleSize, sampleRate, windowSize, windowDistance, zeroPadding, midiTempo, frames);

                            // parse data to MIDI object
                            uint32_t tempo = midiTempo;
                            MIDIParser parser{tempo, PPQ, maxNotes, minVolume, noteSwitchThreshold};
                            MIDIFile midiFile = parser.getMidiFile(midiTable, frames);

                            // save as MIDI file
                            string filenameSuffix1 = "-" + to_string(windowSize) + "-" + to_string(windowDistance) + "-" + to_string(zeroPadding);
                            string filenameSuffix2 = "-" + to_string(maxNotes) + "-" + to_string(minVolume) + "-" + to_string(noteSwitchThreshold);
                            string outputFilename = "./output/" + inputFilename + filenameSuffix1 + filenameSuffix2 + ".mid";
                            midiFile.saveAs(outputFilename.c_str());

                            cout << "file created: " << outputFilename << endl;
                            free(midiTable);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
