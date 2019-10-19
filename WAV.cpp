#include "WAV.h"
#include <iostream>
#define NUMBER_OF_BITS 8
#define INT16_SIGNED_CUTOFF 0x7FFF
#define INT16_SIZE 65536 

//load a series of bytes as either a big or little-endian number
//also includes an optional check to test where the result fits a given value
template <class number>
bool loadFromBytes(number& loadedNumber, std::vector <uint8_t>& data, uint32_t & offset, ENDIAN endianness, bool verbose = false, bool testResult = false, number testValue = 0, std::string failMessage = "") {
	//check bounds
	if (offset + sizeof(loadedNumber) <= data.size()) {
		uint32_t accumulator = 0;

		if (endianness == ENDIAN::BIG) {
			for (int n = 0; n < sizeof(loadedNumber); n++) {
				if (verbose) {
					std::cout << "Position: " << (n + offset) << ", Data: " << std::hex << (unsigned int) data[n + offset] << "\n";
				}
				accumulator = accumulator << NUMBER_OF_BITS;
				if (verbose) {
					std::cout << "Accumulator value: " << std::hex << accumulator << "->";
				}
				accumulator += data[n + offset];
				if (verbose) {
					std::cout << std::hex << accumulator << "\n";
				}
			}
		}
		else {
			for (int n = (sizeof(loadedNumber) - 1); n >= 0; n--) {
				if (verbose) {
					std::cout << "Position: " << (n + offset) << ", Data: " << std::hex << ((unsigned int) data[n + offset]) << "\n";
				}
				accumulator = accumulator << NUMBER_OF_BITS;
				if (verbose) {
					std::cout << "Accumulator value: " << std::hex << accumulator << "->";
				}
				accumulator += data[n + offset];
				if (verbose) {
					std::cout << std::hex << accumulator << "\n";
				}
			}
		}
		if (verbose) {
			getchar();
		}
		loadedNumber = accumulator;
		offset += sizeof(loadedNumber);

		if (testResult) {
			if (loadedNumber != testValue) {
				std::cout << failMessage << "\n";
				return false;
			}
		}
		return true;
	}
	return false;
}

//convert number to series of bytes and attach it to a vector
template <class number>
void loadToBytes(number loadedNumber, std::vector <uint8_t>& data, ENDIAN endianness) {
	//std::cout << "Loading in " << std::hex << loadedNumber << "...\n";
	number buffer;
	const number bytemask = 0xff;
	//fix this, maybe using AND?
	if (endianness == ENDIAN::BIG) {
		for (int n = (sizeof(loadedNumber) - 1); n >= 0; n--) {
			buffer = loadedNumber;
			buffer = buffer & (bytemask << (NUMBER_OF_BITS * n));
			//std::cout << std::hex << buffer << "\n";
			buffer = buffer >> (NUMBER_OF_BITS * n);
			//std::cout << std::hex << buffer << "\n";
			data.push_back(buffer);
			//getchar();
		}
	}
	else {
		for (int n = 0; n < sizeof(loadedNumber); n++) {
			buffer = loadedNumber;
			buffer = buffer & (bytemask << (NUMBER_OF_BITS * n));
			//std::cout << std::hex << buffer << "\n";
			buffer = buffer >> (NUMBER_OF_BITS * n);
			//std::cout << std::hex << buffer << "\n";
			data.push_back(buffer);
			//getchar();
		}
	}
}


