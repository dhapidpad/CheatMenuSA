#include "Esp.h"
#include "GameUtils.h"
#include "Renderer.h"

void Esp::on_draw()
{
	if (!g_Config || !g_Config->visuals.esp_Enabled)
		return;

	auto local = FindPlayerPed();
	if (!local) return;

	CPed* aimTarget = g_Config->visuals.esp_AimTarget ? utils::find_ped_to_attack() : nullptr;

	for (auto ped : CPools::ms_pPedPool)
	{
		if (!ped || !ped->IsAlive() || ped == local || ped->m_fHealth <= 0.0f)
			continue;

		process_ped(local, ped, aimTarget);
	}

	if (g_Config->visuals.esp_Vehicles)
	{
		process_vehicles(local);
	}

	draw_crosshair();
}

void Esp::draw_skeleton(CPed* ped, const CRGBA& color)
{
	static const std::pair<std::size_t, std::size_t> boneConnections[] = {
		{ BONE_HEAD2,         BONE_NECK },
		{ BONE_NECK,          BONE_SPINE1 },
		{ BONE_SPINE1,        BONE_PELVIS },
		{ BONE_NECK,          BONE_LEFTSHOULDER },
		{ BONE_LEFTSHOULDER,  BONE_LEFTELBOW },
		{ BONE_LEFTELBOW,     BONE_LEFTHAND },
		{ BONE_NECK,          BONE_RIGHTSHOULDER },
		{ BONE_RIGHTSHOULDER, BONE_RIGHTELBOW },
		{ BONE_RIGHTELBOW,    BONE_RIGHTHAND },
		{ BONE_PELVIS,        BONE_LEFTKNEE },
		{ BONE_LEFTKNEE,      BONE_LEFTFOOT },
		{ BONE_PELVIS,        BONE_RIGHTKNEE },
		{ BONE_RIGHTKNEE,     BONE_RIGHTFOOT }
	};

	for (const auto& conn : boneConnections)
	{
		CVector b1 = utils::get_bone_position(ped, conn.first);
		CVector b2 = utils::get_bone_position(ped, conn.second);

		CVector2D s1, s2;
		if (utils::world_to_screen(b1, s1) && utils::world_to_screen(b2, s2))
		{
			g_Renderer->line(s1.x, s1.y, s2.x, s2.y, color, 1.0f);
		}
	}
}

