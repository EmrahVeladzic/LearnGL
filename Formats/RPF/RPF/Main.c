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
		printf("\nError processing image.\nMake sure that the image is a BMP file, with a maximum size of 256x256 and a bit depth of 24, and that you selected a valid method.\n");
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

	

	if (Compressor.LARGE_CLUT != NULL) {

		free(Compressor.LARGE_CLUT);

	}


	if (Compressor.Swap_Table != NULL) {
		free(Compressor.Swap_Table);
	}


	if (Compressor.Occurence_Table != NULL) {
		free(Compressor.Occurence_Table);
	}


	if (ProtectedBuffer != NULL) {
		free(ProtectedBuffer);
	}

	if (fileName != NULL) {
		free(fileName);
	}
	if (fileexnm != NULL) {

		free(fileexnm);
	}


	if (mode_in != NULL) {

		free(mode_in);
	}


	return 0;
}