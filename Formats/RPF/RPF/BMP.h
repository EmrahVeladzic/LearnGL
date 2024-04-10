#ifndef BMP_HEADER
#define BMP_HEADER





#include <stdint.h>


#pragma pack(push, 4) // Ensure structure packing
typedef struct bmp {

	uint16_t magic;

	uint32_t file_size;
	uint32_t reserved;
	uint32_t data_offset;

	uint32_t header_size;
	int32_t width;
	int32_t height;

	uint16_t planes;
	uint16_t bpp;

	uint32_t compression;
	uint32_t img_size;

	int32_t xpixelperm;
	int32_t ypixelperm;


	uint32_t colors_used;
	uint32_t important_colors;


}BMP;
#pragma pack(pop) // Ensure structure packing

#endif // !BMP_HEADER