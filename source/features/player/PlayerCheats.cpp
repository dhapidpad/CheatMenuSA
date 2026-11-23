#include "PlayerCheats.h"
#include "GameUtils.h"

void PlayerCheats::process_map_fix()
{
	static auto frame_limiter = FrontEndMenuManager.m_bPrefsVsync;

	if (FrontEndMenuManager.m_nCurrentMenuPage == MENUPAGE_DISPLAY_ADVANCED && frame_limiter != FrontEndMenuManager.m_bPrefsVsync)
		frame_limiter = FrontEndMenuManager.m_bPrefsVsync;

	if (FrontEndMenuManager.m_nCurrentMenuPage == MENUPAGE_MAP && !frame_limiter)
		FrontEndMenuManager.m_bPrefsVsync = true;

	if (FrontEndMenuManager.m_nCurrentMenuPage != MENUPAGE_MAP && FrontEndMenuManager.m_bPrefsVsync != frame_limiter)
		FrontEndMenuManager.m_bPrefsVsync = frame_limiter;
}

void PlayerCheats::on_game_process()
{
	if (!g_isSamp)
		process_map_fix();

	auto local = FindPlayerPed();
	if (!local) return;

	auto local_info = local->GetPlayerInfoForThisPlayerPed();
	if (local_info)
	{
		// Player Godmode
		if (g_Config->player.sp_InvPlayer)
		{
			const float max_armour = static_cast<float>(local_info->m_nMaxArmour);
			const float max_health = std::max(static_cast<float>(local_info->m_nMaxHealth), local->m_fMaxHealth);

			local->m_fArmour = max_armour;
			local->m_fHealth = max_health;

			if (!g_isSamp)
				local->bInvulnerable = true;

			local->CantBeKnockedOffBike = 1;
		}

		auto local_data = &local_info->m_PlayerData;
		if (local_data)
		{
			// Infinite Stamina & Oxygen
			if (g_Config->movement.infinite_stamina)
			{
				local_data->m_fTimeCanRun = 1000.0f;
				local_data->m_bPlayerSprintDisabled = false;
			}

			if (g_Config->movement.infinite_oxygen)
			{
				local_data->m_fBreath = 1000.0f;
				local_data->m_bRequireHandleBreath = 0;
			}
			// Perfect accuracy & range
			if (g_Config->combat.aim_PerfectAccuracy)
			{
				local->m_nWeaponAccuracy = 100;
				local_data->m_fAttackButtonCounter = 0.0f;
				local_info->m_bFastReload = true;
			}

			// Never Wanted
			if (!g_isSamp && g_Config->player.sp_NeverWanted)
			{
				local->SetWantedLevel(0);
			}

			// 6-Star Wanted
			if (!g_isSamp && g_Config->player.sp_RussianMostWanted)
			{
				auto wanted = local->GetWanted();
				if (wanted)
				{
					wanted->MaximumWantedLevel = 6;
					wanted->nMaximumWantedLevel = 9200;
				}
				local->SetWantedLevel(6);
				g_Config->player.sp_RussianMostWanted = false;
			}
		}

		// SAMP Anim Reset ('Z' Key)
		if (utils::key_pressed('Z'))
		{
			if (auto intelligence = local->m_pIntelligence)
				intelligence->ClearTasks(true, true);

			*reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(local) + offsetof(CPhysical, m_nLastCollisionTime) + sizeof(unsigned int)) &= 0xFFFFDFFF;
			local_info->MakePlayerSafe(false, 10.0f);
		}
	}

	// Mega Jump
	if (g_Config->movement.mega_jump && local->m_vecMoveSpeed.z > 0.04f && utils::key_down(RsKeyCodes::rsSPACE))
	{
		local->m_vecMoveSpeed.z += 0.06f;
	}

	// Speedhack
	if (g_Config->movement.speedhack)
	{
		int loops = static_cast<int>(g_Config->movement.speedhack_mult);
		for (int i = 0; i < loops; ++i)
		{
			local->ApplyMoveSpeed();
		}
	}

	// Vehicle Cheats
	auto vehicle = FindPlayerVehicle(-1, false);
	if (vehicle)
	{
		// Vehicle Godmode
		if (g_Config->player.sp_InvCar)
		{
			vehicle->Fix();
			vehicle->m_fHealth = 1000.0f;
		}

		// Auto-Flip Vehicle
		if (g_Config->player.auto_flip_car && vehicle->m_matrix && vehicle->m_matrix->up.z < -0.3f)
		{
			utils::flip_vehicle();
		}

		// Super Heavy Car
		if (g_Config->movement.car_heavy)
		{
			vehicle->bIsHeavy = true;
		}

		// Drive on Water
		if (g_Config->movement.car_drive_on_water && vehicle->m_matrix)
		{
			if (vehicle->m_matrix->pos.z < 0.5f)
			{
				vehicle->m_matrix->pos.z = 1.0f;
				vehicle->m_vecMoveSpeed.z = 0.0f;
			}
		}

		// Nitro Rocket Boost (Left Alt or Left Shift)
		if (g_Config->movement.car_nitro)
		{
			if ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_LSHIFT) & 0x8000))
			{
				vehicle->m_vecMoveSpeed += vehicle->GetForward() * 0.06f;
			}
		}

		// Instant Emergency Stop ('X' Key)
		if (g_Config->movement.car_instant_stop && (GetAsyncKeyState('X') & 0x8000))
		{
			vehicle->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
			vehicle->m_vecTurnSpeed.Set(0.0f, 0.0f, 0.0f);
		}
	}

	// Leg Slide
	if (g_Config->player.sp_LegSlide)
	{
		local->bResetWalkAnims = true;
	}

	// No Reload
	auto weapon = &local->m_aWeapons[local->m_nActiveWeaponSlot];
	if (!g_isSamp && g_Config->player.sp_NoReload && weapon)
	{
		weapon->m_nAmmoInClip = weapon->m_nTotalAmmo;
	}

	// Dumbass Peds & Telekinesis
	if (g_Config->player.sp_DumbassPeds || g_Config->player.sp_Telekinesis)
	{
		CPed* aimTarget = g_Config->player.sp_Telekinesis ? utils::find_ped_to_attack() : nullptr;

		for (auto ped : CPools::ms_pPedPool)
		{
			if (!ped || !ped->IsAlive() || ped == local)
				continue;

			if (g_Config->player.sp_DumbassPeds)
			{
				if (auto intel = ped->m_pIntelligence)
					intel->ClearTasks(true, true);
			}

			if (g_Config->player.sp_Telekinesis && ped == aimTarget)
			{
				if (ped->m_matrix)
				{
					ped->m_matrix->pos = local->GetPosition() + (local->GetForward() * 1.5f);
					ped->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
					ped->bIsStanding = true;
					ped->bWasStanding = true;
					ped->bStayInSamePlace = true;
				}
			}
		}
	}
}
