#include "Radar.h"
#include "Renderer.h"

void Radar::process_ped(CPed* local, CPed* ped)
{
	CVector diff = ped->GetPosition() - local->GetPosition();
	if (diff.Magnitude() > 800.0f)
		return;

	float angle = TheCamera.m_mCameraMatrix.right.Heading() - static_cast<float>(M_PI) / 2.0f;

	CVector2D out;
	out.x = diff.x * cosf(angle) - -diff.y * sinf(angle);
	out.y = diff.x * sinf(angle) + -diff.y * cosf(angle);

	const float aspect_ratio = SCREEN_COORD_MAX_X / SCREEN_COORD_MAX_Y;
	out.x /= aspect_ratio;
	out.y /= aspect_ratio;

	out.x = stretch_x(70.0f) + roundf(out.x);
	out.y = stretch_from_bottom(66.0f) + roundf(out.y);

	g_Renderer->bordered_rect(out.x - 3.0f, out.y - 3.0f, 6.0f, 6.0f, color::AccentCyan, CRGBA(0, 0, 0, 160), 1.0f);
}

void Radar::on_draw_hud()
{
	if (!g_Config || !g_Config->visuals.esp_Radar)
		return;

	auto local = FindPlayerPed();
	if (!local) return;

	for (auto ped : CPools::ms_pPedPool)
	{
		if (!ped || !ped->IsAlive() || ped == local || ped->m_fHealth <= 0.0f)
			continue;

		if (!CHud::bDrawingVitalStats)
			process_ped(local, ped);
	}
}
