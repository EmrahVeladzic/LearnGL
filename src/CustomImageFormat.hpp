#ifndef RPF_FORMAT
#define RPF_FORMAT
#include <stdint.h>

#pragma pack(push,4)
struct RPF
{
	uint8_t magic[4];// Byte 0 = "R" , Byte 1 = CLUT size, Byte 2 = width, Byte 3 = height

	uint16_t* CLUT;//Color lookup table

	uint8_t* data;//Pixel references to the CLUT
};
#pragma pack(pop)




#endif // !RPF_FORMAT