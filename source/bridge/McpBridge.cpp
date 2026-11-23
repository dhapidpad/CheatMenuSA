#include "McpBridge.h"
#include "features/diagnostics/Logger.h"
#include "features/diagnostics/OffsetScanner.h"
#include "utils/GameUtils.h"
#include "Config.h"
#include <sstream>
#include <iomanip>

McpBridge::McpBridge()
	: m_running(false),
	  m_clientCount(0),
	  m_port(13337),
	  m_lastCommand("None")
{
}

McpBridge::~McpBridge()
{
	stop();
}

bool McpBridge::start(int port)
{
	if (m_running.load())
		return true;

	m_port = port;
	m_running.store(true);

	m_serverThread = std::thread(&McpBridge::server_worker, this);
	Logger::Mcp("MCP Bridge initialized on port %d", m_port);
	return true;
}

void McpBridge::stop()
{
	if (!m_running.load())
		return;

	m_running.store(false);
	if (m_serverThread.joinable())
	{
		m_serverThread.join();
	}
	Logger::Mcp("MCP Bridge stopped");
}

void McpBridge::enqueue_command(const McpCommand& cmd)
{
	std::lock_guard<std::mutex> lock(m_cmdMutex);
	m_pendingCommands.push(cmd);
}

void McpBridge::server_worker()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Logger::Error("MCP Bridge WSAStartup failed");
		m_running.store(false);
		return;
	}

	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSock == INVALID_SOCKET)
	{
		Logger::Error("MCP Bridge socket creation failed");
		WSACleanup();
		m_running.store(false);
		return;
	}

	// Allow address reuse
	int opt = 1;
	setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
	serverAddr.sin_port = htons(static_cast<u_short>(m_port));

	if (bind(serverSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		Logger::Error("MCP Bridge bind failed on port %d", m_port);
		closesocket(serverSock);
		WSACleanup();
		m_running.store(false);
		return;
	}

	if (listen(serverSock, 4) == SOCKET_ERROR)
	{
		Logger::Error("MCP Bridge listen failed");
		closesocket(serverSock);
		WSACleanup();
		m_running.store(false);
		return;
	}

	Logger::Mcp("MCP Socket server listening on 127.0.0.1:%d", m_port);

	while (m_running.load())
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverSock, &readSet);

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000; // 200ms

		int sel = select(0, &readSet, nullptr, nullptr, &timeout);
		if (sel > 0 && FD_ISSET(serverSock, &readSet))
		{
			sockaddr_in clientAddr;
			int clientLen = sizeof(clientAddr);
			SOCKET clientSock = accept(serverSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

			if (clientSock != INVALID_SOCKET)
			{
				m_clientCount.fetch_add(1);
				Logger::Mcp("MCP client connected");
				handle_client(clientSock);
				closesocket(clientSock);
				m_clientCount.fetch_sub(1);
				Logger::Mcp("MCP client disconnected");
			}
		}
	}

	closesocket(serverSock);
	WSACleanup();
}

void McpBridge::handle_client(SOCKET clientSock)
{
	char recvBuf[4096];
	std::string accumulated;

	while (m_running.load())
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(clientSock, &readSet);

		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int sel = select(0, &readSet, nullptr, nullptr, &timeout);
		if (sel <= 0) continue;

		int bytesRecv = recv(clientSock, recvBuf, sizeof(recvBuf) - 1, 0);
		if (bytesRecv <= 0) break;

		recvBuf[bytesRecv] = '\0';
		accumulated += recvBuf;

		// Process complete newline-terminated commands
		size_t pos;
		while ((pos = accumulated.find('\n')) != std::string::npos)
		{
			std::string line = accumulated.substr(0, pos);
			accumulated.erase(0, pos + 1);

			// Trim CR
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			if (line.empty()) continue;

			m_lastCommand = line;
			std::string response = process_json_request(line);
			send(clientSock, response.c_str(), static_cast<int>(response.length()), 0);
		}
	}
}

