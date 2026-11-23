#include "Aimbot.h"
#include "GameUtils.h"
#include "MathUtils.h"
#include "Renderer.h"

void Aimbot::on_draw()
{
	if (!g_Config || !g_Config->combat.aim_Enabled || !g_Config->combat.aim_DrawFov)
		return;

	if (TheCamera.m_PlayerWeaponMode.m_nMode == MODE_SNIPER)
		return;

	CVector2D crosshair_pos(SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX, SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY);
	g_Renderer->circle(crosshair_pos.x, crosshair_pos.y, g_Config->combat.aim_SmartFov, color::AccentCyan, 48);
}

void Aimbot::on_game_process()
{
	auto local = FindPlayerPed();
	if (!local) return;

	auto local_info = local->GetPlayerInfoForThisPlayerPed();
	if (local_info && g_Config->combat.rapid_fire)
	{
		local_info->m_PlayerData.m_fAttackButtonCounter = 0.0f;
		local_info->m_bFastReload = true;
	}

	// Triggerbot
	if (g_Config->combat.triggerbot)
	{
		CPed* target = utils::find_ped_to_attack();
		if (target)
		{
			CVector2D crosshair_pos(SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX, SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY);
			CVector head = utils::get_bone_position(target, BONE_HEAD2);
			CVector2D head_s;
			if (utils::world_to_screen(head, head_s))
			{
				if ((crosshair_pos - head_s).Magnitude() < 24.0f)
				{
					if (local_info)
						local_info->m_PlayerData.m_fAttackButtonCounter = 0.0f;
				}
			}
		}
	}
}

CVector Aimbot::calculate_aim_vector(CPed* firingPed, const CVector& start)
{
	auto target = utils::find_ped_to_attack();
	if (!target || !utils::is_weapon_valid(firingPed->m_nActiveWeaponSlot))
		return start;

	CVector2D crosshair_pos(SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX, SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY);
	CVector target_spot = utils::get_bone_position(target, BONE_HEAD2);

	if (g_Config->combat.aim_Smart)
	{
		float best_distance = g_Config->combat.aim_SmartFov;
		for (std::size_t i = 0; i <= 8; ++i)
		{
			CVector bone_pos = utils::get_bone_position(target, i);
			CVector2D bone_s;
			if (utils::world_to_screen(bone_pos, bone_s))
			{
				float dist = (crosshair_pos - bone_s).Magnitude();
				if (dist < best_distance)
				{
					best_distance = dist;
					target_spot = bone_pos;
				}
			}
		}
	}

	if (g_Config->combat.aim_Randomize)
	{
		float factor = g_Config->combat.aim_RandomizeFactor;
		target_spot.x += plugin::RandomNumberInRange<float>(-factor, factor);
		target_spot.y += plugin::RandomNumberInRange<float>(-factor, factor);
		target_spot.z += plugin::RandomNumberInRange<float>(-factor, factor);
	}

	return math::extend_vec3(start, target_spot, 1000.0f);
}
