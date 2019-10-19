#include "AudioFunctions.h"
#include <cstdint>
#define SIZEOF_BYTE 16

//multiplies the audio by a value, either quieting it (if <1) or making it louder (if >1)
//returns false if the audio clips (ie becomes larger than its container)

template <class number>
bool AudioFunctions::amplify(std::vector<number>& audio, bool numberissigned, float amplificationFactor)