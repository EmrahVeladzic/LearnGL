#ifndef WL_LOADER
#define WL_LOADER

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SystemConfig.h"

typedef enum {

	NONE = 0,
	SINGLE = 1,
	DOUBLE_1 =2,
	DOUBLE_2 =3,
	DOUBLE_3 =4

}SPU_FILTER;


class Audio_Handler {
private:
	int16_t* rawAudio;



	WL uninitialised;

public:

	ALuint * load_WL(const char* filepathRel);
	inline int32_t filter(int32_t current, int32_t old, int32_t older, uint8_t filter_select);

};

#endif // !WL_LOADER