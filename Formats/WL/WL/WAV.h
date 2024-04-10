#ifndef WAV_HEADER
#define WAV_HEADER




#include <stdint.h>

#pragma pack(push,1)
typedef struct uncompressed_audio {

	uint8_t magic[4];
	uint32_t file_size;
	uint8_t file_type[4];
	uint8_t fmt_marker[4];
	uint32_t fmt_size;
	uint16_t format;
	uint16_t num_of_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;//rate * bps * channel/8
	uint16_t block_align;//channel * bps/8
	uint16_t bits_per_sample;




	uint8_t data_marker[4];
	uint32_t data_section_size;

}WAV;
#pragma pack (pop)

#pragma pack(push,1)
typedef struct wav_sample {

	int16_t data;

} WAV16;
#pragma pack (pop)

#endif // !WAV