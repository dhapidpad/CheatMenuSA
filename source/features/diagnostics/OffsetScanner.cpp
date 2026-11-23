#include "OffsetScanner.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <iomanip>

std::vector<OffsetInfo> OffsetScanner::s_results;

uintptr_t OffsetScanner::FindPattern(const char* moduleName, const char* pattern, const char* mask)
{
	HMODULE hMod = moduleName ? GetModuleHandleA(moduleName) : GetModuleHandleA(nullptr);
	if (!hMod) return 0;

	auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(hMod);
	if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;

	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(hMod) + dos->e_lfanew);
	if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

	uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
	size_t size = nt->OptionalHeader.SizeOfImage;
	size_t patternLen = strlen(mask);

	for (size_t i = 0; i < size - patternLen; ++i)
	{
		bool match = true;
		for (size_t j = 0; j < patternLen; ++j)
		{
			if (mask[j] != '?' && pattern[j] != *reinterpret_cast<const char*>(base + i + j))
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			return base + i;
		}
	}

	return 0;
}

uintptr_t OffsetScanner::FindPatternIDA(const char* moduleName, const char* idaPattern)
{
	std::string patternBytes;
	std::string mask;

	std::stringstream ss(idaPattern);
	std::string byteStr;

	while (ss >> byteStr)
	{
		if (byteStr == "?" || byteStr == "??")
		{
			patternBytes.push_back('\x00');
			mask.push_back('?');
		}
		else
		{
			auto b = static_cast<char>(strtoul(byteStr.c_str(), nullptr, 16));
			patternBytes.push_back(b);
			mask.push_back('x');
		}
	}

	return FindPattern(moduleName, patternBytes.c_str(), mask.c_str());
}

void OffsetScanner::RegisterOffset(const std::string& name, uintptr_t addr, const std::string& valStr, const std::string& desc, bool found)
{
	char hexBuf[32];
	snprintf(hexBuf, sizeof(hexBuf), "0x%08X", static_cast<unsigned int>(addr));

	s_results.push_back({ name, addr, std::string(hexBuf), valStr, desc, found });

	if (found)
		Logger::Offset("[Scanner] %s -> %s (%s)", name.c_str(), hexBuf, valStr.c_str());
	else
		Logger::Warn("[Scanner] %s not resolved", name.c_str());
}