//initialize and load in wav data from a path
WAV::WAV(std::string & path) {
	filePath = path;

	std::ifstream wavFile;
	wavFile.open(path, std::ios::binary | std::ios::ate);
	if (!wavFile.is_open()) {
		std::cout << "Failed to open file from path \"" << path << "\"!\n";
		openflag = false;
	}
	else {
		openflag = true;
		std::cout << "File opened successfully from path \"" << path << "\"!\n";

		//read data from file and get its size
		std::streampos fileSize = wavFile.tellg();
		char* dataArray = new char[fileSize];

		wavFile.seekg(0, std::ios::beg);
		wavFile.read(dataArray, fileSize);

		//load data uinto a safer storage container
		std::vector <uint8_t> dataVector;
		for (uint32_t i = 0; i < fileSize; i++) {
			dataVector.push_back((uint8_t)dataArray[i]);
		}
		//char data is no longer necessary and can be deleted; the file can be cloesed as well
		delete[] dataArray;
		wavFile.close();

		if (loadData(dataVector)) {
			std::cout << "Audio loading complete!\n\n";
		}
		else {
			std::cout << "File loading failed!\n\n";
		}
	}
}

//load data from vector uinto struct
//includes checks to stop range problems
bool WAV::loadData(std::vector <uint8_t> data) {
	clear();

	const int HEADER_SIZE = 32;
	//all WAV files have a header that is at least 32 bytes long
	if (data.size() < HEADER_SIZE) {
		std::cout << "File is not large anough to be a WAV!\n";
		return false;
	}
	else {
		std::cout << "Loading WAV header data...\n";
		//begin loading
		uint32_t pos = 0; //memory position

		if (!loadFromBytes(file_identifier, data, pos, ENDIAN::BIG, false, true, (uint32_t)0x52494646, "File does not have a WAV identifier!")) {
			return false;
		}
		loadFromBytes(file_size, data, pos, ENDIAN::LITTLE);
		if (!loadFromBytes(format, data, pos, ENDIAN::BIG, false, true, (uint32_t)0x57415645, "File does not have a format marker!")) {
			return false;
		}
		if (!loadFromBytes(format_identifier, data, pos, ENDIAN::BIG, false, true, (uint32_t)0x666d7420, "File does not have a format identifier!")) {
			return false;
		}
		if (!loadFromBytes(subchunk_size, data, pos, ENDIAN::LITTLE, false, true, (uint32_t)0x10, "File is not PCM! This program currently only supports PCM audio.\n")) {
			return false;
		}
		if (!loadFromBytes(audio_format, data, pos, ENDIAN::LITTLE, false, true, (uint16_t)0x01, "File is compressed! This program currently only supports uncompressed WAV audio.\n")) {
			return false;
		}
		if (!loadFromBytes(numberof_channels, data, pos, ENDIAN::LITTLE, false, true, (uint16_t)0x01, "File uses non-mono audio! This program currently only supports mono audio.\n")) {
			return false;
		}
		loadFromBytes(sample_rate, data, pos, ENDIAN::LITTLE);
		loadFromBytes(byte_rate, data, pos, ENDIAN::LITTLE);
		loadFromBytes(block_align, data, pos, ENDIAN::LITTLE);
		if (!loadFromBytes(bits_persample, data, pos, ENDIAN::LITTLE, false, true, (uint16_t)0x10, "File does not use 16 bits per sample! This program current only supports 16-bit audio.\n")) {
			return false;
		}
		if (!loadFromBytes(data_identifier, data, pos, ENDIAN::BIG, false, true, (uint32_t)0x64617461, "File does not have a data identifier!\n")) {
			return false;
		}
		loadFromBytes(data_size, data, pos, ENDIAN::LITTLE);

		//load in audio data
		std::cout << "Loading audio data...\n";
		uint32_t audioStart = pos;
		int counter = 0;
		std::vector<uint16_t> unsigned_audio_data;
		//std::cout << std::hex << (file_size + sizeof(file_identifier) + sizeof(file_size)) << "\n";
		while (pos <= (file_size + sizeof(file_identifier) + sizeof(file_size))) {
			
			//std::cout << "Sample #" << (pos - audioStart + 1) << " at " << std::hex << pos << " ";
			unsigned_audio_data.push_back(0);
			//std::cout << " loading...";
			uint16_t buffer = unsigned_audio_data[counter];
			if (!loadFromBytes(buffer, data, pos, ENDIAN::LITTLE)) { break; }
			//std::cout << std::hex << buffer << "... " << std::hex << pos << "\n";
			unsigned_audio_data[counter] = buffer;
			counter++;
		}
		//cheesy fix to minor bug
		//if you can solve it better, please do so
		unsigned_audio_data.pop_back();

		//convert binary representation of signed data into actual signed data
		int32_t conversionBuffer;
		for (auto n : unsigned_audio_data) {
			conversionBuffer = n;
			//positive numbers
			if (conversionBuffer <= INT16_SIGNED_CUTOFF) {
				//std::cout << "Value " << conversionBuffer << " is positive. It is now ";
				//std::cout << conversionBuffer << " .\n";
			}
			//negative numbers
			else {
				//std::cout << "Value " << conversionBuffer << " is negative. It is now ";
				conversionBuffer -= INT16_SIZE;
				//std::cout << conversionBuffer << " .\n";
			}
			audio_data.push_back(conversionBuffer);
		}

		std::cout << "Audio data loading complete!\n";
		return true;
	}
}

