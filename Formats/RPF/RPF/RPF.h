#ifndef RPF_HEADER
#define RPF_HEADER





#include <stdint.h>
#include <stdlib.h>
#include <math.h>

size_t SCALEX;
size_t SCALEY;

#define DEPTH 256

#define PROTECTED_BUFFER_SIZE 8

size_t ProtectedBufferAccess;

uint8_t* mode_in;


typedef struct bit
{
	unsigned int value : 1;
}BIT;





//24-bit Pixel value.
typedef struct px {


	uint8_t r;//red

	uint8_t g;//green

	uint8_t b;//blue

}Pixel;



//15 bit value (16-bits, the 1st being alpha);

typedef struct px16 {

	uint16_t a : 1;
	uint16_t r : 5;
	uint16_t g : 5;
	uint16_t b : 5;

}Pixel15;


Pixel15* ProtectedBuffer;



typedef struct rpf {



	uint8_t magic[4];// - magic byte value - Will == P"#%% - "#" being the 8bit value which corresponds to CLUT size
	// -% corresponds to SCALE

	Pixel15* CLUT;//Color lookup table - [0] will be 31,0,31- and will be used as transparency

	uint8_t* data;//max 256 values of color

	

}RPF;



typedef struct oe {
	Pixel15 Value; //Pixel value - specifies which color 
	uint32_t Occurence; //Times the above color was found in the image

}Occurence_Entry;
//When looping, if a Pixel is not found, it will create a new entry. Otherwise it will increment the Occurence

typedef struct se {
	Pixel15 Donor;//This pixel replaces the Recipient
	Pixel15 Recipient;//This pixel is the one replaced

}Swap_Entry;
//In the later part of the code, each pixel value will be compared and replaced with the donor. 
//When replacing, if Recipient is itself a donor in some entries, the current Donor will override the donor in tose entries


typedef struct com {

	RPF Export;//Final Image File

	Pixel* LARGE_CLUT;//WIll store as many idividual values as it finds. Not resized until end of iteration, if ever
	Pixel15* LARGE_CLUT15;


	
	Pixel15* RESIZED_CLUT;//Will store a dynamic amount of pixels

	uint32_t UNIQUE_PIXEL_COUNT;//Number detailing unique pixels found. Iterations stop when it falls below DEPTH
	uint32_t MAX_PIXEL_COUNT;//Meant to resolve a edge case where the input has fewer unique colours than DEPTH
	Occurence_Entry* Occurence_Table;//Internal array of occurence entries 

	uint32_t SwapCount;
	Swap_Entry* Swap_Table;//Internal array of swap entries


	Pixel ALPHA;//255 0 255 - Must be index 0 in new CLUT
	Pixel15 ALPHA15;//ALPHA scaled down to 16 bits



}RPF_Compressor;

RPF NewRPF;

RPF_Compressor Compressor;

;

int Compress();
//A 3D vector. Will be used to calculate most similar colors
typedef struct vec3
{
	double b;

	double g;

	double r;
}PIX3D;

double vec_dist(PIX3D a, PIX3D b);
int BMPimport();

char* string_in();

char* fileName;

char* fileexnm;

int Export();


BIT Compare(Pixel a, Pixel b);

Pixel15 Convert_to_15Bit(Pixel in);

BIT chk;

BIT flow;

BIT method;

PIX3D potential_donor;
PIX3D potential_recipient;

#endif // !RPF_HEADER