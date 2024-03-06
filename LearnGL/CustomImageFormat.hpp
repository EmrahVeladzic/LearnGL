#ifndef RPF_FORMAT
#define RPF_FORMAT
#include <stdint.h>





#pragma pack(push,4)
struct Pixel
{	
	uint16_t a : 1;//Simple alpha clip bit
	uint16_t r : 5;//Red channel
	uint16_t g : 5;//Green Channel
	uint16_t b : 5;//Blue channel
	
};
#pragma pack(pop)

#pragma pack(push,4)
struct Pixel32
{
	

	uint8_t r;

	uint8_t g;
	
	uint8_t b;
		
	uint8_t a;
	
	
	
};
#pragma pack(pop)

#pragma pack(push,4)
struct RPF
{
	uint8_t magic[4];// Byte 0 = "R" , Byte 1 = CLUT size, Byte 2 = width, Byte 3 = height

	Pixel32* CLUT;//Color lookup table

	uint8_t* data;//Pixel references to the CLUT
};
#pragma pack(pop)




#endif // !RPF_FORMAT