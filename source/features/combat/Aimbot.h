#pragma once

#include "Globals.h"
#include "Config.h"

class Aimbot
{
public:
	Aimbot() = default;
	~Aimbot() = default;

	void on_draw();
	CVector calculate_aim_vector(CPed* firingPed, const CVector& start);
};
