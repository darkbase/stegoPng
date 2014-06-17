#include "stdafx.h"
#define BITS_PER_BYTE	4

bool HideData(std::string& inStr, std::string& sFileName, int bits_per_byte, std::string& sTmpFileName)
{
	return hide_message(sFileName.c_str(), bits_per_byte, sTmpFileName.c_str(), inStr.c_str(), inStr.size());
}

bool PutMsg(std::string& sMsg, std::string& sFileName, int bits_per_byte)
{
	std::string sTmpFileName;
	bool bResult = false;

	std::stringstream ss;
	ss << sFileName << "_" << std::hex << rand() << rand();
	sTmpFileName = ss.str();

	bResult = HideData(sMsg, sFileName, bits_per_byte, sTmpFileName);

	if (!bResult)
		return false;

	return true;
}

bool CheckPic(size_t msg_size, std::string& sPicName)
{
	unsigned int capacity, size;
	bool bResult = get_file_capacity(sPicName.c_str(), BITS_PER_BYTE, &capacity, &size);

	if (bResult && capacity >= msg_size)
		return true;
	return false;
}

bool ExtractData(std::string& outStr, std::string& sFileName)
{
	return extract_message(sFileName.c_str(), outStr);
}

bool GetMsg(std::string& sFileName, std::string& outStr)
{
	bool bResult = false;
	bResult = ExtractData(outStr, sFileName);
	if (!bResult)
		return false;
	return true;
}

int main(int argc, char* argv[])
{
	std::string sendMsg = "dddaf9c5c2ba90f9d971a7392446575b";
	std::string sPicName = "path_to_png";
	if (!CheckPic(sendMsg.size(), sPicName)) {
		printf("wrong pic\n");
		return 1;
	}
	if (!PutMsg(sendMsg, sPicName, BITS_PER_BYTE)) {
		printf("somethin wrong while hiding msg\n");
		return 2;
	}
	/*
	std::string recvMsg;
	std::string sPicName = "path_to_png";
	if (!GetMsg(sPicName, recvMsg)) {
	printf("somethin wrong while unhiding msg\n");
	return 3;
	}	*/
	return 0;
}

