#include "audiofile/AudioFile.h"
#include "kissfft/kiss_fftr.h"

class FrequencyApprox
{
  private:
	float **movingFFT(float *samples, int sampleSize, int windowSize, int windowDistance, int zeroPadding, int &nrows, int &ncolumns);
	float abs(kiss_fft_cpx c);
	float *frequencyToKeys(float *spectrum, int size, int sampleRate);
	short **velocityTable(float **mfft, int nrows, int ncolumns, int sampleRate);
	short **normalize(float **keyStrokes, int nrows);

  public:
	FrequencyApprox();
	float *loadAudiofile(const char *filename, int &outSize, int &sampleRate);

	short **toMIDI(const char *filename, int windowSize, int windowDistance, int zeroPadding, int &midiTempo, int &frames);
	short **toMIDI(float *samples, int size, int sampleRate, int windowSize, int windowDistance, int zeroPadding, int &midiTempo, int &frames);
};