void Esp::process_ped(CPed* local, CPed* ped, CPed* aimTarget)
{
	CVector2D left_bone_s(99999.0f, 99999.0f);
	CVector2D right_bone_s(-99999.0f, -99999.0f);
	CVector2D top_bone_s(99999.0f, 99999.0f);
	CVector2D bottom_bone_s(-99999.0f, -99999.0f);
	bool anyBoneVisible = false;

	for (std::size_t i = 0; i <= BONE_RIGHTFOOT; ++i)
	{
		if (!utils::is_bone_valid(i))
			continue;

		CVector bone = utils::get_bone_position(ped, i);
		CVector2D bone_screen;
		if (utils::world_to_screen(bone, bone_screen))
		{
			anyBoneVisible = true;
			if (bone_screen.x < left_bone_s.x) left_bone_s.x = bone_screen.x;
			if (bone_screen.x > right_bone_s.x) right_bone_s.x = bone_screen.x;
			if (bone_screen.y < top_bone_s.y) top_bone_s.y = bone_screen.y;
			if (bone_screen.y > bottom_bone_s.y) bottom_bone_s.y = bone_screen.y;
		}
	}

	if (!anyBoneVisible)
		return;

	// Add slight padding around bones for aesthetics
	float x = left_bone_s.x - 2.0f;
	float y = top_bone_s.y - 2.0f;
	float w = (right_bone_s.x - left_bone_s.x) + 4.0f;
	float h = (bottom_bone_s.y - top_bone_s.y) + 4.0f;

	if (w < 4.0f || h < 4.0f)
		return;

	bool isTarget = (ped == aimTarget);
	CRGBA boxColor = isTarget ? color::DangerRed : color::TextWhite;

	// Skeleton ESP
	if (g_Config->visuals.esp_Skeleton)
	{
		draw_skeleton(ped, isTarget ? color::DangerRed : color::AccentCyan);
	}

	// Bounding Box
	if (g_Config->visuals.esp_BoxType == 1) // Full 2D Box
	{
		g_Renderer->border_only(x - 1.0f, y - 1.0f, w + 2.0f, h + 2.0f, CRGBA(0, 0, 0, 150), 1.0f);
		g_Renderer->border_only(x, y, w, h, boxColor, 1.0f);
		g_Renderer->border_only(x + 1.0f, y + 1.0f, w - 2.0f, h - 2.0f, CRGBA(0, 0, 0, 150), 1.0f);
	}
	else if (g_Config->visuals.esp_BoxType == 2) // Corner Box
	{
		g_Renderer->corner_box(x, y, w, h, boxColor, std::min(w / 3.0f, 10.0f));
	}

	// Snapline
	if (g_Config->visuals.esp_Snapline)
	{
		g_Renderer->line(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT, x + (w / 2.0f), y + h, isTarget ? color::DangerRed : color::AccentCyan);
	}

	// Health Bar (below box)
	float curBottomY = y + h + 2.0f;
	if (g_Config->visuals.esp_Health)
	{
		float health = std::clamp(ped->m_fHealth, 0.0f, 100.0f);
		float barW = std::round((w * health) / 100.0f);

		int red = std::clamp(static_cast<int>(510.0f - health * 5.1f), 0, 255);
		int green = std::clamp(static_cast<int>(health * 5.1f), 0, 255);
		CRGBA healthColor(static_cast<unsigned char>(red), static_cast<unsigned char>(green), 0, 255);

		// Background & fill
		g_Renderer->filled_rect(x - 1.0f, curBottomY, w + 2.0f, 4.0f, CRGBA(0, 0, 0, 160));
		g_Renderer->filled_rect(x, curBottomY + 1.0f, barW, 2.0f, healthColor);
		curBottomY += 5.0f;
	}

	// Armor Bar
	if (g_Config->visuals.esp_Armor && ped->m_fArmour > 0.0f)
	{
		float armor = std::clamp(ped->m_fArmour, 0.0f, 100.0f);
		float barW = std::round((w * armor) / 100.0f);

		g_Renderer->filled_rect(x - 1.0f, curBottomY, w + 2.0f, 4.0f, CRGBA(0, 0, 0, 160));
		g_Renderer->filled_rect(x, curBottomY + 1.0f, barW, 2.0f, color::Silver);
		curBottomY += 5.0f;
	}

	// Distance
	if (g_Config->visuals.esp_Distance)
	{
		float dist = (ped->GetPosition() - local->GetPosition()).Magnitude();
		g_Renderer->text_small(x + (w / 2.0f), curBottomY, DT_CENTER, color::TextWhite, "%.0fm", dist);
		curBottomY += 12.0f;
	}

	// Weapon Held
	if (g_Config->visuals.esp_Weapon)
	{
		auto weapon = &ped->m_aWeapons[ped->m_nActiveWeaponSlot];
		if (weapon && weapon->m_eWeaponType != WEAPONTYPE_UNARMED)
		{
			const char* wepName = utils::get_weapon_name(weapon->m_eWeaponType);
			g_Renderer->text_small(x + (w / 2.0f), curBottomY, DT_CENTER, color::WarningYellow, wepName);
		}
	}
}

void Esp::process_vehicles(CPed* local)
{
	auto localVeh = FindPlayerVehicle(-1, false);

	for (auto veh : CPools::ms_pVehiclePool)
	{
		if (!veh || veh == localVeh)
			continue;

		CVector vehPos = veh->GetPosition();
		float dist = (vehPos - local->GetPosition()).Magnitude();
		if (dist > 180.0f)
			continue;

		CVector2D screenPos;
		if (utils::world_to_screen(vehPos, screenPos))
		{
			int health = static_cast<int>(veh->m_fHealth);
			g_Renderer->text_small(screenPos.x, screenPos.y, DT_CENTER, color::AccentCyan, "[VEH %d] %d HP (%.0fm)", veh->m_nModelIndex, health, dist);
		}
	}
}

void Esp::draw_crosshair()
{
	if (!g_Config || !g_Config->visuals.esp_Crosshair)
		return;

	if (TheCamera.m_PlayerWeaponMode.m_nMode == MODE_SNIPER)
		return;

	float cx = SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX;
	float cy = SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY;

	// Sleek crosshair with center dot and 4 hashes
	g_Renderer->filled_rect(cx - 1.0f, cy - 1.0f, 3.0f, 3.0f, color::WarningYellow);
	g_Renderer->line(cx - 6.0f, cy, cx - 2.0f, cy, color::WarningYellow);
	g_Renderer->line(cx + 3.0f, cy, cx + 7.0f, cy, color::WarningYellow);
	g_Renderer->line(cx, cy - 6.0f, cx, cy - 2.0f, color::WarningYellow);
	g_Renderer->line(cx, cy + 3.0f, cx, cy + 7.0f, color::WarningYellow);
}
