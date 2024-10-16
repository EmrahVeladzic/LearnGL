#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include"RPF.h"

double vec_dist(PIX3D a, PIX3D b) {


	return sqrt(pow((a.r - b.r), 2) + pow((a.g - b.g), 2) + pow((a.b - b.b), 2));

}


BIT Compare(Pixel a, Pixel b) {
	BIT answer;

	answer.value = (a.r == b.r && a.g == b.g && a.b == b.b) ? 1 : 0;

	return answer;

}

BIT Compare15(Pixel15 a, Pixel15 b) {
	BIT answer;

	answer.value = (a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b) ? 1 : 0;


	return answer;

}

Pixel15 GetHue(Pixel15 in) {
	Pixel15 out;
	

	if (in.r >= 16) {
		out.r = 31;
	}
	else
	{
		out.r = 0;
	}
	if (in.g >= 16) {
		out.g = 31;
	}
	else
	{
		out.g = 0;
	}
	if (in.b >= 16) {
		out.b = 31;
	}
	else
	{
		out.b = 0;
	}



	return out;
}

PIX3D GetHueAsVector(Pixel15 in) {
	PIX3D out;


	if (in.r >= 16) {
		out.r = 31.0f;
	}
	else
	{
		out.r = 0.0f;
	}
	if (in.g >= 16) {
		out.g = 31.0f;
	}
	else
	{
		out.g = 0.0f;
	}
	if (in.b >= 16) {
		out.b = 31.0f;
	}
	else
	{
		out.b = 0.0f;
	}



	return out;
}

BIT CheckHue(Pixel15 a, Pixel15 b) {
	BIT out;
	out.value = 0;

	Pixel15 aNorm = GetHue(a);
	Pixel15 bNorm = GetHue(b);

	if ((chk = Compare15(aNorm, bNorm)).value == 1) {

		out.value = 1;

	}





	return out;
}


BIT Ensure_DUAL(Pixel15 a, Pixel15 b) {
	BIT out;
	out.value = 1;


	if ((chk = Compare15(a, Compressor.ALPHA15)).value == 0 && (chk = Compare15(b, Compressor.ALPHA15)).value == 0 && (chk = Compare15(b, a)).value == 0 && (chk=CheckHue(a,b)).value==1) {
		out.value=1;


		if (PROTECTED_BUFFER_SIZE <= 0) {

			return out;
		}

		else
		{
			for (size_t i = 0; i < PROTECTED_BUFFER_SIZE; i++)
			{
				if ((chk = Compare15(a, ProtectedBuffer[i])).value == 1 || (chk = Compare15(b, ProtectedBuffer[i])).value == 1) {

					out.value = 0;
					break;
				}


			}
		}

		

	}

	else
	{
		out.value = 0;
	}


	

	return out;
}


BIT EnsureSINGLE(Pixel15 a, Pixel15 b) {
	BIT out;
	out.value = 1;


	if ((chk = Compare15(a, Compressor.ALPHA15)).value == 0 && (chk = Compare15(b, Compressor.ALPHA15)).value == 0 && (chk = Compare15(b, a)).value == 0) {
		out.value = 1;

	}

	else
	{
		out.value = 0;
	}

	return out;
}




int Protect(Pixel15 recentDonor) {

	if (PROTECTED_BUFFER_SIZE <= 0) {
		return 0;
	}

	size_t index = ProtectedBufferAccess % PROTECTED_BUFFER_SIZE;

	if (index < PROTECTED_BUFFER_SIZE) {

		ProtectedBuffer[index] = recentDonor;

		ProtectedBufferAccess++;
	}

	return 0;
}





int Resize_Occurence() {
	Occurence_Entry* temp = NULL;
	size_t newsize = sizeof(Occurence_Entry) * (Compressor.UNIQUE_PIXEL_COUNT + 1);



	temp = (Occurence_Entry*)realloc(Compressor.Occurence_Table, newsize);

	Compressor.Occurence_Table = temp;
	Compressor.UNIQUE_PIXEL_COUNT++;



	return 0;
}



