#define _CRT_SECURE_NO_WARNINGS



#include "BMP.h"
#include <stdio.h>
#include "RPF.h"
#include<string.h>
#include<stdlib.h>
#include<math.h>

BMP IMPORTED;


const int a = 0;



char* string_in() {
	char* input = NULL;
	int size = 1;
	char ch;
	char* temp;

	do {
		ch = getchar();

		size++;

		if (input != NULL) {

			temp = (char*)realloc(input, size);

			if (temp != NULL) {


				input = temp;
			}


		}
		else
		{
			input = (char*)malloc(size);
		}

		if (input != NULL) {
			input[size - 2] = ch;
		}

	} while (ch != '\0' && ch != '\n');


	if (input != NULL) {
		input[size - 2] = '\0';
	}


	return input;
}

int BMPimport() {

	printf("Enter the filename of a BMP file (with extension) \n");

	fileName = string_in();

	printf("Enter the filename of a RPF file you wish to export to (with extension) \n");

	fileexnm = string_in();




	FILE* fileloc = fopen(fileName, "rb");

	if (fileloc == NULL)
	{
		return 1;
	}

	if (a == 0) {

		fread(&IMPORTED.magic, sizeof(uint16_t), 1, fileloc);

		fread(&IMPORTED.file_size, sizeof(uint32_t), 1, fileloc);
		fread(&IMPORTED.reserved, sizeof(uint32_t), 1, fileloc);
		fread(&IMPORTED.data_offset, sizeof(uint32_t), 1, fileloc);


		fread(&IMPORTED.header_size, sizeof(uint32_t), 1, fileloc);
		fread(&IMPORTED.width, sizeof(int32_t), 1, fileloc);
		fread(&IMPORTED.height, sizeof(int32_t), 1, fileloc);

		fread(&IMPORTED.planes, sizeof(uint16_t), 1, fileloc);
		fread(&IMPORTED.bpp, sizeof(uint16_t), 1, fileloc);

		fread(&IMPORTED.compression, sizeof(uint32_t), 1, fileloc);
		fread(&IMPORTED.img_size, sizeof(uint32_t), 1, fileloc);

		fread(&IMPORTED.xpixelperm, sizeof(int32_t), 1, fileloc);
		fread(&IMPORTED.ypixelperm, sizeof(int32_t), 1, fileloc);

		fread(&IMPORTED.colors_used, sizeof(uint32_t), 1, fileloc);
		fread(&IMPORTED.important_colors, sizeof(uint32_t), 1, fileloc);


		fseek(fileloc, IMPORTED.data_offset, SEEK_SET);

	}


	SCALEX = IMPORTED.width;
	SCALEY = IMPORTED.height;
	

	if (SCALEX > 256 || SCALEY > 256 || DEPTH > 256 || SCALEX < 1 || SCALEY < 1 || DEPTH < 1 || SCALEX%2!=0||SCALEY%2!=0)
	{
		return 1;
	}



	Compressor.ALPHA.r = 255;
	Compressor.ALPHA.g = 0;
	Compressor.ALPHA.b = 255;

	Compressor.ALPHA15 = Convert_to_15Bit(Compressor.ALPHA);


	NewRPF.data = (uint8_t*)malloc(sizeof(uint8_t) * (SCALEX * SCALEY));
	

	Compressor.RESIZED_CLUT = (Pixel15*)malloc(sizeof(Pixel15) * DEPTH);
	for (size_t i = 0; i < (size_t)DEPTH; i++)
	{
		Compressor.RESIZED_CLUT[i] = Compressor.ALPHA15;
	}


	Compressor.LARGE_CLUT = (Pixel*)malloc(sizeof(Pixel) * (SCALEX * SCALEY));
	Compressor.LARGE_CLUT15 = (Pixel15*)malloc(sizeof(Pixel15) * (SCALEX * SCALEY));

	for (size_t i = 0; i < (SCALEX * SCALEY); i++)
	{
		Compressor.LARGE_CLUT[i] = Compressor.ALPHA;
		Compressor.LARGE_CLUT15[i] = Compressor.ALPHA15;
	}

	Compressor.Occurence_Table = (Occurence_Entry*)malloc(sizeof(Occurence_Entry));
	Compressor.Occurence_Table[0].Value = Compressor.ALPHA15;
	Compressor.Occurence_Table[0].Occurence = 0;



	Compressor.SwapCount = 0;

	Compressor.Occurence_Table[0].Value = Compressor.ALPHA15;
	Compressor.UNIQUE_PIXEL_COUNT = 1;

	Compressor.RESIZED_CLUT[0] = Compressor.ALPHA15;
	
	ProtectedBuffer = (Pixel15*)malloc(PROTECTED_BUFFER_SIZE * sizeof(Pixel15));


	for (int i = 0; i < (SCALEX * SCALEY); i++)
	{

		fread(&Compressor.LARGE_CLUT[i].b, sizeof(uint8_t), 1, fileloc);
		fread(&Compressor.LARGE_CLUT[i].g, sizeof(uint8_t), 1, fileloc);
		fread(&Compressor.LARGE_CLUT[i].r, sizeof(uint8_t), 1, fileloc);

		Compressor.LARGE_CLUT15[i] = Convert_to_15Bit(Compressor.LARGE_CLUT[i]);
	}

	fclose(fileloc);

	ProtectedBufferAccess = 0;


	

	printf("\nPlease select a method for compressing.\n0 - Popularity method \t 1 - Proximity method\n");

	mode_in = string_in();

	if (mode_in[0] == '0') {

		method.value = 0;
	}
	else if(mode_in[0] == '1')
	{
		method.value = 1;
	}
	else
	{
		return 1;
	}



		


	return 0;
}


int Export() {


	FILE* tempoutf = fopen(fileexnm, "wb");



	fwrite(NewRPF.magic, sizeof(uint8_t), 4, tempoutf);

	uint16_t wrt15;


	

	for (size_t i = 0; i < (size_t)NewRPF.magic[1]+1; i++)
	{


		wrt15 = (NewRPF.CLUT[i].a << 0) | (NewRPF.CLUT[i].b << 11) | (NewRPF.CLUT[i].g << 6) | (NewRPF.CLUT[i].r << 1);

		fwrite(&wrt15, sizeof(uint16_t), 1, tempoutf);


	

	}




	if ((NewRPF.magic[1] + 1 )<= 16)
	{
		fwrite(NewRPF.data, sizeof(uint8_t), ((SCALEX  * SCALEY )/2), tempoutf);


		
	}

	

	else
	{
		fwrite(NewRPF.data, sizeof(uint8_t), (SCALEX * SCALEY), tempoutf);
	}

	


	

	fclose(tempoutf);


	return 0;
}

