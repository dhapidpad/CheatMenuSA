#pragma once

#include "Globals.h"
#include "Config.h"

class Airbrake
{
	CVector       m_origPos;
	float         m_fallSpeedMult;
	bool          m_wasEnabled;
	std::uint64_t m_timeStart;

public:
	Airbrake();
	~Airbrake() = default;

	void process(CPlayerPed* ped, double time_diff);
	void on_draw();
	bool is_active() const { return g_Config && g_Config->movement.airbrake_active; }
};
