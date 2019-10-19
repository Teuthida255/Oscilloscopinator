#pragma once

//representation of the WAV file format, with functions for reading and writing into it
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#define BYTE_BASE 0x100 

//enumerated endianness value, for more readable variables
enum class ENDIAN {
	LITTLE = 0,
	BIG = 1,
};

template<class number>
bool loadFromBytes(number& loadedNumber, std::vector <uint8_t>& data, uint32_t & offset, ENDIAN endianness, bool verbose, bool testResult, number testValue, std::string failMessage);

template<class number>
void loadToBytes(number loadedNumber, std::vector <uint8_t>& data, ENDIAN endianness);

struct WAV {
	std::string filePath;

	uint32_t file_identifier;
	uint32_t file_size;
	uint32_t format;

	uint32_t format_identifier;
	uint32_t subchunk_size;
	uint16_t audio_format;
	uint16_t numberof_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_persample;

	uint32_t data_identifier;
	uint32_t data_size;
	std::vector <int16_t> audio_data; //only 16-bit WAVs are supported for now

	bool openflag;

	WAV(std::string & path);
	bool loadData(std::vector <uint8_t> data);

	bool exportData(std::ofstream& file);

	void resizeData();

	void clear();
};