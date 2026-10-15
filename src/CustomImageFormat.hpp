#ifndef RPF_FORMAT
#define RPF_FORMAT
#include <stdint.h>

#pragma pack(push,4)
struct RPF
{
	uint8_t magic[4];

	uint16_t* CLUT;

	uint8_t* data;
};
#pragma pack(pop)




#endif // !RPF_FORMAT