int Resize_Swap() {
	Swap_Entry* temp = NULL;
	if (Compressor.SwapCount > 0) {

		size_t newsize = sizeof(Swap_Entry) * (Compressor.SwapCount + 1);
		temp = (Swap_Entry*)realloc(Compressor.Swap_Table, newsize);
		Compressor.Swap_Table = temp;


	}
	else
	{
		Compressor.Swap_Table = (Swap_Entry*)malloc(sizeof(Swap_Entry));
	}

	Compressor.SwapCount++;
	return 0;
}







int SwitchOccurenceEntry(int index_a, int index_b) {
	Pixel15 TempVal;
	int TempOcc;


	TempVal = Compressor.Occurence_Table[index_a].Value;
	TempOcc = Compressor.Occurence_Table[index_a].Occurence;

	Compressor.Occurence_Table[index_a].Value = Compressor.Occurence_Table[index_b].Value;
	Compressor.Occurence_Table[index_a].Occurence = Compressor.Occurence_Table[index_b].Occurence;

	Compressor.Occurence_Table[index_b].Value = TempVal;
	Compressor.Occurence_Table[index_b].Occurence = TempOcc;

	return 0;
}


int sortOccurence() {

	BIT sorted;
	sorted.value = 0;

	while (sorted.value == 0)
	{
		sorted.value = 1;


		for (size_t i = 1; i < (size_t)(Compressor.MAX_PIXEL_COUNT); i++)
		{
			if (Compressor.Occurence_Table[i - 1].Occurence > Compressor.Occurence_Table[i].Occurence) {

				SwitchOccurenceEntry((int)(i - 1), (int)(i));
				sorted.value = 0;
				break;
			}

		}

		if (sorted.value == 1) {
			break;
		}
	}


	return 0;
}



int newColor(Pixel15 a) {
	Resize_Occurence();
	Compressor.Occurence_Table[Compressor.UNIQUE_PIXEL_COUNT - 1].Value = a;
	Compressor.Occurence_Table[Compressor.UNIQUE_PIXEL_COUNT - 1].Occurence = 1;
	return 0;
}

int overrideSwaps(Pixel15 a, Pixel15 b) {

	for (size_t i = 0; i < Compressor.SwapCount; i++)
	{
		if ((chk = Compare15(a, Compressor.Swap_Table[i].Donor)).value == 1) {

			Compressor.Swap_Table[i].Donor = b;

		}

	}

	return 0;
}

int createSwapEntry(Pixel15 a, Pixel15 b) {

	Resize_Swap(Compressor.SwapCount);
	Compressor.Swap_Table[Compressor.SwapCount - 1].Recipient = a;
	Compressor.Swap_Table[Compressor.SwapCount - 1].Donor = b;
	Compressor.UNIQUE_PIXEL_COUNT--;
	overrideSwaps(a, b);
	return 0;
}





int determine_best_swap(int input_index) {

	

	Pixel15 input = Compressor.Occurence_Table[input_index].Value;

	potential_recipient.r = (double)input.r;
	potential_recipient.g = (double)input.g;
	potential_recipient.b = (double)input.b;

	Pixel15 compare = Compressor.ALPHA15;

	size_t chosen_index = 0;

	double distance = INFINITY;
	double new_distance = distance;


	for (size_t i = Compressor.MAX_PIXEL_COUNT-Compressor.UNIQUE_PIXEL_COUNT; i < Compressor.MAX_PIXEL_COUNT; i++)
	{

		compare = Compressor.Occurence_Table[i].Value;

		potential_donor.r = (float)compare.r;
		potential_donor.g = (float)compare.g;
		potential_donor.b = (float)compare.b;

		new_distance = vec_dist(potential_donor, potential_recipient);

		if (new_distance < distance && (chk=EnsureSINGLE(input, compare)).value==1) {

			distance = new_distance;
			chosen_index = i;
		}

	}

	compare = Compressor.Occurence_Table[chosen_index].Value;

	
		
		Compressor.Occurence_Table[chosen_index].Occurence += Compressor.Occurence_Table[input_index].Occurence;
		Compressor.Occurence_Table[input_index].Occurence = 0;
		createSwapEntry(input, compare);
		
		
	


	return 0;
}


