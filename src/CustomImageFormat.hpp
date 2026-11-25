#ifndef RPF_FORMAT
#define RPF_FORMAT
#include <stdint.h>

#pragma pack(push,4)
struct RPF
{
	uint8_t magic;

	uint8_t anim_data;

	uint16_t width;
	uint16_t height;

	uint16_t plt_size;

	uint16_t* CLUT;

	uint8_t* data;

	bool anim;

	uint8_t swap_frame;

	uint8_t frames;

	uint8_t current_frame;
};
#pragma pack(pop)




#endif // !RPF_FORMAT