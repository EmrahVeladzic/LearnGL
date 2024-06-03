#ifndef BUFFER_WRITER
#define BUFFER_WRITER

#include "CustomImageFormat.hpp"
#include "CustomSoundFormat.hpp"
#include "ModelLoader.hpp"
#include <fstream>

void RPF_Buffer(RPF& in);

void WL_Buffer(int16_t* audio, size_t audio_size);


#endif // !BUFFER_WRITER


