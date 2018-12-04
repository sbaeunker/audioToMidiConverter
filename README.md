# Audio to MIDI converter

This project is meant to transform audio files (currently only .wav's are supported) to MIDI files.

### Building

Call `make`.

(For Windows, make needs to be installed manually first!)

### Running

Pass audio file to convert as first parameter. E. g. `./main.exe .\audioFiles\SpracheTest1.wav`

Other (optional) parameters are:

* windowSize (used for fast-fourier transformation)
* windowDistance (used for fast-fourier transformation)
* zeroPadding (not used (yet)...)
* maxNotes (limit of number of notes that are played in parallel -> required for some hardware)

### Links

* [MIDI](https://en.wikipedia.org/wiki/MIDI)
* [MIDI Format](http://www.ccarh.org/courses/253/handout/smf/)
* [Fast-Fourier Transformation](https://en.wikipedia.org/wiki/Fast_Fourier_transform)

Used libraries:
* [for loading audio file](https://github.com/adamstark/AudioFile)
* [for fast-fourier transformation](https://github.com/mborgerding/kissfft)
