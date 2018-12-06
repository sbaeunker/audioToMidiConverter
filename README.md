# Audio to MIDI converter

This project is meant to transform audio files to MIDI files.

### Building

Call `make`.

(For Windows, make needs to be installed manually first!)

### Running

Pass path of audio file to convert as first parameter (required).

Currently supported formats are: WAV, AIFF

Example call: `./main.exe .\audioFiles\SpracheTest1.wav`

Other (optional) parameters are:

* windowSize (used for fast-fourier transformation)
* windowDistance (used for fast-fourier transformation)
* zeroPadding
* maxNotes (limit of number of notes that are played in parallel -> required for some hardware)
* minVolume (don't play too silent notes to generate less MIDI messages)
* noteSwitchThreshold (don't switch on too similar notes to generate less MIDI messages)

### Testing

The `test.cpp` contains a test script that basically tries out several parameter constellations.

As for the `main.exe` the path of the target audio file must be passed as first parameter.

Example call: `./test.exe .\audioFiles\SpracheTest1.wav`

### Links

* [MIDI](https://en.wikipedia.org/wiki/MIDI)
* [MIDI Format](http://www.ccarh.org/courses/253/handout/smf/)
* [Fast-Fourier Transformation](https://en.wikipedia.org/wiki/Fast_Fourier_transform)

Used libraries:
* [for loading audio file](https://github.com/adamstark/AudioFile)
* [for fast-fourier transformation](https://github.com/mborgerding/kissfft)
