#pragma once
#include <vector>
#include <cmath>
#include <string>
#include "WAV.h"

#define BITS_IN_BYTE 8

enum Position {
	NONE = 0,
	START = 1,
	END = 2,
	BOTH = 3
};

enum class timeFormat {
	SECONDS,
	SAMPLES
};

struct AudioFunctions {

	//finds and returns the peak value of a vector of audio data, for use in amplification
	template <class number>
	static double getNormalizeRatio(std::vector <number> & audio) {
		number buffer = 0;
		for (auto n : audio) {
			//check to prevent wraparound
			//to-do: make more efficient
			if (((n != -(ceil(pow(2, BITS_IN_BYTE * sizeof(number))) / 2)) ? abs(n) : abs(++n)) > buffer) {
				buffer = (n != -(ceil(pow(2, BITS_IN_BYTE * sizeof(number))) / 2)) ? abs(n) : abs(++n);
			}
		}
		return (((ceil(pow(2, BITS_IN_BYTE * sizeof(number))) / 2) - 1) / buffer);
	}

	//multiplies the audio by a value, either quieting it (if <1) or making it louder (if >1)
	//returns false if the audio clips (ie becomes larger than its container)
	//use 0 as your amplification factor to normalize the audio
	template <class number>
	static bool amplify(std::vector <number>& audio, double amplificationFactor) {
		std::cout << "Amplifying file with factor " << amplificationFactor << "...\n";
		int64_t buffer;

		int64_t upper_limit = (ceil(pow(2, BITS_IN_BYTE * sizeof(number))) / 2);
		int64_t lower_limit = -(ceil(pow(2, BITS_IN_BYTE * sizeof(number))) / 2);

		bool clipped = false;
		int64_t counter = 0;
		for (auto n : audio) {
			//std::cout << n << ' ';
			buffer = (double)n * amplificationFactor;
			//std::cout << buffer << "\n";

			//clipping test and handler
			if (buffer >= upper_limit ||
				buffer < lower_limit){
				if (!clipped) {
					std::cout << "The audio data is clipping!\n";
					clipped = true;
				}
				if (buffer >= 0) {
					buffer = --upper_limit;
				}
				else {
					buffer = lower_limit;
				}
			}
			//std::cout << buffer << "\n";
			//getchar();
			audio.at(counter) = buffer;
			++counter;
		}
		std::cout << "Amplification complete.\n";
		return clipped;
	}


	//remove silence from track
	//position can be specific to be from beginning and/or from end of track
	//can also remove clicks from track in a nonspecific and clunky way
	//return value gives number of samples removed from front and back
	template <class number>
	static std::pair<uint32_t, uint32_t> removeSilence(std::vector<number>& audio, Position removePosition = BOTH, bool removeClick = false, Position clickPosition = NONE, int16_t threshold = 0) {
		std::cout << "Removing silence...\n";

		std::pair<uint32_t, uint32_t> samplesRemoved;
		samplesRemoved.first = 0;
		samplesRemoved.second = 0;

		if ((removePosition & START) == START) {
			std::cout << "Removing silence from start...";
			for (auto i = audio.begin(); i != audio.end(); ++i) {
				if (*i != threshold || *i != -threshold) {
					if (removeClick && ((clickPosition & START) == START)) {
						std::cout << "Click found!\n";
						removeClick = false;
						for (auto it = i; it != audio.end(); ++it) {
							if (*it == threshold || *it == -threshold) {
								i = it;
								break;
							}
						}
					}
					else {
						std::cout << "Silence found!\n";
						samplesRemoved.first = std::distance(audio.begin(), i);
						audio.erase(audio.begin(), i);
						break;
					}
				}
			}
			std::cout << "Silence removed from start.\n";
		}

		if (clickPosition != NONE) {
			removeClick = true;
		}

		if ((removePosition & END) == END) {
			std::cout << "Removing silence from end...";
			for (auto i = (audio.end() - 1); i != audio.begin(); --i) {
				if (*i != threshold || *i != -threshold) {
					if (removeClick && ((clickPosition & END) == END)) {
						std::cout << "Click found!\n";
						removeClick = false;
						for (auto it = i; it != audio.begin(); --it) {
							if (*it == threshold || *it == -threshold) {
								i = it;
								break;
							}
						}
					}
					else {
						std::cout << "Silence found!\n";
						samplesRemoved.second = std::distance(i, audio.end());
						audio.erase(i, audio.end());
						break;
					}
				}
			}
			std::cout << "Silence removed from end.\n";
		}
		std::cout << "Silence removal complete.\n";
		return samplesRemoved;
	}

	//similar to removeSilence(), but with a specified range of samples to remove, expressed as a pair
	template <class number>
	static void removeSamples(std::vector <number>& audio, std::pair<uint32_t, uint32_t> numberofSamples) {
		if (numberofSamples.first + numberofSamples.second < audio.size()) {
			audio.erase(audio.begin(), audio.begin() + numberofSamples.first);
			audio.erase((audio.end() - numberofSamples.second), audio.end());
		}
	}

	//add silence to beginning and/or end of track
	//can be used in either seconds or samples
	template <class number>
	static void addSilence(std::vector<number>& audio, timeFormat format, double silenceLength, Position addPosition, uint32_t sampleRate = 1) {
		if (format == timeFormat::SAMPLES) {
			sampleRate = 1;
		}
		if ((addPosition & START) == START) {
			std::cout << "Adding " << silenceLength << " " << ((format == timeFormat::SAMPLES) ? "samples" : "seconds") << " of silence to the start...\n";
			audio.insert(audio.begin(), (silenceLength * ((format == timeFormat::SAMPLES) ? sampleRate : 1)), 0);
		}
		if ((addPosition & END) == END) {
			std::cout << "Adding " << silenceLength << " " << ((format == timeFormat::SAMPLES) ? "samples" : "seconds") << " of silence to the end...\n";
			audio.insert(audio.end(), (silenceLength * ((format == timeFormat::SAMPLES) ? sampleRate : 1)), 0);
		}
		std::cout << "Silence adding complete!\n";
	}

	//fade the audio out in a specified timeframe
	//if used with samples, do not include the sample rate in the arguments
	//future plans: add fadein option
	template <class number>
	static void fade(std::vector<number>& audio, timeFormat format, double fadeLength, Position fadePosition = END, uint32_t sampleRate = 1) {
		if (format == timeFormat::SECONDS) {
			fadeLength *= sampleRate;
		}

		if (fadeLength > audio.size()) {
			fadeLength = audio.size();
		}

		double fadeFactor = 1.0;
		double fadeRate = fadeFactor / fadeLength;

		if ((fadePosition & START) == START) {
			--fadeLength;
			std::cout << "Fading in audio...";
			for (auto i = (audio.begin() + fadeLength); i != audio.begin(); --i) {
				*i *= fadeFactor;
				fadeFactor -= fadeRate;
			}
		}
		else {
			std::cout << "Fading out audio...";
			for (auto i = (audio.end() - fadeLength); i != audio.end(); ++i) {
				*i *= fadeFactor;
				fadeFactor -= fadeRate;
			}
		}

		std::cout << "Fade complete.\n";
	}
};


