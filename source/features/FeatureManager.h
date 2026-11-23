#pragma once

#include "Globals.h"
#include "Config.h"
#include "combat/Aimbot.h"
#include "visuals/Esp.h"
#include "visuals/WorldEsp.h"
#include "visuals/Radar.h"
#include "movement/Airbrake.h"
#include "player/PlayerCheats.h"

class FeatureManager
{
	std::unique_ptr<Aimbot>       m_aimbot;
	std::unique_ptr<Esp>          m_esp;
	std::unique_ptr<WorldEsp>     m_worldEsp;
	std::unique_ptr<Radar>        m_radar;
	std::unique_ptr<Airbrake>     m_airbrake;
	std::unique_ptr<PlayerCheats> m_playerCheats;

public:
	FeatureManager();
	~FeatureManager() = default;

	void on_draw();
	void on_draw_hud();
	void on_game_process();

	Aimbot* aimbot() const { return m_aimbot.get(); }
	Esp* esp() const { return m_esp.get(); }
	Airbrake* airbrake() const { return m_airbrake.get(); }
	PlayerCheats* player_cheats() const { return m_playerCheats.get(); }
};
