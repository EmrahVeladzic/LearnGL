#ifndef WL_LOADER
#define WL_LOADER

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>



#define SAMPLES_PER_BLOCK 28

#define DIFF_THRESHOLD 500

#define PICK_UP_TRESHOLD 2000


const uint16_t rounding_table[28] = { 1000,750,0,50,100,150,200, 250,300,350,400,450,500,550, 600,650,700,750,800,850,900, 950,960,970,980,990,0,0 };



class Audio_Handler {
private:
	int16_t* rawAudio;



	WL uninitialised;

public:

	ALuint load_WL(const char* filepathRel);

};

#endif // !WL_LOADER