#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"




ALuint Audio_Handler::load_WL(const char* filepathRel) {
	ALuint out;
	uint8_t temp = 0;

	const std::string filePath = "Assets/" + (std::string)filepathRel +".WL";

	std::ifstream stream(filePath, std::ios::binary);

	stream.read(reinterpret_cast<char*>(&uninitialised.magic), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.num_of_channels), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.clamp_bits), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.block_count), sizeof(uint32_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.sample_rate), sizeof(uint16_t));

	const uint32_t pcm_sample_count = SAMPLES_PER_BLOCK * uninitialised.block_count;

	uninitialised.data = new Block[uninitialised.block_count];

	rawAudio = new int16_t[pcm_sample_count]();

	

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
	uint16_t tempu16 = 0;



	for (size_t i = 0; i < uninitialised.block_count * SAMPLES_PER_BLOCK; i++)
	{
			

		sh_val = ((uninitialised.data[i / SAMPLES_PER_BLOCK].shift_filter) >> 4) & 0xF;



			tempu16 = (uint16_t)(uninitialised.data[i / SAMPLES_PER_BLOCK].Samples[i % SAMPLES_PER_BLOCK].value & 0xF);
			if (tempu16 & 0x8) {
				tempu16 |= 0xFFF0;
			}


			tempu16 *= (1 << (sh_val + uninitialised.clamp_bits));



			rawAudio[i] = (int16_t)tempu16;




			if (interpolateWL && i >= (size_t)uninitialised.num_of_channels * 3)
			{


				temp16 = (rawAudio[i] - rawAudio[i - (size_t)(uninitialised.num_of_channels * 3)]) / 4;

				rawAudio[i - (size_t)(uninitialised.num_of_channels * 1)] += temp16;

				temp16 += (temp16 /= 2);

				rawAudio[i - (size_t)(uninitialised.num_of_channels * 2)] += temp16;



			}



		

	}


	alGenBuffers(1, &out);
	alBufferData(out, (uninitialised.num_of_channels==1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16, rawAudio, (pcm_sample_count * sizeof(int16_t)), uninitialised.sample_rate);
	delete[] rawAudio;
	delete[] uninitialised.data;

	return out;
}