// Lightweight JSON helper
static std::string extract_json_string(const std::string& json, const std::string& key)
{
	std::string search = "\"" + key + "\":";
	size_t pos = json.find(search);
	if (pos == std::string::npos) return "";

	pos += search.length();
	while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\"'))
		pos++;

	size_t end = json.find('\"', pos);
	if (end == std::string::npos) return "";

	return json.substr(pos, end - pos);
}

static float extract_json_float(const std::string& json, const std::string& key, float defaultVal = 0.0f)
{
	std::string search = "\"" + key + "\":";
	size_t pos = json.find(search);
	if (pos == std::string::npos) return defaultVal;

	pos += search.length();
	while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
		pos++;

	return static_cast<float>(atof(json.c_str() + pos));
}

static int extract_json_int(const std::string& json, const std::string& key, int defaultVal = 0)
{
	std::string search = "\"" + key + "\":";
	size_t pos = json.find(search);
	if (pos == std::string::npos) return defaultVal;

	pos += search.length();
	while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
		pos++;

	return atoi(json.c_str() + pos);
}

static bool extract_json_bool(const std::string& json, const std::string& key, bool defaultVal = false)
{
	std::string search = "\"" + key + "\":";
	size_t pos = json.find(search);
	if (pos == std::string::npos) return defaultVal;

	pos += search.length();
	while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
		pos++;

	if (json.compare(pos, 4, "true") == 0) return true;
	if (json.compare(pos, 5, "false") == 0) return false;
	return defaultVal;
}

std::string McpBridge::process_json_request(const std::string& req)
{
	std::string action = extract_json_string(req, "action");
	if (action.empty())
		action = extract_json_string(req, "command");

	Logger::Mcp("Request: %s", action.c_str());

	if (action == "ping")
	{
		return "{\"status\":\"ok\",\"service\":\"CheatMenuSA-MCP\",\"version\":\"2.0\",\"game\":\"GTA San Andreas\"}\n";
	}
	else if (action == "unload" || action == "eject")
	{
		UnloadCheatMenu();
		return "{\"status\":\"ok\",\"message\":\"CheatMenuSA is unloading and ejecting from game.\"}\n";
	}
	else if (action == "get_player_state")
	{
		std::lock_guard<std::mutex> lock(m_snapshotMutex);
		std::stringstream ss;
		ss << "{\n"
		   << "  \"status\": \"ok\",\n"
		   << "  \"health\": " << m_snapshot.health << ",\n"
		   << "  \"armor\": " << m_snapshot.armor << ",\n"
		   << "  \"x\": " << m_snapshot.x << ",\n"
		   << "  \"y\": " << m_snapshot.y << ",\n"
		   << "  \"z\": " << m_snapshot.z << ",\n"
		   << "  \"heading\": " << m_snapshot.heading << ",\n"
		   << "  \"wanted\": " << m_snapshot.wanted << ",\n"
		   << "  \"in_car\": " << (m_snapshot.in_car ? "true" : "false") << "\n"
		   << "}\n";
		return ss.str();
	}
	else if (action == "teleport")
	{
		float x = extract_json_float(req, "x");
		float y = extract_json_float(req, "y");
		float z = extract_json_float(req, "z");

		McpCommand cmd;
		cmd.type = McpCommand::Type::TELEPORT;
		cmd.x = x;
		cmd.y = y;
		cmd.z = z;
		enqueue_command(cmd);

		return "{\"status\":\"ok\",\"message\":\"Teleport queued\"}\n";
	}
	else if (action == "spawn_vehicle")
	{
		int modelId = extract_json_int(req, "model_id", 411);

		McpCommand cmd;
		cmd.type = McpCommand::Type::SPAWN_VEHICLE;
		cmd.int_val = modelId;
		enqueue_command(cmd);

		return "{\"status\":\"ok\",\"message\":\"Vehicle spawn queued\"}\n";
	}
	else if (action == "scan_offsets")
	{
		McpCommand cmd;
		cmd.type = McpCommand::Type::SCAN_OFFSETS;
		enqueue_command(cmd);

		return OffsetScanner::DumpToJson() + "\n";
	}
	else if (action == "get_logs")
	{
		auto entries = Logger::GetEntries();
		std::stringstream ss;
		ss << "{\n  \"status\": \"ok\",\n  \"logs\": [\n";
		for (size_t i = 0; i < entries.size(); ++i)
		{
			const auto& e = entries[i];
			ss << "    {\"time\": \"" << e.time << "\", \"level\": \"" << Logger::LevelToString(e.level) << "\", \"msg\": \"" << e.message << "\"}"
			   << (i + 1 < entries.size() ? ",\n" : "\n");
		}
		ss << "  ]\n}\n";
		return ss.str();
	}
	else if (action == "set_cheat")
	{
		std::string name = extract_json_string(req, "name");
		bool val = extract_json_bool(req, "enabled", true);

		McpCommand cmd;
		cmd.type = McpCommand::Type::SET_CHEAT;
		cmd.str_val = name;
		cmd.bool_val = val;
		enqueue_command(cmd);

		return "{\"status\":\"ok\",\"message\":\"Setting queued\"}\n";
	}

	return "{\"status\":\"error\",\"message\":\"Unknown action\"}\n";
}

