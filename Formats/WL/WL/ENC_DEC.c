#include "WAV.h"
#include "WL.h"
#include <math.h>

#define MAX_INT_16 32767
#define MIN_INT_16 -32768

#define LOW_THRESHOLD -16000
#define HIGH_THRESHOLD 16000

const uint16_t rounding_table[28] = { 1000,750,0,50,100,150,200, 250,300,350,400,450,500,550, 600,650,700,750,800,850,900, 950,960,970,980,990,0,0 };

#define MAX_INT_4 7
#define MIN_INT_4 -8

#define DIFF_THRESHOLD 500

#define PICK_UP_TRESHOLD 2000

uint16_t last_value = 0;
uint16_t future_value = 0;

uint8_t calculate_shift(WAV_BLOCK in) {
	uint8_t shift = 0;
	uint8_t repeat = 1;

	WAV_BLOCK testBlock;

	



	while (repeat == 1)
	{
		repeat = 0;



		for (size_t i = 0; i < 28; i++)
		{
			testBlock.data[i] = in.data[i]>>shift;
		}

		



		for (size_t i = 0; i < 28; i++)
		{
			if ((int)testBlock.data[i] > (int)MAX_INT_4 || (int)testBlock.data[i] < (int)MIN_INT_4) {
				repeat = 1;

			}
		}

		if (repeat == 1) {
			shift++;
		}
		else
		{
			break;
		}
	}




	return shift;
}

int Encode() {

	uint16_t temp;
	
	for (size_t i = 0; i < Encoder.spu_sample_count; i++)
	{
		for (size_t j = 0; j < 28; j++)
		{
			ActiveBlock.data[j] = (uint16_t)Encoder.sample_buffer[(i * 28) + j].data;

			if (ActiveBlock.data[j] > HIGH_THRESHOLD) {
				ActiveBlock.data[j] = HIGH_THRESHOLD;
			}

			if (ActiveBlock.data[j] < LOW_THRESHOLD) {
				ActiveBlock.data[j] = LOW_THRESHOLD;
			}
		}

		uint8_t sh_val = calculate_shift(ActiveBlock);

		Encoder.spu_buffer[i].shift_filter = (sh_val << 4) & 0xF0;


		for (size_t j = 0; j < 28; j++)
		{
			temp = (uint16_t)(Encoder.sample_buffer[(i * 28) + j].data);


			if (temp > HIGH_THRESHOLD) {
				temp = HIGH_THRESHOLD;
			}

			if (temp < LOW_THRESHOLD) {
				temp = LOW_THRESHOLD;
			}

			if ((i * 28) + j > 0) {
				last_value = Encoder.sample_buffer[(i * 28) + j - 1].data;
			}
			
			
			
			if ((i * 28) + j < ((Encoder.spu_sample_count*28) + 27)) {
				future_value = Encoder.sample_buffer[(i * 28) + j + 1].data;
			}

			if (temp == last_value || temp == future_value) {

				Encoder.spu_buffer[i].samples[j].value = (uint8_t)(temp >> (sh_val+1));
			}

			else
			{
				Encoder.spu_buffer[i].samples[j].value = (uint8_t)(temp >> sh_val);
			}
			
			
			

			
		}

		

	
		

	}


	return 0;
}

int Decode() {

	uint16_t temp16 =0;
	uint8_t sh_val =0;

	for (size_t i = 0; i < Encoder.sample_count; i++)
	{

		sh_val = (NewWL.data[i/ 28].shift_filter);

		sh_val >>= 4;

		if (i > 0) {
			last_value = Encoder.sample_buffer[i - 1].data;
		}

		if (i < (Encoder.sample_count)-1) {
			future_value = Encoder.sample_buffer[i+1].data;
		}

		temp16 = (uint16_t)(NewWL.data[i / 28].samples[i % 28].value);

		temp16 = temp16 << sh_val;

		if (abs(abs((int16_t)last_value) - abs((int16_t)temp16)) < DIFF_THRESHOLD) {

			if (abs((int16_t)temp16) >= PICK_UP_TRESHOLD && temp16!=0) {
				temp16 =(uint16_t)((int16_t)temp16 - ((int16_t)temp16/abs((int16_t)temp16))*rounding_table[i % 28]);
			}
		}
		

		Encoder.sample_buffer[i].data = (int16_t)temp16;
		
	}


	return 0;
}