#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include "CustomSoundFormat.hpp"
#include <vector>
#include <fstream>
#include "SoundLoader.hpp"

#define MAX_INT_16 32767
#define MIN_INT_16 -32768

#define MIN_STEP 1500
#define STEP_SIZE 150
#define MAX_STEP 3000
#define DEFAULT_SHIFT 4




int16_t adapt_step(int16_t pred, int16_t current_step, int8_t shift) {
	int16_t new_step;

	if (current_step < pred << shift)
	{
		new_step = current_step + STEP_SIZE;

		if (new_step > MAX_STEP) {
			new_step = MAX_STEP;
		}

	}

	else if (current_step > pred << shift)
	{
		new_step = current_step - STEP_SIZE;

		if (new_step < MIN_STEP)
		{
			new_step = MIN_STEP;
		}
	}

	else
	{
		new_step = current_step;
	}

	return new_step;
}



ALuint Audio_Handler::load_WL(const char* filepathRel) {
	ALuint out;

	std::string filepath = "Assets/" + (std::string)filepathRel;

	std::ifstream filestr = std::ifstream(filepath,std::ios::in);

	filestr.read(reinterpret_cast<char*>(&uninitialised.magic),sizeof(uint8_t));
	filestr.read(reinterpret_cast<char*>(&uninitialised.block_count), sizeof(uint16_t));
	filestr.read(reinterpret_cast<char*>(&uninitialised.sample_rate), sizeof(uint16_t));



	uninitialised.data = new Block[uninitialised.block_count];

	rawAudio = new int16_t[uninitialised.block_count * 28];



	int8_t reader;
	for (size_t i = 0; i < (size_t) uninitialised.block_count; i++)
	{
		filestr.read(reinterpret_cast<char*>(&uninitialised.data[i].shift_filter), sizeof(uint8_t));
		filestr.read(reinterpret_cast<char*>(&uninitialised.data[i].flags), sizeof(uint8_t));

		for (size_t j = 0; j < 28; j+=2)
		{
			filestr.read(reinterpret_cast<char*>(&reader),sizeof(int8_t));

			uninitialised.data[i].Samples[j].value = (reader >> 0) & 0x08;
			uninitialised.data[i].Samples[j+1].value = (reader >> 4) & 0x08;


		}
	}

	Four_bit shiftval;

	int16_t shift_step = STEP_SIZE;

	int64_t buffer_offset;
	int64_t buffer_pos;

	int16_t prediction_error;

	int32_t temp;

	int16_t newval;

	for (size_t i = 0; i < (size_t)(uninitialised.block_count*28); i++)
	{

			buffer_offset = (int64_t)i % 28;
			buffer_pos = ((int64_t)i - buffer_offset) / 28;

			shiftval.value = uninitialised.data[buffer_pos].shift_filter >> 4;

			if (buffer_offset == 0 && buffer_pos > 0) {
				temp = uninitialised.data[buffer_pos - 1].Samples[27].value * shift_step;
				prediction_error = (int16_t)(temp >> shiftval.value);
			}
			else {
				temp = uninitialised.data[buffer_pos].Samples[buffer_offset].value * shift_step;
				prediction_error = (int16_t)(temp >> shiftval.value);
			}

			shift_step = adapt_step(prediction_error, shift_step,shiftval.value);

			

			newval = (uninitialised.data[buffer_pos].Samples[buffer_offset].value * shift_step) + prediction_error;
			rawAudio[i]= newval;
		
			

	}

	delete[] uninitialised.data;

	alGenBuffers(1,&out);
	alBufferData(out,AL_FORMAT_STEREO16,rawAudio,sizeof(int16_t)*(uninitialised.block_count*28),uninitialised.sample_rate);




	delete[] rawAudio;

	return out;
}