void McpBridge::on_game_process()
{
	// 1. Update Snapshot
	CPed* local = FindPlayerPed(0);
	if (local)
	{
		std::lock_guard<std::mutex> lock(m_snapshotMutex);
		m_snapshot.health = local->m_fHealth;
		m_snapshot.armor = local->m_fArmour;

		CVector pos = local->GetPosition();
		m_snapshot.x = pos.x;
		m_snapshot.y = pos.y;
		m_snapshot.z = pos.z;
		m_snapshot.heading = local->GetHeading();
		m_snapshot.in_car = (local->m_pVehicle != nullptr);

		auto pInfo = &CWorld::Players[0];
		if (pInfo && pInfo->m_PlayerData.m_pWanted)
		{
			m_snapshot.wanted = pInfo->m_PlayerData.m_pWanted->m_nWantedLevel;
		}
	}

	// 2. Process Pending Commands
	std::queue<McpCommand> localQueue;
	{
		std::lock_guard<std::mutex> lock(m_cmdMutex);
		localQueue.swap(m_pendingCommands);
	}

	while (!localQueue.empty())
	{
		auto cmd = localQueue.front();
		localQueue.pop();

		switch (cmd.type)
		{
		case McpCommand::Type::TELEPORT:
		{
			if (local)
			{
				if (local->m_pVehicle)
					local->m_pVehicle->SetPosn(cmd.x, cmd.y, cmd.z);
				else
					local->SetPosn(cmd.x, cmd.y, cmd.z);

				Logger::Mcp("Teleported player to (%.1f, %.1f, %.1f)", cmd.x, cmd.y, cmd.z);
			}
			break;
		}
		case McpCommand::Type::SPAWN_VEHICLE:
		{
			utils::spawn_vehicle(cmd.int_val);
			Logger::Mcp("Spawned vehicle model %d", cmd.int_val);
			break;
		}
		case McpCommand::Type::SCAN_OFFSETS:
		{
			OffsetScanner::ScanAllCoreOffsets();
			break;
		}
		case McpCommand::Type::SET_CHEAT:
		{
			if (g_Config)
			{
				if (cmd.str_val == "godmode") g_Config->player.sp_InvPlayer = cmd.bool_val;
				else if (cmd.str_val == "vehicle_godmode") g_Config->player.sp_InvCar = cmd.bool_val;
				else if (cmd.str_val == "never_wanted") g_Config->player.sp_NeverWanted = cmd.bool_val;
				else if (cmd.str_val == "aimbot") g_Config->combat.aim_Enabled = cmd.bool_val;
				else if (cmd.str_val == "esp") g_Config->visuals.esp_Enabled = cmd.bool_val;
				else if (cmd.str_val == "airbrake") g_Config->movement.airbrake = cmd.bool_val;
				else if (cmd.str_val == "borderless") g_Config->display.borderless_fullscreen = cmd.bool_val;

				Logger::Mcp("Cheat '%s' set to %s", cmd.str_val.c_str(), cmd.bool_val ? "ON" : "OFF");
			}
			break;
		}
		}
	}
}
