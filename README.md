# Oscilloscopinator
Simple .program that prepares .wav files for oscilloscope videos in a semi-customizable manner 

## IMPORTANT NOTE
To convert multiple files at once, please follow this naming scheme:
file.wav (master audio) file1.wav file2.wav file3.wav...etc.
use the -n/--number option to load the additional files into the program.

## Command line options:
* -f, --file:
Location of master audio file
(Default argument: file.wav)

* -e, --export:
Location of exported audio file
(Default argument: export.wav)

* -n, --number:
Number of additional audio files to convert
(Default argument: 0)

* -k, --skip:
Uses binary logic to detemine which additional files to normalize and not amplify (Example: -k 5 = 0101 = amplify the first and third files; normalize the second and fourth) (Default argument: 0)

* -s, --samples:
Switches time format to samples instead of seconds

* -a, --amplify:
Amplification factor for audio
(Default argument: Normalize audio)

* -r, --remove:
Removes silence from start and/or end of audio (0 = none, 1 = start, 2 = end, 3 = both) (Default argument: 3)

* -c, --click:
Removes 'clicks' from start and/or end of audio-not perfect! (0 = none, 1 = start, 2 = end, 3 = both) (Default argument: 0)

* -t, --threshold:
Make the silence remover function less precise
(Default argument: 0)

* -d, --addstart:
Add amount of silence to start of audio
(Default argument: 0)

* -e, --addend:
Add amount of silence to end of audio
(Default argument: 0)

* -i, --fadein:
Add fade in to start of audio; argument determines length of fade
(Default argument: 0)

* -o, --fadeout:
Add fade out to end of audio; argument determines length of fade
(Default argument: 0)
