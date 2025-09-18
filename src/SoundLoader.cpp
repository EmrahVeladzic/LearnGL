#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"


int32_t Audio_Handler::filter(int32_t current, int32_t old, int32_t older, uint8_t select) {

	switch (select)
	{
	case SPU_FILTER::SINGLE: { return current+(((60 * old) + 32) / 64); break; }
	case SPU_FILTER::DOUBLE_1: { return current + (((115 * old) - (52 * older) + 32) / 64); break;}
	case SPU_FILTER::DOUBLE_2: { return current + (((98 * old) - (55 * older) + 32) / 64); break; }
	case SPU_FILTER::DOUBLE_3: { return current + (((122 * old) - (60 * older) + 32) / 64); break; }
	default:
		return current;
		break;
	}



}


ALuint * Audio_Handler::load_WL(const char* filepathRel) {
	ALuint * out;
	uint8_t temp = 0;
	bool loop;

	const std::string filePath = "Assets/" + (std::string)filepathRel +".WL";

	std::ifstream stream(filePath, std::ios::binary);

	stream.read(reinterpret_cast<char*>(&uninitialised.magic), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.num_of_channels), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.clamp_bits), sizeof(uint8_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.block_count), sizeof(uint32_t));
	stream.read(reinterpret_cast<char*>(&uninitialised.sample_rate), sizeof(uint16_t));

	const uint32_t pcm_sample_count = SAMPLES_PER_BLOCK * uninitialised.block_count * uninitialised.num_of_channels;

	uninitialised.data = new Block[uninitialised.block_count*(uint32_t)uninitialised.num_of_channels];

	rawAudio = new int16_t[pcm_sample_count]();
	

	for (size_t i = 0; i < (uninitialised.block_count * (uint32_t)uninitialised.num_of_channels); i++)
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



	if (uninitialised.block_count > 0) {
		if (uninitialised.data[0].flags == 6) {
			loop = true;
		}
		else
		{
			loop = false;
		}
	}


	int16_t temp16 = 0;
	uint8_t sh_val = 0;
	int32_t temp32 = 0;
	
	int32_t old = 0;
	int32_t older = 0;

	for (size_t i = 0; i < pcm_sample_count-1; i+=2)
	{

		sh_val = uninitialised.data[i / 28].shift_filter & 0xF;

		temp = uninitialised.data[i / 28].Samples[i % 28].value;

		temp32 = (int32_t)((temp >> 4)&0xF);

		if (temp32 > 7) {
			temp32 -= 16;
		}		

		temp32 *= (1 << (12-sh_val));

		temp32 = filter(temp32, old, older, (uninitialised.data[i/28].shift_filter >> 4) & 0x7);

		temp16 =(int16_t)(temp32 &0xFFFF);

		rawAudio[i] = temp16;

		older = old;
		old = (int32_t)temp16;

		temp32 = (int32_t)((temp) & 0xF);

		if (temp32 > 7) {
			temp32 -= 16;
		}

		temp32 *= (1 <<(12-sh_val));

		temp32 = filter(temp32, old, older, (uninitialised.data[i / 28].shift_filter >> 4) & 0x7);

		temp16 = (int16_t)(temp32 & 0xFFFF);

		rawAudio[i+1] = temp16;

		older = old;
		old = (int32_t)temp16;
		


		if(((i+2)*uninitialised.num_of_channels)%pcm_sample_count==0){
			old = 0;
			older = 0;				
		}
	}

	int16_t* sorted_audio = new int16_t[pcm_sample_count];

	size_t index = 0;

	for (size_t i = 0; i < uninitialised.num_of_channels; i++)
	{

		for (size_t j = i; j < pcm_sample_count; j+=(size_t)uninitialised.num_of_channels)
		{

			sorted_audio[j] = rawAudio[index];
			index++;

		}


	}

	delete[] rawAudio;

	rawAudio = sorted_audio;

	

	out = new ALuint[2];


	alGenBuffers(1, &out[0]);
	alBufferData(out[0], (uninitialised.num_of_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, rawAudio, (pcm_sample_count * sizeof(int16_t)), uninitialised.sample_rate);
	delete[] rawAudio;
	delete[] uninitialised.data;

	out[1] = (loop == true) ? 1 : 0;

	return out;
}