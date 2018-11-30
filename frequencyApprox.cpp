#include <iostream>
#include <string>
#include <math.h> 
#include "frequencyApprox.h"
#define MIDI_MAX 127.0 //maximum amplitude in MIDI
#define NO_KEYS 88

FrequencyApprox::FrequencyApprox(){
	
}
void FrequencyApprox::toMIDI(const char *filename, int windowSize, int windowDistance, int zeroPadding)
{	
	float * samples; //sample Array to be filled
	int size; 
	int sampleRate; 
	samples = loadAudiofile(filename, size, sampleRate); //fill Samples
	//std::cout << "No. of samples read: "<<std::to_string(size) << std::endl;
	int nrows, ncolumns;
	float ** mfft = movingFFT(samples, size, windowSize ,windowDistance , zeroPadding, nrows, ncolumns); //calculate fft for windows
	free(samples);
	short ** velocities = velocityTable(mfft, nrows, ncolumns, sampleRate);
	free(mfft);
	
	for(int i=0; i < nrows; i++){		
		for(int j=0; j< NO_KEYS; j++){
			std::cout << std::to_string(velocities[i][j]) << ",";
		}
		std::cout << std::endl;
	}	
	free(velocities);
}

short ** FrequencyApprox::velocityTable(float ** mfft, int nrows, int ncolumns, int sampleRate){
	float ** kv = (float **) malloc(nrows * sizeof(float *));
    for(int i = 0; i < nrows; i++)
    {
        kv[i]= frequencyToKeys(mfft[i], ncolumns, sampleRate);
    }
    short ** norm = normalize(kv, nrows);
    return norm;
}

short ** FrequencyApprox::normalize(float ** keyStrokes, int nrows){
	float max = 0.0f;
	
	short ** norm = (short **) malloc(nrows * sizeof(short *));
    for(int i = 0; i < nrows; i++)
    {
        norm[i] = (short *) malloc(NO_KEYS * sizeof(short));
    }
	for(int i=0; i < nrows; i++){//find maximum and applay weighting, here: sqrt
		for(int j=0; j< NO_KEYS; j++){
			if(keyStrokes[i][j]> max){
				keyStrokes[i][j] = sqrt(keyStrokes[i][j]); //weighted with sqrt;
				max = keyStrokes[i][j];				
			}
		}
	}
	for(int i=0; i < nrows; i++){//normalise and round
		for(int j=0; j< NO_KEYS; j++){
			norm[i][j] = (short) (MIDI_MAX*(keyStrokes[i][j]/max)); 
		}
	}
	free(keyStrokes);
	return norm;
	
}

float * FrequencyApprox::frequencyToKeys(float * spectrum, int size, int sampleRate){
	float fres= sampleRate / size;//frequency resolution
	
	float * velocity = (float *) malloc(sizeof(float)*NO_KEYS); //force of each key stroke
	float * fgu = (float*) malloc(sizeof(float)*NO_KEYS);
	float * fgo = (float*) malloc(sizeof(float)*NO_KEYS);
	//define frequencyBins for each key through upper and lower limits
	for(int i=0; i < NO_KEYS; i++){
		fgu[i]= 440.0 * pow(2.0,((i-49.5)/12));// obere grenzfrequenzen
		fgo[i]= 440.0 * pow(2.0,((i-48.5)/12));// untere grenzfrequenzen
	}
	for(int i = 0; i <  NO_KEYS; i++){
		velocity[i] = 0.0f; //init with 0 if not all keys are played
	}
	for(int i = 0; i< (int) size; i++){ //only using to nyquist f 	
		int j=0;
		for(j;j < NO_KEYS; j++){			
			if(i*fres > fgu[j] && i*fres <= fgo[j]){//frequency is in bin of key j
				velocity[j] =velocity[j]+ spectrum[i]; //sum up all frequencies in bin
				break;
			}
		}
	}	
	
	free(fgu);
	free(fgo);
	return velocity;
}

float ** FrequencyApprox::movingFFT(float * samples,int sampleSize, int windowSize, int windowDistance, int zeroPadding, int &nrows, int &ncolumns)
{
	float ** mfft;
	//allocating space for 2D array
	nrows = (int) (sampleSize-windowSize)/windowDistance;
	ncolumns = windowSize + zeroPadding; 
	mfft = (float **) malloc(nrows * sizeof(float *));
    for(int i = 0; i < nrows; i++)
    {
        mfft[i] = (float *) malloc(ncolumns * sizeof(float));
    }
    // init kiss fft and allocate temp array for window ffts https://github.com/mborgerding/kissfft
    kiss_fftr_cfg cfg = kiss_fftr_alloc(ncolumns, false, 0,0);
    kiss_fft_cpx * windowfft = (kiss_fft_cpx *) malloc(ncolumns * sizeof(kiss_fft_cpx *));
    float * rowSamples = (float *) malloc(ncolumns * sizeof(float));
    //every Row contains a fft of a segment 
    for(int i=0; i < nrows; i++){
		//calculate fft of respective segment
		kiss_fftr(cfg, samples+(i*windowDistance) , windowfft);//TODO: implement zeroPadding
		for(int j=0; j< ncolumns; j++){//fill in fft data
			mfft[i][j] = abs(windowfft[j]);
		}
	}
	free(rowSamples);
    free(windowfft);
	return mfft;
}

float FrequencyApprox::abs(kiss_fft_cpx c){ // returns abs from complex number
	return sqrt(c.r * c.r + c.i * c.i);
}

float * FrequencyApprox::loadAudiofile(const char *filename, int &outSize, int &sampleRate)
{
	//using https://github.com/adamstark/AudioFile to load audio Samples
	AudioFile<float> audioFile;
	audioFile.load(filename);
	sampleRate = audioFile.getSampleRate();
	//audioFile.printSummary();
	
	//writing Samples to array
	int channel = 0;
	outSize = audioFile.getNumSamplesPerChannel();
	float * samples = (float *) malloc(sizeof(float)*outSize);

	for (int i = 0; i < outSize; i++)
	{
		samples[i] = audioFile.samples[channel][i];		
	}
	
	return samples;
}