int determine_best_swap_with_profile() {


	Pixel15 input =Compressor.ALPHA15;

	potential_recipient.r = (double)input.r;
	potential_recipient.g = (double)input.g;
	potential_recipient.b = (double)input.b;

	Pixel15 compare = Compressor.ALPHA15;

	size_t chosen_compare = 0;
	size_t chosen_input = 0;


	double distance = INFINITY;
	double new_distance = distance;

	for (size_t r = 0; r < (PROTECTED_BUFFER_SIZE+1); r++)
	{

		if (r > 0) {
			Protect(Compressor.ALPHA15);
		}


		for (size_t i = Compressor.MAX_PIXEL_COUNT - Compressor.UNIQUE_PIXEL_COUNT; i < Compressor.MAX_PIXEL_COUNT; i++)
		{

			input = Compressor.Occurence_Table[i].Value;



			potential_recipient.r = (double)input.r;
			potential_recipient.g = (double)input.g;
			potential_recipient.b = (double)input.b;



			for (size_t j = Compressor.MAX_PIXEL_COUNT - Compressor.UNIQUE_PIXEL_COUNT; j < Compressor.MAX_PIXEL_COUNT; j++)
			{



				compare = Compressor.Occurence_Table[j].Value;

				potential_donor.r = (float)compare.r;
				potential_donor.g = (float)compare.g;
				potential_donor.b = (float)compare.b;

				new_distance = vec_dist(potential_donor, potential_recipient);

				


				if (r < PROTECTED_BUFFER_SIZE) {

					if (new_distance < distance && (chk = Ensure_DUAL(input, compare)).value == 1) {

						distance = new_distance;
						chosen_compare = j;
						chosen_input = i;
					}

				}

				else
				{
					if (new_distance < distance && (chk = EnsureSINGLE(input, compare)).value == 1) {

						distance = new_distance;
						chosen_compare = j;
						chosen_input = i;
					}
				}

			}

		}

		compare = Compressor.Occurence_Table[chosen_compare].Value;
		input = Compressor.Occurence_Table[chosen_input].Value;

		if ((chk = Ensure_DUAL(input, compare)).value == 1) {

			break;
		}

	}

	

	compare = Compressor.Occurence_Table[chosen_compare].Value;
	input = Compressor.Occurence_Table[chosen_input].Value;

	if ((chk = Ensure_DUAL(input, compare)).value == 1) {

		input.a = 1;
		compare.a = 1;


		PIX3D sat = GetHueAsVector(input);

		potential_recipient.r = (double)input.r;
		potential_recipient.g = (double)input.g;
		potential_recipient.b = (double)input.b;



		potential_donor.r = (double)compare.r;
		potential_donor.g = (double)compare.g;
		potential_donor.b = (double)compare.b;


		distance = vec_dist(sat, potential_recipient);
		new_distance = vec_dist(sat, potential_donor);


		if (new_distance < distance) {

			Compressor.Occurence_Table[chosen_compare].Occurence += Compressor.Occurence_Table[chosen_input].Occurence;
			Compressor.Occurence_Table[chosen_input].Occurence = 0;
			Protect(compare);
			createSwapEntry(input, compare);
			return 0;

		}



		else
		{


			Compressor.Occurence_Table[chosen_input].Occurence += Compressor.Occurence_Table[chosen_compare].Occurence;
			Compressor.Occurence_Table[chosen_compare].Occurence = 0;
			Protect(input);
			createSwapEntry(compare, input);


			return 0;
		}




	}
	
	else if ((chk = EnsureSINGLE(input,compare)).value==1){

		input.a = 1;
		compare.a = 1;


		if(Compressor.Occurence_Table[chosen_input].Occurence>=Compressor.Occurence_Table[chosen_compare].Occurence) {
		

			Compressor.Occurence_Table[chosen_input].Occurence += Compressor.Occurence_Table[chosen_compare].Occurence;
			Compressor.Occurence_Table[chosen_compare].Occurence = 0;
			Protect(input);
			createSwapEntry(compare, input);


			return 0;
		}


		else
		{
			Compressor.Occurence_Table[chosen_compare].Occurence += Compressor.Occurence_Table[chosen_input].Occurence;
			Compressor.Occurence_Table[chosen_input].Occurence = 0;
			Protect(compare);
			createSwapEntry(input, compare);
			return 0;

		}



	}

		return 0;
}


