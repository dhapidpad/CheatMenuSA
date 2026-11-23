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

	// Car Godmode
	if (g_Config->player.sp_InvCar)
	{
		auto vehicle = FindPlayerVehicle(-1, false);
		if (vehicle)
		{
			vehicle->Fix();
			vehicle->m_fHealth = 1000.0f;
		}
	}

	// Leg Slide
	if (g_Config->player.sp_LegSlide)
	{
		local->bResetWalkAnims = true;
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
