#pragma once

#include "Globals.h"
#include "Config.h"

class PlayerCheats
{
	void process_map_fix();

public:
	PlayerCheats() = default;
	~PlayerCheats() = default;

	void on_game_process();
};