int Init() {

	printf("\nInitialising\n");
	Pixel15 new_p = Compressor.ALPHA15;
	BIT write_new;

	for (size_t i = 0; i < (SCALEX * SCALEY); i++)
	{
		new_p = Compressor.LARGE_CLUT15[i];
		write_new.value = 1;
		for (size_t j = 0; j < (size_t)Compressor.UNIQUE_PIXEL_COUNT; j++)
		{
			if ((chk = Compare15(new_p, Compressor.Occurence_Table[j].Value)).value == 1) {

				write_new.value = 0;
				Compressor.Occurence_Table[j].Occurence++;
			}


		}

		if (write_new.value == 1) {
			newColor(new_p);

		}
	}



	return 0;
}


uint8_t settle(Pixel15 in) {
	uint8_t tmp = 0;

	potential_recipient.r = (double)in.r;
	potential_recipient.g = (double)in.g;
	potential_recipient.b = (double)in.b;

	double distance = INFINITY;
	double new_distance = distance;

	Pixel15 compare;

	for (size_t i = 0; i < (size_t)NewRPF.magic[1]+1; i++)
	{
		compare = Compressor.RESIZED_CLUT[i];

		potential_donor.r = (double)compare.r;
		potential_donor.g = (double)compare.g;
		potential_donor.b = (double)compare.b;

		new_distance = vec_dist(potential_donor, potential_recipient);

		if (new_distance <= distance) {
			distance = new_distance;

			tmp = (uint8_t)i;

		}

	}



	return tmp;
}




uint8_t write_CLUT_to_data4bpp(int index) {
	uint8_t temp = 0;
	uint8_t msb = 0;
	uint8_t lsb = 0;

	Pixel15 true_valuemsb = Compressor.LARGE_CLUT15[(2 * index)];
	Pixel15 true_valuelsb = Compressor.LARGE_CLUT15[(2 * index) + 1];





	for (size_t i = 0; i < (size_t)Compressor.SwapCount; i++)
	{



		if ((chk = Compare15(true_valuemsb, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_valuemsb = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_valuemsb, Compressor.RESIZED_CLUT[j])).value == 1) {
					msb = (uint8_t)j;


				}
			}

		}

		else if ((chk = Compare15(true_valuemsb, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_valuemsb = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_valuelsb, Compressor.RESIZED_CLUT[j])).value == 1) {
					lsb = (uint8_t)j;


				}
			}


		}

		else {
			msb = settle(true_valuemsb);


			break;
		}


	}


	for (size_t i = 0; i < (size_t)Compressor.SwapCount; i++)
	{



		if ((chk = Compare15(true_valuelsb, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_valuelsb = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_valuelsb, Compressor.RESIZED_CLUT[j])).value == 1) {
					lsb = (uint8_t)j;


				}
			}

		}

		else if ((chk = Compare15(true_valuelsb, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_valuelsb = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_valuelsb, Compressor.RESIZED_CLUT[j])).value == 1) {
					lsb = (uint8_t)j;


				}
			}


		}

		else {
			lsb = settle(true_valuelsb);

			break;
		}


	}




	temp = ((msb & 0x0F) << 4) | (lsb & 0x0F);







	return temp;
}



uint8_t write_CLUT_to_data(int index) {
	uint8_t temp = 0;
	Pixel15 true_value = Compressor.LARGE_CLUT15[index];




	for (size_t i = 0; i < (size_t)Compressor.SwapCount; i++)
	{


		if ((chk = Compare15(true_value, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_value = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_value, Compressor.RESIZED_CLUT[j])).value == 1) {
					temp = (uint8_t)j;


				}
			}

		}

		else if ((chk = Compare15(true_value, Compressor.Swap_Table[i].Recipient)).value == 1) {


			true_value = Compressor.Swap_Table[i].Donor;


			for (size_t j = 0; j < (size_t)NewRPF.magic[1] + 1; j++)
			{
				if ((chk = Compare15(true_value, Compressor.RESIZED_CLUT[j])).value == 1) {
					temp = (uint8_t)j;


				}
			}


		}

		else {
			temp = settle(true_value);

			break;
		}

	}



	return temp;
}


