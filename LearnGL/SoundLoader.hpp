#ifndef WL_LOADER
#define WL_LOADER

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>




#define SAMPLES_PER_BLOCK 28

class Audio_Handler {
private:
	int16_t* rawAudio;

	int16_t WL_frame_count;

	int16_t Sample_rate;

	WL uninitialised;

public:

	ALuint load_WL(const char* filepathRel);

};

#endif // !WL_LOADER