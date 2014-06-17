
/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    by yakman
*/

#include "stdafx.h"
#include "lodepng.h"


unsigned LodePNG_read32bitInt(const unsigned char* buffer)
{
	return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}


bool parse_hdr(unsigned int *w, unsigned int *h, FILE *pFile)
{
	unsigned char in[24];

	size_t n = fread(in, 1, 24, pFile);

	if(n != 24)
	{
		printf("png file too small\n");
		return false;
	}

	if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) 
	{ 
		printf("error: the first 8 bytes are not the correct PNG signature\n");
		return false; 
	}

	if(in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R') 
	{
		printf("error: png file doesn't start with a IHDR chunk\n");
		return false; 
	} 

	/*read the values given in the header*/
	*w = LodePNG_read32bitInt(&in[16]);
	*h = LodePNG_read32bitInt(&in[20]);

	return true;
}


unsigned int calculate_capacity(int w, int h, int bits_per_byte) {

	switch(bits_per_byte) {
	case 1:	return (w*h*4)/8 - 33;
	case 2:	return (w*h*4)/4 - 17;
	case 4:	return (w*h*4)/2 - 9;
	default: {
		printf("invalid bits_per_byte: %d\n", bits_per_byte);
			 }
	}

	return 0;
}


bool get_file_capacity(const char *szFileName, int bits_per_byte, unsigned int *capacity, unsigned int *size)
{
	FILE *pFile = fopen(szFileName, "rb");

	if(pFile == NULL)
	{
		printf("fopen() failed\n");
		return false;
	}

	fseek(pFile, 0L, SEEK_END);
	*size = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);

	unsigned int w, h;

	bool bResult = parse_hdr(&w, &h, pFile);

	if(bResult) 
	{
		*capacity = calculate_capacity(w, h, bits_per_byte);
	}
	else
	{
		printf("parse_hdr() failed\n");
	}

	fclose(pFile);
	return bResult;
}


bool hide_message(const char* pngfilename, int bits_per_byte, const char* outputname, const char* data, unsigned int size) {

	if(bits_per_byte != 1 && bits_per_byte != 2 && bits_per_byte != 4) {
		printf("only does bits_per_byte = 1, 2 or 4. not %d\n", bits_per_byte);
		return false;
	}

	unsigned int capacity, file_size;
	bool bResult = get_file_capacity(pngfilename, bits_per_byte, &capacity, &file_size);

	if(!bResult) {
		return false;
	}

	if(size > capacity) {
		printf("message too big: %u\n", size);
		return false;
	}

	unsigned int error;
	unsigned char* image = NULL;
	unsigned int w, h;

	error = LodePNG_decode32f(&image, &w, &h, pngfilename);
	if(error) {
		printf("LodePNG_decode32f() failed: 0x%X\n", error);
		free(image);
		return false;
	}

/**
 * 
 * Format
 * First byte, at 0,0 uses its two low bits to store how many bits will be used for the rest of the file.
 *  must have 1 added, the number of bits can be in the range 1-4, zero isnt included, 3 will probably not be used
 * 
 * When i say 'next pixel', i mean iterating along the images width, then down its height. Like reading latin
 * writing. Helpfully, the lodepng library gives you a memory buffer where the pixels are arranged like this.
 * the low bits of all 4 components are used, a, r, g, b
 * 
 * The next few pixels will be read and interpreted until there are enough bits to make a 32-bit integer, this
 * is the size of the hidden file. This integer is in network byte order, so htonl() can be used
 * 
 * The next pixels will be read until enough bytes are parsed to make reveal the file.
 * 
 */

	unsigned int f, g;
	unsigned char* p = image;
	int bit_masks[] = {0x01, 0x03, 0, 0x0f};
	//referenced like bit_masks[bits_per_byte-1]

	*p &= ~0x03;
	*p |= (bits_per_byte - 1) & 0x03;
	p++;

	int ml_net = htonl(size);
	for(f = 0; f < 32; f += bits_per_byte) {
		char c = *p;

		*p &= ~bit_masks[bits_per_byte-1];
		*p |= (ml_net >> f) & bit_masks[bits_per_byte-1];
		p++;

	}

	for(f = 0; f < size; f++) { 
		for(g = 0; g < 8; g += bits_per_byte) {
			*p &= ~bit_masks[bits_per_byte-1];
			*p |= (data[f] >> g) & bit_masks[bits_per_byte-1];
			p++;
		}
	}


	unsigned char* buffer = NULL;
	size_t buffersize;
	LodePNG_Encoder encoder;

	LodePNG_Encoder_init(&encoder);
	encoder.settings.zlibsettings.windowSize = 2048;
	LodePNG_encode(&encoder, &buffer, &buffersize, image, w, h);

	FILE* out_fd = fopen(outputname, "wb");

	if(out_fd == NULL)
	{
		printf("Error opening output pic\n");
		free(buffer);
		free(image);
		return false;
	}

	fwrite(buffer, 1, buffersize, out_fd);
	fclose(out_fd);
  

	LodePNG_Encoder_cleanup(&encoder);
	free(buffer);
	free(image);

	return true;
}

bool extract_message(const char* pngfilename, std::string& out) {

	unsigned int error;
	unsigned char* image = NULL;
	unsigned int w, h;

	error = LodePNG_decode32f(&image, &w, &h, pngfilename);
	if(error) {
		printf("LodePNG_decode32f() failed: 0x%X\n", error);
		free(image);
		return false;
	}

	if(w*h < 5) {
		printf("image too small to possibly hold anything\n");
		free(image);
		return false;
	}


	int bit_masks[] = {0x01, 0x03, 0, 0x0f};
	//referenced like bit_masks[bits_per_byte-1]

	unsigned char* p = image;
	unsigned int f, g;

	int bits_per_byte = (*(p++) & 0x03) + 1;
	if(bits_per_byte != 1 && bits_per_byte != 2 && bits_per_byte != 4) {
		printf("invalid bits_per_byte: %d\n", bits_per_byte);
		free(image);
		return false;
	}

	int ml_net = 0;
	for(f = 0; f < 32; f += bits_per_byte) {
		ml_net |= (*(p++) & bit_masks[bits_per_byte-1]) << f;
	}

	unsigned int message_length = ntohl(ml_net);
	if(message_length > calculate_capacity(w, h, bits_per_byte)) {
		printf("message too big: %u\n", message_length);
		free(image);
		return false;
	}


	char* message = (char*) malloc(message_length);
	for(f = 0; f < message_length; f++) {
		message[f] = 0;
		for(g = 0; g < 8; g += bits_per_byte) {
			message[f] |= (*(p++) & bit_masks[bits_per_byte-1]) << g;
		}
	}

	out.assign(message, message_length);

	free(image);
	free(message);

	return true;
}