//exports all data to a char array
bool WAV::exportData(std::ofstream& file) {
	std::cout << "Converting to file...\n";
	resizeData();

	std::vector <uint8_t> byte_data;
	loadToBytes(file_identifier, byte_data, ENDIAN::BIG);
	loadToBytes(file_size, byte_data, ENDIAN::LITTLE);
	loadToBytes(format, byte_data, ENDIAN::BIG);

	loadToBytes(format_identifier, byte_data, ENDIAN::BIG);
	loadToBytes(subchunk_size, byte_data, ENDIAN::LITTLE);
	loadToBytes(audio_format, byte_data, ENDIAN::LITTLE);
	loadToBytes(numberof_channels, byte_data, ENDIAN::LITTLE);
	loadToBytes(sample_rate, byte_data, ENDIAN::LITTLE);
	loadToBytes(byte_rate, byte_data, ENDIAN::LITTLE);
	loadToBytes(block_align, byte_data, ENDIAN::LITTLE);
	loadToBytes(bits_persample, byte_data, ENDIAN::LITTLE);
	loadToBytes(data_identifier, byte_data, ENDIAN::BIG);
	loadToBytes(data_size, byte_data, ENDIAN::LITTLE);

	//audio handling
	//convert to hex signed representation first
	std::vector<uint16_t> unsigned_audio_data;
	int32_t conversionBuffer;
	for (auto n : audio_data) {
		conversionBuffer = n;
		//negative numbers; do nothing for positives
		if (conversionBuffer < 0) {
			conversionBuffer += INT16_SIZE;
		}
		unsigned_audio_data.push_back(conversionBuffer);
	}
	//then move to byte data
	for (auto n : unsigned_audio_data) {
		loadToBytes(n, byte_data, ENDIAN::LITTLE);
	}

	//lastly, move all the byte data into the file array...
	uint32_t byteSize = byte_data.size();
	//std::cout << byteSize << "\n";
	char* dataArray = new char[byteSize];
	for (uint32_t i = 0; i < byteSize; i++) {
		*(dataArray + (i * sizeof(char))) = byte_data[i];
		//std::cout << byte_data[i] << " " << *(dataArray + (i * sizeof(char))) << "\n";
	}

	std::cout << "Writing file...\n";
	file.write(dataArray, byteSize);
	file.flush();

	std::cout << "File saving complete!";
	
	//may add some error checking here
	return true;
}

//alter the WAV file size and data size parameters (for use when adding or removing parts of the audio)
void WAV::resizeData(){
	//this isn't going to scale
	data_size = audio_data.size() * sizeof(int16_t);
	file_size = data_size + 36;
}

//empties the file so new data can be added
void WAV::clear() {
	filePath.clear();

	file_identifier = 0;
	file_size = 0;
	format = 0;

	format_identifier = 0;
	subchunk_size = 0;
	audio_format = 0;
	numberof_channels = 0;
	sample_rate = 0;
	byte_rate = 0;
	block_align;
	bits_persample = 0;

	data_identifier = 0;
	data_size = 0;
	audio_data.clear();
}