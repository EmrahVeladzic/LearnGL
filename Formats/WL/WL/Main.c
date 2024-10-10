#define _CRT_SECURE_NO_WARNINGS

#include "WAV.h"
#include "WL.h"





int main(int argc, char** argv) {




	int result = 0;

	result = open_WAV();

	if (result != 0) {

	}


	else{

		//sanity_check();

		Encode();

		Export();

		open_WL();

		Decode();

		Reconvert();

	}

	

	

	if (Encoder.sample_buffer != NULL) {
		free(Encoder.sample_buffer);
	}
	if (Encoder.sample_buffer != NULL) {
		free(Encoder.spu_buffer);
	}
	if (Encoder.sample_buffer != NULL) {
		free(NewWL.data);
	}
	if (Encoder.sample_buffer != NULL) {
		free(Encoder.differential_buffer);
	}


	
	return 0;
}

