#ifndef WL_FORMAT
#define WL_FORMAT
#include <stdint.h>


#define DIVISOR 5



#define MAX_INT_4 7
#define MIN_INT_4 -8


#pragma pack(push,1)
struct Four_bit {

	uint8_t value : 4;

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
	uint32_t block_count;
	uint16_t sample_rate;

	Block* data;

};
#pragma pack(pop)


#endif // !WL_FORMAT