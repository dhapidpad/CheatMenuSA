#pragma once

#include "Globals.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

struct McpCommand
{
	enum class Type
	{
		TELEPORT,
		SPAWN_VEHICLE,
		SET_CHEAT,
		SCAN_OFFSETS
	} type;

	float       x = 0.0f;
	float       y = 0.0f;
	float       z = 0.0f;
	int         int_val = 0;
	bool        bool_val = false;
	std::string str_val;
};

class McpBridge
{
	std::thread         m_serverThread;
	std::atomic<bool>   m_running;
	std::atomic<int>    m_clientCount;
	int                 m_port;
	std::string         m_lastCommand;
	std::mutex          m_cmdMutex;
	std::queue<McpCommand> m_pendingCommands;

	// Cached state for safe client querying
	struct PlayerSnapshot
	{
		float health = 100.0f;
		float armor = 0.0f;
		float x = 0.0f, y = 0.0f, z = 0.0f;
		float heading = 0.0f;
		int   wanted = 0;
		bool  in_car = false;
		int   model_id = 0;
	} m_snapshot;
	std::mutex m_snapshotMutex;

public:
	McpBridge();
	~McpBridge();

	bool start(int port = 13337);
	void stop();

	void on_game_process();

	bool is_running() const { return m_running.load(); }
	int  port() const { return m_port; }
	int  client_count() const { return m_clientCount.load(); }
	std::string last_command() const { return m_lastCommand; }

private:
	void server_worker();
	void handle_client(SOCKET clientSock);
	std::string process_json_request(const std::string& requestStr);
	void enqueue_command(const McpCommand& cmd);
};
