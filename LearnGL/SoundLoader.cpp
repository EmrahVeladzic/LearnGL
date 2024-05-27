#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"



ALuint Audio_Handler::load_WL(const char* filepathRel) {
	ALuint out;

	uint16_t last_value = 0;
	
	const std::string filePath = "Assets/" + (std::string)filepathRel +".wl";

	std::ifstream stream(filePath,std::ios::binary);

	stream.read(reinterpret_cast<char*>(&uninitialised.magic),sizeof(uint8_t));
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

		for (size_t j = 0; j < SAMPLES_PER_BLOCK; j+=2)
		{
			stream.read(reinterpret_cast<char*>(&temp), sizeof(uint8_t));

			uninitialised.data[i].Samples[j].value = ((temp>>4)&0xF);
			uninitialised.data[i].Samples[(j+1)].value = (temp & 0xF);

		}
	}

	uint16_t newval = 0;

	for (size_t i = 0; i < pcm_sample_count; i++)
	{
		temp = (uninitialised.data[i / SAMPLES_PER_BLOCK].shift_filter >> 4);

		newval = (((uint16_t)uninitialised.data[i/SAMPLES_PER_BLOCK].Samples[i%SAMPLES_PER_BLOCK].value) <<temp);

		if (i > 0) {
			last_value = (uint16_t)rawAudio[i - 1];
		}

		
		if (abs(abs((int16_t)last_value) - abs((int16_t)newval)) < DIFF_THRESHOLD) {

			if (abs((int16_t)newval) >= PICK_UP_TRESHOLD && newval != 0) {
				newval = (uint16_t)((int16_t)newval - ((int16_t)newval / abs((int16_t)newval)) * rounding_table[i % 28]);
			}
		}


		rawAudio[i] = (int16_t)newval;

	}


	alGenBuffers(1, &out);
	alBufferData(out, AL_FORMAT_MONO16, rawAudio, (pcm_sample_count * sizeof(int16_t)), uninitialised.sample_rate);
	delete[] rawAudio;
	delete[] uninitialised.data;

	return out;
}