#include "Logger.h"
#include "render/Colors.h"
#include <cstdarg>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

std::deque<LogEntry> Logger::s_entries;
std::mutex           Logger::s_mutex;
size_t               Logger::s_maxEntries = 300;
bool                 Logger::s_logToFile = true;
std::string          Logger::s_logFilePath = "CheatMenu.log";

void Logger::Init(const std::string& logFilePath, bool logToFile)
{
	std::lock_guard<std::mutex> lock(s_mutex);
	s_logFilePath = logFilePath;
	s_logToFile = logToFile;

	if (s_logToFile)
	{
		std::ofstream ofs(s_logFilePath, std::ios::out | std::ios::trunc);
		if (ofs.is_open())
		{
			ofs << "=== CheatMenuSA v2.0 Pro - Session Log Started ===" << std::endl;
		}
	}
}

void Logger::Log(LogLevel level, const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	// Get current timestamp
	std::time_t now = std::time(nullptr);
	std::tm ltm;
	localtime_s(&ltm, &now);

	char timeBuf[32];
	snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);

	std::string timeStr(timeBuf);
	std::string msgStr(buffer);

	{
		std::lock_guard<std::mutex> lock(s_mutex);
		if (s_entries.size() >= s_maxEntries)
		{
			s_entries.pop_front();
		}
		s_entries.push_back({ timeStr, level, msgStr });

		if (s_logToFile)
		{
			std::ofstream ofs(s_logFilePath, std::ios::out | std::ios::app);
			if (ofs.is_open())
			{
				ofs << "[" << timeStr << "] [" << LevelToString(level) << "] " << msgStr << std::endl;
			}
		}
	}
}

void Logger::Info(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Log(LogLevel::INFO, "%s", buffer);
}

void Logger::Warn(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Log(LogLevel::WARN, "%s", buffer);
}

void Logger::Error(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Log(LogLevel::ERR, "%s", buffer);
}

void Logger::Offset(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Log(LogLevel::OFFSET, "%s", buffer);
}

void Logger::Mcp(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	Log(LogLevel::MCP, "%s", buffer);
}

std::vector<LogEntry> Logger::GetEntries()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	return std::vector<LogEntry>(s_entries.begin(), s_entries.end());
}

void Logger::Clear()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	s_entries.clear();
}

void Logger::ExportToFile(const std::string& filepath)
{
	std::lock_guard<std::mutex> lock(s_mutex);
	std::string targetPath = filepath.empty() ? s_logFilePath : filepath;
	std::ofstream ofs(targetPath, std::ios::out | std::ios::trunc);
	if (!ofs.is_open()) return;

	ofs << "=== CheatMenuSA Log Dump ===" << std::endl;
	for (const auto& entry : s_entries)
	{
		ofs << "[" << entry.time << "] [" << LevelToString(entry.level) << "] " << entry.message << std::endl;
	}
}

size_t Logger::Count()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	return s_entries.size();
}

const char* Logger::LevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::INFO:   return "INFO";
	case LogLevel::WARN:   return "WARN";
	case LogLevel::ERR:    return "ERROR";
	case LogLevel::OFFSET: return "OFFSET";
	case LogLevel::MCP:    return "MCP";
	default:               return "DEBUG";
	}
}

CRGBA Logger::LevelToColor(LogLevel level)
{
	switch (level)
	{
	case LogLevel::INFO:   return color::TextWhite;
	case LogLevel::WARN:   return color::WarningYellow;
	case LogLevel::ERR:    return color::HealthRed;
	case LogLevel::OFFSET: return color::AccentCyan;
	case LogLevel::MCP:    return color::AccentBlue;
	default:               return color::TextMuted;
	}
}
