#ifndef RPF_FORMAT
#define RPF_FORMAT
#include <stdint.h>

#pragma pack(push,4)
struct RPF
{
	uint8_t magic;

	uint8_t width;
	uint8_t height;

	uint8_t plt_size;

	uint16_t* CLUT;

	uint8_t* data;
};
#pragma pack(pop)




#endif // !RPF_FORMAT