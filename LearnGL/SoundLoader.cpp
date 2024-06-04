#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"




ALuint Audio_Handler::load_WL(const char* filepathRel) {
	ALuint out;
	

	const std::string filePath = "Assets/" + (std::string)filepathRel +".wl";

	std::ifstream stream(filePath, std::ios::binary);

	stream.read(reinterpret_cast<char*>(&uninitialised.magic), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.block_count), sizeof(uint32_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.sample_rate), sizeof(uint16_t));

	const uint32_t pcm_sample_count = SAMPLES_PER_BLOCK * uninitialised.block_count;

	uninitialised.data = new Block[uninitialised.block_count];

	rawAudio = new int16_t[pcm_sample_count];

	uint8_t temp = 0;

	for (size_t i = 0; i < uninitialised.block_count; i++)
	{
		stream.read(reinterpret_cast<char*>(&uninitialised.data[i].shift_filter), sizeof(uint8_t));
		stream.read(reinterpret_cast<char*>(&uninitialised.data[i].flags), sizeof(uint8_t));

		for (size_t j = 0; j < SAMPLES_PER_BLOCK; j += 2)
		{
			stream.read(reinterpret_cast<char*>(&temp), sizeof(uint8_t));

			uninitialised.data[i].Samples[j].value = ((temp >> 4) & 0xF);
			uninitialised.data[i].Samples[(j + 1)].value = (temp & 0xF);

		}
	}

	int16_t temp16 = 0;
	uint8_t sh_val = 0;
	int16_t old = 0;
	int16_t older = 0;
	int16_t oldest = 0;

	const int16_t rounding_table[4] = { 2,3,7,8 };


	for (size_t i = 0; i < uninitialised.block_count * 28; i++)
	{

		sh_val = (uninitialised.data[i / 28].shift_filter);

		sh_val >>= 4;

		if (i > 0) {
			old = rawAudio[i - 1];
		}
		if (i > 1) {
			older = rawAudio[i - 2];
		}
		if (i > 2) {
			oldest = rawAudio[i - 3];
		}


		temp16 = (int16_t)(uninitialised.data[i / 28].Samples[i % 28].value);

		if (temp16 > MAX_INT_4) {
			temp16 = 0 - (temp16 - MAX_INT_4);
		}

		else if (temp16 < MIN_INT_4) {
			temp16 = 0 - (temp16 - MIN_INT_4);
		}

		temp16 <<= sh_val;


		temp16 -= old;

		int32_t out = 0;

		out += ((int32_t)oldest * rounding_table[0] / DIVISOR);
		out += ((int32_t)older * rounding_table[1] / DIVISOR);
		out += ((int32_t)old * rounding_table[2] / DIVISOR);
		out += ((int32_t)temp16 * rounding_table[3] / DIVISOR);



		rawAudio[i] = (int16_t)out;


	}




	alGenBuffers(1, &out);
	alBufferData(out, AL_FORMAT_MONO16, rawAudio, (pcm_sample_count * sizeof(int16_t)), uninitialised.sample_rate);
	delete[] rawAudio;
	delete[] uninitialised.data;

	return out;
}