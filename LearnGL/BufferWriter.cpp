#include "BufferWriter.hpp"

void RPF_Buffer(RPF& in) {

	std::vector<uint8_t> SerialisedImage;

	uint8_t CLUT_index;

	for (size_t i = 0; i < ((((size_t)in.magic[2]) + 1) * (((size_t)in.magic[3]) + 1)); i++)
	{
		CLUT_index = in.data[i];

		SerialisedImage.push_back(in.CLUT[CLUT_index].r);
		SerialisedImage.push_back(in.CLUT[CLUT_index].g);
		SerialisedImage.push_back(in.CLUT[CLUT_index].b);
		SerialisedImage.push_back(in.CLUT[CLUT_index].a);

	}



	std::ofstream outStream;

	outStream.open("Buffers/Image.bfr", std::ios::binary);

	outStream.write(reinterpret_cast<char*>(SerialisedImage.data()), (sizeof(uint8_t) * SerialisedImage.size()));


	outStream.close();


	SerialisedImage.clear();

}

void WL_Buffer(int16_t* audio, size_t audio_size) {

	std::ofstream outStream;

	outStream.open("Buffers/Sound.bfr", std::ios::binary);

	outStream.write(reinterpret_cast<char*>(audio), audio_size);


	outStream.close();


}

