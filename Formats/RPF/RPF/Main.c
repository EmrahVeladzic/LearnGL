#include <stdio.h>
#include "BMP.h"
#include "RPF.h"





int main() {

	flow.value = BMPimport();


	if (flow.value == 0) {

		flow.value = Compress();

	}

	else
	{
		printf("\nError processing image.\nMake sure that the image is a BMP file, with a maximum size of 256x256 and a bit depth of 24.\n");
		char end = getchar();
		return 1;
	}

	if (flow.value == 0) {

		flow.value = Export();

	}

	else
	{
		printf("\nError compressing image.\nThis is a unhandled exception.\n");
		char end = getchar();
		
	}




	if (NewRPF.CLUT != NULL) {
		free(NewRPF.CLUT);
	}


	if (NewRPF.data != NULL) {
		free(NewRPF.data);
	}

	if (NewRPF.data16max != NULL) {
		free(NewRPF.data16max);
	}

	if (Compressor.LARGE_CLUT != NULL) {

		free(Compressor.LARGE_CLUT);

	}


	if (Compressor.Swap_Table != NULL) {
		free(Compressor.Swap_Table);
	}


	if (Compressor.Occurence_Table != NULL) {
		free(Compressor.Occurence_Table);
	}





	return 0;
}