#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"



ALuint Audio_Handler::load_WL(const char* filepathRel) {
	ALuint out;

	std::string filepath = "Assets/" + (std::string)filepathRel;

	std::ifstream filestr = std::ifstream(filepath,std::ios::in);

	filestr.read(reinterpret_cast<char*>(&uninitialised.magic),sizeof(uint8_t));
	filestr.read(reinterpret_cast<char*>(&uninitialised.block_count), sizeof(uint32_t));
	filestr.read(reinterpret_cast<char*>(&uninitialised.sample_rate), sizeof(uint16_t));

	

	uninitialised.data = new Block[uninitialised.block_count];

	rawAudio = new int16_t[uninitialised.block_count * SAMPLES_PER_BLOCK];



	int8_t reader;
	for (size_t i = 0; i < (size_t) uninitialised.block_count; i++)
	{
		filestr.read(reinterpret_cast<char*>(&uninitialised.data[i].shift_filter), sizeof(uint8_t));
		filestr.read(reinterpret_cast<char*>(&uninitialised.data[i].flags), sizeof(uint8_t));

		for (size_t j = 0; j < SAMPLES_PER_BLOCK; j+=2)
		{
			filestr.read(reinterpret_cast<char*>(&reader),sizeof(int8_t));

			uninitialised.data[i].Samples[j*2].value = (reader >> 0) & 0x08;
			uninitialised.data[i].Samples[(j*2)+1].value = (reader >> 4) & 0x08;


		}
	}



	

	uint16_t temp16 = 0;
	uint8_t sh_val = 0;

	for (size_t i = 0; i < uninitialised.block_count * SAMPLES_PER_BLOCK; i++)
	{

		sh_val = (uninitialised.data[i / SAMPLES_PER_BLOCK].shift_filter);

		sh_val >>= 4;


		temp16 = (uint16_t)(uninitialised.data[i / SAMPLES_PER_BLOCK].Samples[i % SAMPLES_PER_BLOCK].value);


		temp16 += rounding_table[i % SAMPLES_PER_BLOCK];


		temp16 = temp16 << sh_val;

		


		rawAudio[i] = (int16_t)temp16;

	}
	
	std::cout << "HI";

	


	delete[] uninitialised.data;

	alGenBuffers(1,&out);
	alBufferData(out,AL_FORMAT_MONO16,rawAudio,sizeof(int16_t)*(uninitialised.block_count* SAMPLES_PER_BLOCK),uninitialised.sample_rate);




	delete[] rawAudio;

	return out;
}