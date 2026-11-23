#pragma once

#include "Globals.h"
#include "Config.h"

class Esp
{
public:
	Esp() = default;
	~Esp() = default;

	void on_draw();

private:
	void process_ped(CPed* local, CPed* ped, CPed* aimTarget);
	void draw_skeleton(CPed* ped, const CRGBA& color);
	void process_vehicles(CPed* local);
	void draw_crosshair();
};
