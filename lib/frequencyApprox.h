#include "audiofile/AudioFile.h"
#include "kissfft/kiss_fftr.h"

class FrequencyApprox
{
	private:
		float * loadAudiofile(const char *filename , int &outSize, int &sampleRate);
		float ** movingFFT(float * samples,int sampleSize, int windowSize, int windowDistance, int zeroPadding, int &nrows, int &ncolumns);
		float abs(kiss_fft_cpx c);
		float * frequencyToKeys(float * spectrum, int size, int sampleRate);
		short ** velocityTable(float ** mfft, int nrows, int ncolumns, int sampleRate);
		short ** normalize(float ** keyStrokes, int nrows);
		
	public:	
		FrequencyApprox();
		short ** toMIDI(const char *filename, int windowSize, int windowDistance, int zeroPadding, bool writeCSV , int &midiTempo, int &frames);
		
};
