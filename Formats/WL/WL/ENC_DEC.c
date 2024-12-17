#include "WAV.h"
#include "WL.h"




#define MAX_INT_4 7
#define MIN_INT_4 -8




uint8_t calculate_shift(WAV_BLOCK in) {
	uint8_t shift = 0;
	uint8_t repeat = 1;	

	while (repeat==1)
	{
		repeat = 0;
		int16_t divisor = (int16_t)(1 << shift);
		
	
		for (size_t i = 0; i < 28; i++)
		{
			int16_t temp = (int16_t)in.data[i];			
			

			temp /= divisor;

			if (temp<MIN_INT_4 || temp> MAX_INT_4)
			{
				

				repeat = 1;

				break;
			}


		}

		if (repeat == 1)
		{
			shift++;
		}
	}


	return shift;
	
}

int Encode() {

	int16_t max_value = (int16_t)(1 << MAX_TRESHOLD);
	


	for (size_t i = 0; i < Encoder.spu_sample_count; i++)
	{
		for (size_t j = 0; j < 28; j++)
		{
			

			
			
			

			if (Encoder.differential_buffer[(i * 28) + j].data > max_value) {
				Encoder.differential_buffer[(i * 28) + j].data = max_value;
			}

			if (Encoder.differential_buffer[(i * 28) + j].data < -max_value) {
				Encoder.differential_buffer[(i * 28) + j].data = -max_value;
			}
			
			


			ActiveBlock.data[j] = (uint16_t)(Encoder.differential_buffer[(i * 28) + j].data);





		}

		uint8_t sh_val = calculate_shift(ActiveBlock);		


		Encoder.spu_buffer[i].shift_filter = (sh_val << 4) & 0xF0;


		for (size_t j = 0; j < 28; j++)
		{			
			
			Encoder.spu_buffer[i].samples[j].value = (uint8_t)((ActiveBlock.data[j] >> sh_val) & 0x0F);
			



		}

		




	}


	return 0;
}

int Decode() {

	uint16_t tempu16 = 0;
	uint8_t sh_val = 0;
	int16_t temp16 = 0;


	for (size_t i = 0; i < Encoder.sample_count; i++)
	{

		sh_val = ((NewWL.data[i / 28].shift_filter)>>4)&0xF;

		

		tempu16 = (uint16_t)(NewWL.data[i / 28].samples[i % 28].value & 0xF);
		if (tempu16 & 0x8) {
			tempu16 |= 0xFFF0; 
		}
		
		
		tempu16 *= (1<<(sh_val+NewWL.clamp_bits));
		
		
		
		Encoder.sample_buffer[i].data = (int16_t)tempu16;
					
		

		if (i>=(size_t)NewWL.num_of_channels * 3)
		{

			
			temp16 = (Encoder.sample_buffer[i].data - Encoder.sample_buffer[i - (size_t)(NewWL.num_of_channels * 3)].data)/4;

			Encoder.sample_buffer[i - (size_t)(NewWL.num_of_channels * 2)].data += temp16;

			temp16 += (temp16 /= 2);

			Encoder.sample_buffer[i - (size_t)(NewWL.num_of_channels * 1)].data += temp16;
			

			
		}
		


	}


	

	return 0;
}