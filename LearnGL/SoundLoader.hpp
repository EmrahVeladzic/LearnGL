#ifndef WL_LOADER
#define WL_LOADER

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SystemConfig.h"




class Audio_Handler {
private:
	int16_t* rawAudio;



	WL uninitialised;

public:

	ALuint load_WL(const char* filepathRel);

};

#endif // !WL_LOADER