#ifndef WL_LOADER
#define WL_LOADER

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>



#define SAMPLES_PER_BLOCK 28


const uint16_t rounding_table[SAMPLES_PER_BLOCK] = { 0,3,3,3,3,3,3, 2,2,2,2,2,2,2, 2,2,1,1,1,1,1, 1,1,1,1,1,1,0 };

class Audio_Handler {
private:
	int16_t* rawAudio;



	WL uninitialised;

public:

	ALuint load_WL(const char* filepathRel);

};

#endif // !WL_LOADER