Pixel15 Convert_to_15Bit(Pixel in) {
	Pixel15 temp;

	if ((chk = Compare(in, Compressor.ALPHA)).value == 1) {
		temp.a = 0;
		temp.r = 31;
		temp.g = 0;
		temp.b = 31;
	}

	else
	{
		temp.a = 1;
		temp.r = in.r >> 3;
		temp.g = in.g >> 3;
		temp.b = in.b >> 3;
	}

	if (temp.r == Compressor.ALPHA15.r && temp.g == Compressor.ALPHA15.g && temp.b == Compressor.ALPHA15.b) {
		temp.a = 0;
	}



	return temp;
}

int Commit() {
	printf("\nWriting to structure\n");

	

	NewRPF.magic[0] = (char)82;//R
	NewRPF.magic[1] = (Compressor.MAX_PIXEL_COUNT > DEPTH) ? (char)DEPTH - 1 : Compressor.MAX_PIXEL_COUNT - 1;
	NewRPF.magic[2] = (char)SCALEX - 1;
	NewRPF.magic[3] = (char)SCALEY - 1;




	for (size_t i = 0; i < (size_t)NewRPF.magic[1]+1; i++) {

		Compressor.RESIZED_CLUT[i] = Compressor.Occurence_Table[Compressor.MAX_PIXEL_COUNT-i-1].Value;
		
	}

	

	if ((NewRPF.magic[1] + 1) > 16) {

		for (size_t i = SCALEY; i > 0; i--)
		{
			
			for (size_t j = 0; j < SCALEX; j++)
			{

				NewRPF.data[((i-1)*SCALEX)+j] = write_CLUT_to_data((int)(((SCALEY-i) * SCALEX) + j));

			}

		}

	}

	else
	{
		for (size_t i = SCALEY; i > 0; i--)
		{

			for (size_t j = 0; j < SCALEX; j++)
			{

				NewRPF.data[(((i - 1) * SCALEX) + j)/2] = write_CLUT_to_data4bpp((int)((((SCALEY - i) * SCALEX) + j)/2));

			}

		}
	}

	NewRPF.CLUT = Compressor.RESIZED_CLUT;



	return 0;
}

int progress() {

	double prg = ((double)(Compressor.MAX_PIXEL_COUNT- Compressor.UNIQUE_PIXEL_COUNT)/(double)(Compressor.MAX_PIXEL_COUNT-DEPTH))*100.0f;

	printf("\r%d%%", (int)prg);

		return 0;
}

int Compress() {
	Init();



	

	Compressor.MAX_PIXEL_COUNT = Compressor.UNIQUE_PIXEL_COUNT;

	sortOccurence();

	printf("\nCompressing\n");

	if (Compressor.UNIQUE_PIXEL_COUNT > (uint32_t)DEPTH) {


		

		while (Compressor.UNIQUE_PIXEL_COUNT > DEPTH)
		{
			


			for (size_t i = 0; i < (size_t)(Compressor.MAX_PIXEL_COUNT); i++)
			{

				if (method.value == 0) {
					determine_best_swap((int)i);
				}

				else
				{
					determine_best_swap_with_profile();
				}

				
				

				sortOccurence();
				progress();



				if (Compressor.UNIQUE_PIXEL_COUNT <= DEPTH) {
					
					break;
				
				}

				

				
			}

			


		}

	}

	else
	{



		for (size_t i = 0; i < Compressor.MAX_PIXEL_COUNT; i++)
		{

			createSwapEntry(Compressor.Occurence_Table[i].Value, Compressor.Occurence_Table[i].Value);


		}



	}



	Commit();
	return 0;
}