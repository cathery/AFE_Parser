#include "panic.h"
#include "AFE.h"
#include "inttypes.h"
#include <cstdio>
#include <fstream>

int main(int argc, char* argv[])
{
	const char* filePath = argv[1];
	if (argc < 2)
	{
		printf("Please put the path to the fatal report .bin file as the first parameter to this program\n");
		return -1;
	}

	std::fstream file;
	file.open(filePath, std::fstream::in | std::fstream::binary);

	//Get file magic
	uint32_t fatal_magic;
	file.read((char*)&fatal_magic, sizeof(fatal_magic));
	file.seekp(std::ios_base::beg);

	//AFE2
	if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC)
	{
		atmosphere_fatal_error_ctx fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE2Report(&fatal_report);
	}
	//AFE1
	else if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_1)
	{
		atmosphere_fatal_error_ctx_1 fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE1Report(&fatal_report);
	}
	//AFE0
	else if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_0)
	{
		atmosphere_fatal_error_ctx_0 fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE0Report(&fatal_report);
	}
	else if ((fatal_magic & 0xF0FFFFFF) == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_0)
	{
		printf("Passed file contains an unknown AFE version:  %.4s (0x%" PRIX32 ")\n", (char*)&fatal_magic, fatal_magic);
	}
	else
	{
		printf("Passed file is not a valid AFE file\n");
	}
	file.close();
}