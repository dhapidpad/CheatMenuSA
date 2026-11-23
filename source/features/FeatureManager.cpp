#include "FeatureManager.h"
#include "Renderer.h"
#include "MathUtils.h"

FeatureManager::FeatureManager()
{
	Logger::Init("CheatMenu.log", g_Config ? g_Config->debug.log_to_file : true);
	Logger::Info("FeatureManager initialized - CheatMenuSA v2.0 Pro");

	m_aimbot = std::make_unique<Aimbot>();
	m_esp = std::make_unique<Esp>();
	m_worldEsp = std::make_unique<WorldEsp>();
	m_radar = std::make_unique<Radar>();
	m_airbrake = std::make_unique<Airbrake>();
	m_playerCheats = std::make_unique<PlayerCheats>();
	m_displayTweaks = std::make_unique<DisplayTweaks>();
	m_mcpBridge = std::make_unique<McpBridge>();

	if (g_Config && g_Config->debug.mcp_server_enabled)
	{
		m_mcpBridge->start(g_Config->debug.mcp_server_port);
	}
}

void FeatureManager::on_draw()
{
	// Watermark
	static float rainbow = 0.0f;
	rainbow = (rainbow > 1.0f) ? 0.0f : rainbow + 0.0002f;
	CRGBA watermarkCol = math::from_hsb(rainbow, 0.8f, 1.0f);

	if (!g_watermarks.empty() && g_watermarkIndex < g_watermarks.size())
	{
		g_Renderer->text_small(10.0f, SCREEN_HEIGHT - 20.0f, DT_LEFT, watermarkCol, "%s | FPS: %d", g_watermarks[g_watermarkIndex].c_str(), static_cast<int>(CTimer::game_FPS));
	}

	m_aimbot->on_draw();
	m_esp->on_draw();
	m_worldEsp->on_draw();
	m_airbrake->on_draw();
}

void FeatureManager::on_draw_hud()
{
	m_radar->on_draw_hud();
}

void FeatureManager::on_game_process()
{
	m_playerCheats->on_game_process();
	m_aimbot->on_game_process();
	m_displayTweaks->on_game_process();
	m_mcpBridge->on_game_process();

	auto local = FindPlayerPed();
	if (local)
	{
		m_airbrake->process(local, g_timeDiff);
	}
}