void OffsetScanner::ScanAllCoreOffsets()
{
	s_results.clear();
	Logger::Info("Starting Core Memory & Structure Offset Scan...");

	// 1. Local Player Ped
	CPed* local = FindPlayerPed(0);
	if (local)
	{
		char val[64];
		snprintf(val, sizeof(val), "HP: %.1f | Armor: %.1f", local->m_fHealth, local->m_fArmour);
		RegisterOffset("FindPlayerPed(0)", reinterpret_cast<uintptr_t>(local), val, "Local Player Object Pointer");
	}
	else
	{
		RegisterOffset("FindPlayerPed(0)", 0, "Null", "Local Player Object Pointer", false);
	}

	// 2. Direct3D 9 Device
	__try {
		auto ppDevice = reinterpret_cast<IDirect3DDevice9**>(0xC97C20);
		if (ppDevice && *ppDevice)
		{
			char val[64];
			snprintf(val, sizeof(val), "Device* = 0x%08X", static_cast<unsigned int>(reinterpret_cast<uintptr_t>(*ppDevice)));
			RegisterOffset("pDirect3DDevice9", 0xC97C20, val, "Pointer to IDirect3DDevice9");
		}
		else
		{
			RegisterOffset("pDirect3DDevice9", 0xC97C20, "Null", "Pointer to IDirect3DDevice9", false);
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("pDirect3DDevice9", 0xC97C20, "Access Error", "Pointer to IDirect3DDevice9", false);
	}

	// 3. CPool<CPlayerPed>
	__try {
		auto pPool = *reinterpret_cast<uintptr_t*>(0xB6F5F0);
		char val[64];
		snprintf(val, sizeof(val), "Pool* = 0x%08X", static_cast<unsigned int>(pPool));
		RegisterOffset("CPool<CPlayerPed>", 0xB6F5F0, val, "Player Ped Pool Pointer");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CPool<CPlayerPed>", 0xB6F5F0, "Error", "Player Ped Pool Pointer", false);
	}

	// 4. CPool<CVehicle>
	__try {
		auto pPool = *reinterpret_cast<uintptr_t*>(0xBA18FC);
		char val[64];
		snprintf(val, sizeof(val), "Pool* = 0x%08X", static_cast<unsigned int>(pPool));
		RegisterOffset("CPool<CVehicle>", 0xBA18FC, val, "Vehicle Pool Pointer");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CPool<CVehicle>", 0xBA18FC, "Error", "Vehicle Pool Pointer", false);
	}

	// 5. CPool<CPed>
	__try {
		auto pPool = *reinterpret_cast<uintptr_t*>(0xB74490);
		char val[64];
		snprintf(val, sizeof(val), "Pool* = 0x%08X", static_cast<unsigned int>(pPool));
		RegisterOffset("CPool<CPed>", 0xB74490, val, "Ped Pool Pointer");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CPool<CPed>", 0xB74490, "Error", "Ped Pool Pointer", false);
	}

	// 6. Camera Instance
	RegisterOffset("TheCamera (CCamera)", 0xB6F028, "Static Struct", "Global Camera Instance");
	RegisterOffset("TheCamera.m_Matrix", 0xB6F290, "RwMatrix", "Active Camera World Matrix");

	// 7. Graphics & Display Engine Offsets
	__try {
		float fov = *reinterpret_cast<float*>(0x8D5038);
		char val[64];
		snprintf(val, sizeof(val), "FOV = %.1f deg", fov);
		RegisterOffset("CDraw::ms_fFOV", 0x8D5038, val, "Camera Field of View");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CDraw::ms_fFOV", 0x8D5038, "Error", "Camera Field of View", false);
	}

	__try {
		float farClip = *reinterpret_cast<float*>(0xB7C4F0);
		char val[64];
		snprintf(val, sizeof(val), "FarClip = %.1f m", farClip);
		RegisterOffset("CDraw::ms_fFarClip", 0xB7C4F0, val, "Maximum Draw Distance");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CDraw::ms_fFarClip", 0xB7C4F0, "Error", "Maximum Draw Distance", false);
	}

	__try {
		float aspect = *reinterpret_cast<float*>(0xC3EFA4);
		char val[64];
		snprintf(val, sizeof(val), "Aspect = %.4f", aspect);
		RegisterOffset("CDraw::ms_fAspectRatio", 0xC3EFA4, val, "Engine Aspect Ratio");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CDraw::ms_fAspectRatio", 0xC3EFA4, "Error", "Engine Aspect Ratio", false);
	}

	// 8. Game Timer & Timescale
	__try {
		unsigned int timeMs = *reinterpret_cast<unsigned int*>(0xB7CB84);
		float timeScale = *reinterpret_cast<float*>(0xB7CB64);
		char val[64];
		snprintf(val, sizeof(val), "Time = %u ms | Scale = %.2f", timeMs, timeScale);
		RegisterOffset("CTimer::m_snTimeInMilliseconds", 0xB7CB84, val, "Engine Internal Tick Counter");
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RegisterOffset("CTimer::m_snTimeInMilliseconds", 0xB7CB84, "Error", "Engine Internal Tick Counter", false);
	}

	// 9. CWeaponInfo Array
	RegisterOffset("CWeaponInfo::aWeaponInfo", 0xC8AAB8, "Array (80 items)", "Weapon Data & Weapon Definitions");

	// 10. SA-MP Module & Base
	HMODULE hSamp = GetModuleHandleA("samp.dll");
	if (hSamp)
	{
		uintptr_t sampBase = reinterpret_cast<uintptr_t>(hSamp);
		char val[64];
		snprintf(val, sizeof(val), "Base = 0x%08X", static_cast<unsigned int>(sampBase));
		RegisterOffset("samp.dll Base", sampBase, val, "SA-MP Client Module Handle");

		// Known SAMP Info pointer offset (0x21A0F8 in 0.3.7-R1)
		__try {
			uintptr_t pSampInfo = *reinterpret_cast<uintptr_t*>(sampBase + 0x21A0F8);
			if (pSampInfo)
			{
				snprintf(val, sizeof(val), "stSamp* = 0x%08X", static_cast<unsigned int>(pSampInfo));
				RegisterOffset("SAMP stSamp", sampBase + 0x21A0F8, val, "SA-MP Main Context (0.3.7 R1)");
			}
		} __except(EXCEPTION_EXECUTE_HANDLER) {}
	}
	else
	{
		RegisterOffset("samp.dll", 0, "Not Loaded (Singleplayer / Offline Mode)", "SA-MP Client Module", false);
	}

	Logger::Info("Scan completed! Found %u offsets.", static_cast<unsigned int>(s_results.size()));
}

const std::vector<OffsetInfo>& OffsetScanner::GetResults()
{
	return s_results;
}

std::string OffsetScanner::DumpToJson()
{
	std::stringstream ss;
	ss << "{\n  \"offsets\": [\n";
	for (size_t i = 0; i < s_results.size(); ++i)
	{
		const auto& o = s_results[i];
		ss << "    {\n";
		ss << "      \"name\": \"" << o.name << "\",\n";
		ss << "      \"address\": \"" << o.addressHex << "\",\n";
		ss << "      \"value\": \"" << o.valueStr << "\",\n";
		ss << "      \"description\": \"" << o.description << "\",\n";
		ss << "      \"found\": " << (o.found ? "true" : "false") << "\n";
		ss << "    }" << (i + 1 < s_results.size() ? ",\n" : "\n");
	}
	ss << "  ]\n}";
	return ss.str();
}

void OffsetScanner::ExportToFile(const std::string& filepath)
{
	std::ofstream ofs(filepath);
	if (!ofs.is_open()) return;

	ofs << "=== CheatMenuSA Scanned Offsets Dump ===" << std::endl;
	for (const auto& o : s_results)
	{
		ofs << std::left << std::setw(30) << o.name
		    << " | " << std::setw(12) << o.addressHex
		    << " | " << std::setw(25) << o.valueStr
		    << " | " << o.description << std::endl;
	}
	Logger::Info("Exported %u offsets to %s", static_cast<unsigned int>(s_results.size()), filepath.c_str());
}
