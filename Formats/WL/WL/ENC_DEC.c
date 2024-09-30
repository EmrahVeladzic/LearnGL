#include "WAV.h"
#include "WL.h"


#define DIVISOR 5

const uint16_t rounding_table[4] = { 2,3,7,8 };

#define MAX_INT_4 7
#define MIN_INT_4 -8

#define MIN_TRESHOLD -30000
#define MAX_TRESHOLD 30000


uint8_t calculate_shift(WAV_BLOCK in) {
	uint8_t shift = 0;
	uint8_t repeat = 1; 

	WAV_BLOCK testBlock;





	while (repeat == 1)
	{
		repeat = 0;



		for (size_t i = 0; i < 28; i++)
		{
			testBlock.data[i] = in.data[i] >> shift;
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
			return shift;
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
			ActiveBlock.data[j] = (uint16_t)((int)Encoder.differential_buffer[(i * 28) + j].data);



			if (ActiveBlock.data[j] > MAX_TRESHOLD) {
				ActiveBlock.data[j] = MAX_TRESHOLD;
			}

			if (ActiveBlock.data[j] < MIN_TRESHOLD) {
				ActiveBlock.data[j] = MIN_TRESHOLD;
			}



		}

		uint8_t sh_val = calculate_shift(ActiveBlock);

		Encoder.spu_buffer[i].shift_filter = (sh_val << 4) & 0xF0;


		for (size_t j = 0; j < 28; j++)
		{
			temp = ActiveBlock.data[j];





			Encoder.spu_buffer[i].samples[j].value = (uint8_t)(temp >> sh_val);






		}






	}


	return 0;
}

int Decode() {

	int16_t temp16 = 0;
	uint8_t sh_val = 0;
	int16_t old = 0;
	int16_t older = 0;
	int16_t oldest = 0;


	for (size_t i = 0; i < Encoder.sample_count; i++)
	{

		sh_val = (NewWL.data[i / 28].shift_filter);

		sh_val >>= 4;

		if (i > 0) {
			old = (int16_t)Encoder.sample_buffer[i - 1].data;
		}
		if (i > 1) {
			older = (int16_t)Encoder.sample_buffer[i - 2].data;
		}
		if (i > 2) {
			oldest = (int16_t)Encoder.sample_buffer[i - 3].data;
		}


		temp16 = (int16_t)(NewWL.data[i / 28].samples[i % 28].value);

		if (temp16 > MAX_INT_4) {
			temp16 = 0 - (temp16 - MAX_INT_4);
		}

		if (temp16 < MIN_INT_4) {
			temp16 = 0 - (temp16 - MIN_INT_4);
		}


		temp16 <<= sh_val;


		temp16 -= old;

		int32_t out = 0;

		out += ((int32_t)oldest * rounding_table[0] / DIVISOR);
		out += ((int32_t)older * rounding_table[1] / DIVISOR);
		out += ((int32_t)old * rounding_table[2] / DIVISOR);
		out += ((int32_t)temp16 * rounding_table[3] / DIVISOR);



		Encoder.sample_buffer[i].data = (int16_t)out;


	}


	return 0;
}