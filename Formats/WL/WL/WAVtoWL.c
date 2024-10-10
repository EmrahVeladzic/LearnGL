#define _CRT_SECURE_NO_WARNINGS
#include "WAV.h"
#include <stdio.h>
#include <stdlib.h>
#include "WL.h"

WAV IMPORTED;

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

int open_WAV() {



	printf("Enter a WAV file name.\n");

	filepath = string_in();

	printf("Enter a WL file name.\n");

	fileexpath = string_in();

	fileio = fopen(filepath, "rb");

	fread(&IMPORTED.magic, sizeof(uint8_t), 4, fileio);

	fread(&IMPORTED.file_size, sizeof(uint32_t), 1, fileio);

	fread(&IMPORTED.file_type, sizeof(uint8_t), 4, fileio);

	fread(&IMPORTED.fmt_marker, sizeof(uint8_t), 4, fileio);

	fread(&IMPORTED.fmt_size, sizeof(uint32_t), 1, fileio);

	fread(&IMPORTED.format, sizeof(uint16_t), 1, fileio);

	fread(&IMPORTED.num_of_channels, sizeof(uint16_t), 1, fileio);

	fread(&IMPORTED.sample_rate, sizeof(uint32_t), 1, fileio);

	fread(&IMPORTED.byte_rate, sizeof(uint32_t), 1, fileio);

	fread(&IMPORTED.block_align, sizeof(uint16_t), 1, fileio);

	fread(&IMPORTED.bits_per_sample, sizeof(uint16_t), 1, fileio);


	fread(&IMPORTED.data_marker, sizeof(uint8_t), 4, fileio);

	fread(&IMPORTED.data_section_size, sizeof(uint32_t), 1, fileio);

	
	

	Encoder.sample_count = IMPORTED.data_section_size / sizeof(WAV16);

	int temp = Encoder.sample_count % 28;

	Encoder.sample_count += temp;

	Encoder.spu_sample_count = (Encoder.sample_count / 28);


	Encoder.sample_buffer = (WAV16*)malloc(Encoder.sample_count * sizeof(WAV16));

	Encoder.differential_buffer = (WAV16*)malloc(Encoder.sample_count * sizeof(WAV16));

	Encoder.spu_buffer = (SPU_sample*)malloc(Encoder.spu_sample_count * sizeof(SPU_sample));

	if (IMPORTED.num_of_channels > 28)
	{
		return 1;
	}

	for (size_t i = 0; i < Encoder.sample_count; i++)
	{

		Encoder.sample_buffer[i].data = 0;

		fread(&Encoder.sample_buffer[i].data, sizeof(WAV16), 1, fileio);

		if (i >= (size_t)IMPORTED.num_of_channels) {
			Encoder.differential_buffer[i].data = Encoder.sample_buffer[i].data- Encoder.sample_buffer[i - (size_t)IMPORTED.num_of_channels].data;
		}
		else
		{
			Encoder.differential_buffer[i].data = Encoder.sample_buffer[i].data;
		}
	}

	fclose(fileio);


	return 0;
}

int Export() {


	NewWL.magic = 87;
	NewWL.num_of_channels = (uint8_t)(IMPORTED.num_of_channels);
	NewWL.clamp_bits = (MAX_TRESHOLD / 2) -1;
	NewWL.block_count = Encoder.spu_sample_count;
	NewWL.default_sample_rate = (uint16_t)IMPORTED.sample_rate;
	NewWL.data = (SPU_sample*)malloc(NewWL.block_count * sizeof(SPU_sample));

	for (size_t i = 0; i < NewWL.block_count; i++)
	{
		NewWL.data[i].flags = Encoder.spu_buffer[i].flags;
		NewWL.data[i].shift_filter = Encoder.spu_buffer[i].shift_filter;
		for (size_t j = 0; j < 28; j++)
		{
			NewWL.data[i].samples[j].value = Encoder.spu_buffer[i].samples[j].value;
		}
	}


	fileio = fopen(fileexpath, "wb");

	

	fwrite(&NewWL.magic, sizeof(uint8_t), 1, fileio);
	fwrite(&NewWL.num_of_channels, sizeof(uint8_t), 1, fileio);
	fwrite(&NewWL.clamp_bits, sizeof(uint8_t), 1, fileio);
	fwrite(&NewWL.block_count, sizeof(uint32_t), 1, fileio);
	fwrite(&NewWL.default_sample_rate, sizeof(uint16_t), 1, fileio);


	uint8_t temp;

	for (size_t i = 0; i < NewWL.block_count; i++)
	{
		fwrite(&NewWL.data[i].shift_filter, sizeof(uint8_t), 1, fileio);
		fwrite(&NewWL.data[i].flags, sizeof(uint8_t), 1, fileio);

		for (size_t j = 0; j < 14; j++)
		{
			temp = (((NewWL.data[i].samples[j * 2].value & 0xF) << 4) | (NewWL.data[i].samples[(j * 2) + 1].value & 0xF));





			fwrite(&temp, sizeof(uint8_t), 1, fileio);
		}
	}

	fclose(fileio);

	return 0;
}

