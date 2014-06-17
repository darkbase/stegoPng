#pragma once

unsigned int calculate_capacity(int w, int h, int bits_per_byte);
bool get_file_capacity(const char *szFileName, int bits_per_byte, unsigned int *capacity, unsigned int *size);
bool hide_message(const char* pngfilename, int bits_per_byte, const char* outputname, const char* data, unsigned int size);
bool extract_message(const char* pngfilename, std::string& out);
