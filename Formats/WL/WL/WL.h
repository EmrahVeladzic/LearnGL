#ifndef WL_HEADER
#define WL_HEADER

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int open_WL();

int open_WAV();

int sanity_check();

int Reconvert();

int Export();

int Encode();

int Decode();

char* filepath;

char* fileexpath;

FILE* fileio;




#pragma pack(push,1)
typedef struct adpcm_value {

	uint8_t value : 4;

}four_bit;
#pragma pack (pop)

#pragma pack(push,1)
typedef struct spu_adpcm {

	uint8_t shift_filter;
	uint8_t flags;

	four_bit samples[28];


}SPU_sample;
#pragma pack (pop)

#pragma pack(push,1)
typedef struct wl_encoder {



	uint32_t sample_count;
	uint32_t spu_sample_count;

	WAV16* sample_buffer;

	SPU_sample* spu_buffer;

	WAV16* differential_buffer;

}WL_ENC;
#pragma pack (pop)

WL_ENC Encoder;

typedef struct wl_file {

	uint8_t magic;
	uint16_t default_sample_rate;
	uint32_t block_count;
	SPU_sample* data;


}WL_FILE;

typedef struct wav_28 {

	int16_t data[28];

}WAV_BLOCK;

WAV_BLOCK ActiveBlock;

WL_FILE NewWL;

#endif