int Reconvert() {

	printf("Enter a new WAV file name.\n");

	filepath = string_in();


	fileio = fopen(filepath, "wb");

	IMPORTED.file_size = Encoder.spu_sample_count * 28 * sizeof(int16_t);


	fwrite(&IMPORTED.magic, sizeof(uint8_t), 4, fileio);

	fwrite(&IMPORTED.file_size, sizeof(uint32_t), 1, fileio);

	fwrite(&IMPORTED.file_type, sizeof(uint8_t), 4, fileio);

	fwrite(&IMPORTED.fmt_marker, sizeof(uint8_t), 4, fileio);

	fwrite(&IMPORTED.fmt_size, sizeof(uint32_t), 1, fileio);

	fwrite(&IMPORTED.format, sizeof(uint16_t), 1, fileio);

	fwrite(&IMPORTED.num_of_channels, sizeof(uint16_t), 1, fileio);

	fwrite(&IMPORTED.sample_rate, sizeof(uint32_t), 1, fileio);

	fwrite(&IMPORTED.byte_rate, sizeof(uint32_t), 1, fileio);

	fwrite(&IMPORTED.block_align, sizeof(uint16_t), 1, fileio);

	fwrite(&IMPORTED.bits_per_sample, sizeof(uint16_t), 1, fileio);


	fwrite(&IMPORTED.data_marker, sizeof(uint8_t), 4, fileio);

	fwrite(&IMPORTED.data_section_size, sizeof(uint32_t), 1, fileio);


	for (size_t i = 0; i < Encoder.sample_count; i++)
	{
		fwrite(&Encoder.sample_buffer[i].data, sizeof(int16_t), 1, fileio);
	}

	fclose(fileio);



	return 0;
}

int open_WL() {

	NewWL.data = (SPU_sample*)malloc(Encoder.spu_sample_count * sizeof(SPU_sample));

	fileio = fopen(fileexpath, "rb");



	fread(&NewWL.magic, sizeof(uint8_t), 1, fileio);
	fread(&NewWL.num_of_channels, sizeof(uint8_t), 1, fileio);
	fread(&NewWL.clamp_bits, sizeof(uint8_t), 1, fileio);
	fread(&NewWL.block_count, sizeof(uint32_t), 1, fileio);
	fread(&NewWL.default_sample_rate, sizeof(uint16_t), 1, fileio);

	uint8_t temp;


	Encoder.sample_count = NewWL.block_count * 28;

	if (NewWL.data != NULL) {

		for (size_t i = 0; i < (size_t)NewWL.block_count; i++)
		{

			fread(&NewWL.data[i].shift_filter, sizeof(uint8_t), 1, fileio);
			fread(&NewWL.data[i].flags, sizeof(uint8_t), 1, fileio);


			for (size_t j = 0; j < 14; j++)
			{


				fread(&temp, sizeof(uint8_t), 1, fileio);


				NewWL.data[i].samples[j * 2].value = ((temp >> 4) & 0xF);
				NewWL.data[i].samples[(j * 2) + 1].value = (temp & 0xF);




			}


		}

	}

	fclose(fileio);


	return 0;
}



int sanity_check() {


	printf("\nResult:\nHeader: %c%c%c%c\n", IMPORTED.magic[0], IMPORTED.magic[1], IMPORTED.magic[2], IMPORTED.magic[3]);
	printf("File Size: %d\n", IMPORTED.file_size);
	printf("File Type: %c%c%c%c\n", IMPORTED.file_type[0], IMPORTED.file_type[1], IMPORTED.file_type[2], IMPORTED.file_type[3]);
	printf("FMT Marker: %c%c%c%c\n", IMPORTED.fmt_marker[0], IMPORTED.fmt_marker[1], IMPORTED.fmt_marker[2], IMPORTED.fmt_marker[3]);
	printf("FMT Size: %d\n", IMPORTED.fmt_size);
	printf("Format: %d\n", IMPORTED.format);
	printf("Channel Count: %d\n", IMPORTED.num_of_channels);
	printf("Sample Rate: %d\n", IMPORTED.sample_rate);
	printf("Byte Rate: %d\n", IMPORTED.byte_rate);
	printf("Block Alignment: %d\n", IMPORTED.block_align);
	printf("Bits Per Sample: %d\n", IMPORTED.bits_per_sample);
	printf("Data Marker: %c%c%c%c\n", IMPORTED.data_marker[0], IMPORTED.data_marker[1], IMPORTED.data_marker[2], IMPORTED.data_marker[3]);
	printf("Data Section Size: %d\n\n", IMPORTED.data_section_size);

	return 0;
}