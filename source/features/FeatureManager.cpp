#include "FeatureManager.h"
#include "Renderer.h"
#include "MathUtils.h"

FeatureManager::FeatureManager()
{
	m_aimbot = std::make_unique<Aimbot>();
	m_esp = std::make_unique<Esp>();
	m_worldEsp = std::make_unique<WorldEsp>();
	m_radar = std::make_unique<Radar>();
	m_airbrake = std::make_unique<Airbrake>();
	m_playerCheats = std::make_unique<PlayerCheats>();
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

	auto local = FindPlayerPed();
	if (local)
	{
		m_airbrake->process(local, g_timeDiff);
	}
}
