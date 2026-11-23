#pragma once

#include "Globals.h"
#include "Config.h"

class Radar
{
	float stretch_x(float a) const { return a * SCREEN_WIDTH / static_cast<float>(DEFAULT_SCREEN_WIDTH); }
	float stretch_y(float a) const { return a * SCREEN_HEIGHT / static_cast<float>(DEFAULT_SCREEN_HEIGHT); }
	float stretch_from_bottom(float a) const { return SCREEN_HEIGHT - stretch_y(a); }

	void process_ped(CPed* local, CPed* ped);

public:
	Radar() = default;
	~Radar() = default;

	void on_draw_hud();
};
