#pragma once
#include <stdint.h>

#pragma pack(push,1)
struct Four_bit {

	int8_t value : 4;

};
#pragma pack(pop)


#pragma pack(push,1)
struct Block {
	uint8_t shift_filter;
	uint8_t flags;

	Four_bit Samples[28];
};
#pragma pack(pop)


#pragma pack(push,1)
struct WL
{
	uint8_t magic;
	uint16_t block_count;
	uint16_t sample_rate;

	Block* data;

};
#pragma pack(pop)