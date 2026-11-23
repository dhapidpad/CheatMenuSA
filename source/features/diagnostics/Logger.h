#pragma once

#include "Globals.h"
#include <mutex>
#include <deque>
#include <string>
#include <vector>

enum class LogLevel
{
	INFO,
	WARN,
	ERR,
	OFFSET,
	MCP
};

struct LogEntry
{
	std::string time;
	LogLevel    level;
	std::string message;
};

class Logger
{
	static std::deque<LogEntry> s_entries;
	static std::mutex           s_mutex;
	static size_t               s_maxEntries;
	static bool                 s_logToFile;
	static std::string          s_logFilePath;

public:
	static void Init(const std::string& logFilePath = "CheatMenu.log", bool logToFile = true);
	static void Log(LogLevel level, const char* fmt, ...);
	static void Info(const char* fmt, ...);
	static void Warn(const char* fmt, ...);
	static void Error(const char* fmt, ...);
	static void Offset(const char* fmt, ...);
	static void Mcp(const char* fmt, ...);

	static std::vector<LogEntry> GetEntries();
	static void Clear();
	static void ExportToFile(const std::string& filepath = "");
	static size_t Count();
	static const char* LevelToString(LogLevel level);
	static CRGBA LevelToColor(LogLevel level);
};
