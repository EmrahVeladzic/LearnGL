#include "WAV.h"
#include "WL.h"

int main(int argc, char** argv) {

	open_WAV();

	//sanity_check();

	Encode();

	Export();

	

	//open_WL();

	//Decode();

	//Reconvert();
	


	free(Encoder.sample_buffer);
	free(Encoder.spu_buffer);
	free(NewWL.data);

	return 0;
}