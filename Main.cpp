#include "WAV.h"
#include "AudioFunctions.h"
#include "CommandLineArguments.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

#define FILE 0
#define EX 1
#define NUMBER 2
#define SKIP 3
#define SAMPLE 4
#define AMPLIFY 5
#define REMOVE 6
#define CLICK 7
#define THRESHOLD 8
#define ADDSTART 9
#define ADDEND 10
#define FADEIN 11
#define FADEOUT 12



int main(int argc, char* argv[]) {

	Option file('f', "file", true,
		"Location of master audio file",
		"",
		"file.wav");
	Option ex('e', "export", true,
		"Location of exported audio file",
		"",
		"yeet.wav");
	Option number('n', "number", true,
		"Number of additional audio files to convert",
		"",
		"0");
	Option skip('k', "skip", true,
		"Uses binary logic to detemine which additional files to normalize and not amplify",
		"Example: -k 5 = 0101 = amplify the first and third files; normalize the second and fourth",
		"0");
	Option samples('s', "samples", false,
		"Switches time format to samples instead of seconds");
	Option amplify('a', "amplify", true,
		"Amplification factor for audio",
		"",
		"Normalize audio");
	Option remove('r', "remove", true,
		"Removes silence from start and/or end of audio",
		"0 = none, 1 = start, 2 = end, 3 = both",
		"3");
	Option click('c', "click", true,
		"Removes 'clicks' from start and/or end of audio-not perfect!",
		"0 = none, 1 = start, 2 = end, 3 = both",
		"0");
	Option threshold('t', "threshold", true,
		"Make the silence remover function less precise",
		"",
		"0");
	Option addstart('d', "addstart", true,
		"Add amount of silence to start of audio",
		"",
		"0");
	Option addend('e', "addend", true,
		"Add amount of silence to end of audio",
		"",
		"0");
	Option fadein('i', "fadein", true,
		"Add fade in to start of audio",
		"Argument: length of fade",
		"0");
	Option fadeout('o', "fadeout", true,
		"Add fade out to end of audio",
		"Argument: length of fade",
		"0");


	std::vector <Option*> opttable;
	opttable.push_back(&file);
	opttable.push_back(&ex);
	opttable.push_back(&number);
	opttable.push_back(&skip);
	opttable.push_back(&samples);
	opttable.push_back(&amplify);
	opttable.push_back(&remove);
	opttable.push_back(&click);
	opttable.push_back(&threshold);
	opttable.push_back(&addstart);
	opttable.push_back(&addend);
	opttable.push_back(&fadein);
	opttable.push_back(&fadeout);

	std::string buffer;
	switch (argc) {
		//if just the program's name was called
	case 1:
		std::cout << "Oscilloscopinator: a program for preparing .wav files for oscillocope videos\n"
			<< "Created by Teuthida\n\n";
		std::cout << "IMPORTANT NOTE: To convert multiple files at once, please follow this naming scheme:\n"
			<< "file.wav (master audio) file1.wav file2.wav file3.wav...etc.\n"
			<< "use the -n/--number option to load the additional files into the program.\n\n";
		std::cout << "Command line options:\n";
		for (auto i : opttable) {
			i->describeOption();
		}
		std::cout << "\n";
		return 0;
		break;
		//if just two things were called, use the second as the filename
	case 2:
		opttable[FILE]->setArgument(argv[1]);
		buffer.assign(argv[1]);
		//remove extension
		buffer.erase(buffer.end() - 4, buffer.end());
		opttable[EX]->setArgument(buffer + "_export.wav");
		break;
	default:
		handleOptions(argc, argv, opttable);
	}

	//do master audio first; this will determine how the others are handled
	std::string filepath = opttable[FILE]->getArgument();
	timeFormat t = (opttable[SAMPLE]->getOptFlag()) ? timeFormat::SAMPLES : timeFormat::SECONDS;
	WAV audio(filepath);
	double ampFactor;
	if (opttable[AMPLIFY]->getArgument() == "Normalize audio") {
		std::cout << "Normalize\n";
		ampFactor = AudioFunctions::getNormalizeRatio(audio.audio_data);
	}
	else {
		std::cout << "Don't normalize\n";
		ampFactor = std::stod(opttable[AMPLIFY]->getArgument());
	}
	Position removepos = (Position)std::stoi(opttable[REMOVE]->getArgument());
	if (removepos > Position::BOTH) {
		removepos = Position::BOTH;
	}
	bool removeclick = opttable[CLICK]->getOptFlag();
	Position clickpos = (Position)std::stoi(opttable[CLICK]->getArgument());
	if (clickpos > Position::BOTH) {
		clickpos = Position::BOTH;
	}
	int16_t thres = std::stoi(opttable[THRESHOLD]->getArgument());
	double addlengthstart = (opttable[ADDSTART]->getOptFlag() && opttable[ADDSTART]->argIsDefined()) ? std::stod(opttable[ADDSTART]->getArgument()) : 0.0;
	double addlengthend = (opttable[ADDEND]->getOptFlag() && opttable[ADDEND]->argIsDefined()) ? std::stod(opttable[ADDEND]->getArgument()) : 0.0;
	double fadelengthstart = (opttable[FADEIN]->getOptFlag() && opttable[FADEIN]->argIsDefined()) ? std::stod(opttable[FADEIN]->getArgument()) : 0.0;
	double fadelengthend = (opttable[FADEOUT]->getOptFlag() && opttable[FADEOUT]->argIsDefined()) ? std::stod(opttable[FADEOUT]->getArgument()) : 0.0;

	AudioFunctions::amplify(audio.audio_data, ampFactor);

	std::pair<uint32_t, uint32_t> silenceLength;
	silenceLength = AudioFunctions::removeSilence(audio.audio_data, removepos, removeclick, clickpos, thres);
	if (addlengthstart != 0) {
		AudioFunctions::addSilence(audio.audio_data, t, addlengthstart, Position::START, audio.sample_rate);
	}
	if (addlengthend != 0) {
		AudioFunctions::addSilence(audio.audio_data, t, addlengthend, Position::END, audio.sample_rate);

	}
	if (fadelengthstart != 0) {
		AudioFunctions::fade(audio.audio_data, t, fadelengthstart, Position::START, audio.sample_rate);

	}
	if (fadelengthend != 0) {
		AudioFunctions::fade(audio.audio_data, t, fadelengthend, Position::END, audio.sample_rate);
	}

	//handle additional audio files
	std::vector<WAV*> audiofiles;
	const std::string extension = ".wav";
	std::string filepath_noext = filepath;
	filepath_noext.erase(filepath_noext.end() - 4, filepath_noext.end());

	//populate the normalization table with values
	int numberoffiles = stoi(opttable[NUMBER]->getArgument());
	std::vector<bool> normalizetable;
	const long bitmask = 0x01 << (numberoffiles - 1);
	long normalizevalue = std::stol(opttable[SKIP]->getArgument());
	if (normalizevalue >= 0) {
		if (normalizevalue >= ceil(pow(2, numberoffiles))) {
			normalizevalue = (ceil(pow(2, numberoffiles)) - 1);
		}
		for (int i = 0; i < numberoffiles; i++) {
			normalizetable.push_back(normalizevalue & (bitmask >> i));
		}
	}

	std::string itfilename;
	if (numberoffiles > 0) {
		for (int i = 1; i <= numberoffiles; i++) {
			itfilename = (filepath_noext + std::to_string(i) + extension);
			audiofiles.push_back(new WAV(itfilename));
		}

		//find the amplification factor for all the files
		ampFactor = AudioFunctions::getNormalizeRatio(audiofiles[0]->audio_data);
		double ampbuf;
		for (int i = 1; i < audiofiles.size(); i++) {
			if (!normalizetable[i]) { //normalized files use their own amplification factor
				ampbuf = AudioFunctions::getNormalizeRatio(audiofiles[i]->audio_data);
				ampFactor = (ampbuf < ampFactor) ? ampbuf : ampFactor;
			}
		}

		for (int i = 0; i < audiofiles.size(); i++) {
			AudioFunctions::amplify(audiofiles[i]->audio_data, (normalizetable[i] ? AudioFunctions::getNormalizeRatio(audiofiles[i]->audio_data) : ampFactor));

			AudioFunctions::removeSamples(audiofiles[i]->audio_data, silenceLength);
			if (addlengthstart != 0) {
				AudioFunctions::addSilence(audiofiles[i]->audio_data, t, addlengthstart, Position::START, audiofiles[i]->sample_rate);
			}
			if (addlengthend != 0) {
				AudioFunctions::addSilence(audiofiles[i]->audio_data, t, addlengthend, Position::END, audiofiles[i]->sample_rate);

			}
			if (fadelengthstart != 0) {
				AudioFunctions::fade(audiofiles[i]->audio_data, t, fadelengthstart, Position::START, audiofiles[i]->sample_rate);

			}
			if (fadelengthend != 0) {
				AudioFunctions::fade(audiofiles[i]->audio_data, t, fadelengthend, Position::END, audiofiles[i]->sample_rate);
			}
		}
	}

	std::string exportpath = opttable[EX]->getArgument();
	std::cout << "Writing to file " << exportpath << "...\n";
	std::ofstream export_file(exportpath, std::ofstream::binary);
	audio.exportData(export_file);

	exportpath.erase(exportpath.end() - 4, exportpath.end());
	int n = 0;
	for (auto i : audiofiles) {
		n++;
		itfilename = (exportpath + std::to_string(n) + extension);
		std::cout << "Writing to file " << itfilename << "...\n";
		std::ofstream export_file(itfilename, std::ofstream::binary);
		i->exportData(export_file);
	}


	return 0;
}
