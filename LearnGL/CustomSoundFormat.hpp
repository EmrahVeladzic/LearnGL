#ifndef WL_FORMAT
#define WL_FORMAT
#include <stdint.h>


#define MAX_INT_4 7
#define MIN_INT_4 -8
#define SAMPLES_PER_BLOCK 28


#pragma pack(push,1)
struct WL_sample {

	uint8_t value : 4;

};
#pragma pack(pop)


#pragma pack(push,1)
struct Block {
	uint8_t shift_filter;
	uint8_t flags;

	WL_sample Samples[SAMPLES_PER_BLOCK];
};
#pragma pack(pop)


#pragma pack(push,1)
struct WL
{
	uint8_t magic;
	uint8_t num_of_channels;
	uint8_t clamp_bits;
	uint32_t block_count;
	uint16_t sample_rate;

	Block* data;

};
#pragma pack(pop)


#endif // !WL_FORMAT