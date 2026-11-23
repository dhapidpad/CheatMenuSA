#pragma once

#include "Globals.h"
#include "Config.h"

class WorldEsp
{
	void process_graffiti(CBuilding* building);
	void process_collectible(CPickup* pickup);

public:
	WorldEsp() = default;
	~WorldEsp() = default;

	void on_draw();
};
