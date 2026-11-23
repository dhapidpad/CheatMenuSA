#pragma once

#include "Globals.h"
#include <string>
#include <vector>

struct OffsetInfo
{
	std::string name;
	uintptr_t   address;
	std::string addressHex;
	std::string valueStr;
	std::string description;
	bool        found;
};

class OffsetScanner
{
public:
	static uintptr_t FindPattern(const char* moduleName, const char* pattern, const char* mask);
	static uintptr_t FindPatternIDA(const char* moduleName, const char* idaPattern);

	static void ScanAllCoreOffsets();
	static const std::vector<OffsetInfo>& GetResults();
	static std::string DumpToJson();
	static void ExportToFile(const std::string& filepath = "offsets_dump.txt");

private:
	static std::vector<OffsetInfo> s_results;
	static void RegisterOffset(const std::string& name, uintptr_t addr, const std::string& valStr, const std::string& desc, bool found = true);
};
