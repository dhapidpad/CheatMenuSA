#include "GameUtils.h"
#include "MathUtils.h"

bool utils::is_bone_valid(std::size_t bone)
{
	return InRange(bone, 0, 9) || InRange(bone, 20, 27) || InRange(bone, 30, 37) || InRange(bone, 40, 45) || InRange(bone, 50, 55);
}

bool utils::is_weapon_valid(unsigned char slot)
{
	return InRange(slot, 1, 8);
}

bool utils::gta_menu_active()
{
	return FrontEndMenuManager.m_bMenuActive;
}

bool utils::key_pressed(int key_code)
{
	return ControlsManager.GetIsKeyboardKeyJustDown(static_cast<RsKeyCodes>(key_code));
}

bool utils::key_down(int key_code)
{
	return ControlsManager.GetIsKeyboardKeyDown(static_cast<RsKeyCodes>(key_code));
}

CVector utils::get_bone_position(CPed* pPed, std::size_t bone, bool transformed)
{
	if (!pPed) return CVector(0.0f, 0.0f, 0.0f);
	RwV3d vBonePos;
	if (transformed)
		pPed->GetTransformedBonePosition(vBonePos, bone, true);
	else
		pPed->GetBonePosition(vBonePos, bone, true);
	return CVector(vBonePos);
}

bool utils::world_to_screen(const CVector& vecWorld, CVector2D& vecScreen)
{
	auto pCamera = &TheCamera;
	if (!pCamera) return false;

	const auto matrix = &pCamera->m_mViewMatrix;
	const auto dwScreenWidth = RsGlobal.maximumWidth;
	const auto dwScreenHeight = RsGlobal.maximumHeight;

	CVector vTransform;
	vTransform.x = (vecWorld.x * matrix->right.x) + (vecWorld.y * matrix->up.x) + (vecWorld.z * matrix->at.x) + matrix->pos.x;
	vTransform.y = (vecWorld.x * matrix->right.y) + (vecWorld.y * matrix->up.y) + (vecWorld.z * matrix->at.y) + matrix->pos.y;
	vTransform.z = (vecWorld.x * matrix->right.z) + (vecWorld.y * matrix->up.z) + (vecWorld.z * matrix->at.z) + matrix->pos.z;

	if (vTransform.z < 0.0f) return false;

	const double dRecip = 1.0f / vTransform.z;
	vecScreen.x = vTransform.x * static_cast<float>(dRecip * dwScreenWidth);
	vecScreen.y = vTransform.y * static_cast<float>(dRecip * dwScreenHeight);
	return true;
}

bool utils::world_to_screen(const CVector& vecWorld)
{
	CVector2D temp;
	return world_to_screen(vecWorld, temp);
}

CVector2D utils::vec_world_to_screen(const CVector& vecWorld)
{
	CVector2D vecScreen;
	world_to_screen(vecWorld, vecScreen);
	return vecScreen;
}

CPed* utils::find_ped_to_attack()
{
	CPed* closestPed = nullptr;
	float closestDistance = std::numeric_limits<float>::max();

	auto local = FindPlayerPed();
	if (!local) return nullptr;

	CVector origin = FindPlayerCoors(-1);
	CVector end = origin + TheCamera.GetForward() * 100.0f;

	CVector2D crosshair_pos(SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX, SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY);

	for (auto ped : CPools::ms_pPedPool)
	{
		if (!ped || ped == local || !ped->IsAlive() || ped->m_fHealth <= 0.0f)
			continue;

		if (g_Config && g_Config->combat.aim_IgnoreGroove && ped->m_nPedType == PED_TYPE_GANG2)
			continue;

		auto local_head = get_bone_position(local, BONE_HEAD2);
		auto target_head = get_bone_position(ped, BONE_HEAD2);

		if (g_Config && g_Config->combat.aim_TargetVisible)
		{
			if (!CWorld::GetIsLineOfSightClear(local_head, target_head, true, false, false, true, true, false, false))
				continue;
		}

		CVector2D target_s;
		if (!world_to_screen(target_head, target_s))
		{
			if (g_Config && g_Config->combat.aim_TargetOnScreen)
				continue;
		}

		CVector point = ped->GetPosition();
		float dist = CCollision::DistToLine(&origin, &end, &point);
		float pointDist = (crosshair_pos - target_s).Magnitude();

		if (pointDist > 20.0f)
			dist += (pointDist - 20.0f) / 5.0f;

		if (g_Config)
		{
			if ((g_Config->combat.aim_GangsPriority && (ped->m_nPedType >= PED_TYPE_GANG1 && ped->m_nPedType <= PED_TYPE_GANG10))
				|| (g_Config->combat.aim_CopsPriority && ped->m_nPedType == PED_TYPE_COP))
			{
				dist = std::max(0.0f, dist / 2.0f - 2.0f);
			}
		}

		if (dist < closestDistance)
		{
			closestDistance = dist;
			closestPed = ped;
		}
	}

	return closestPed;
}

std::uint64_t utils::time_get()
{
	static int            init = 0;
	static ULARGE_INTEGER init_time;
	FILETIME              ft;
	ULARGE_INTEGER        time_now;

	GetSystemTimeAsFileTime(&ft);
	time_now.LowPart = ft.dwLowDateTime;
	time_now.HighPart = ft.dwHighDateTime;
	time_now.QuadPart /= static_cast<std::uint64_t>(1000);

	if (!init)
	{
		init_time.QuadPart = time_now.QuadPart - MSEC_TO_TIME(1000);
		init = 1;
	}

	return static_cast<std::uint64_t>(time_now.QuadPart - init_time.QuadPart);
}

bool utils::teleport_to_marker()
{
	auto player = FindPlayerPed();
	if (!player) return false;

	// Loop through radar traces looking for target blip
	for (int i = 0; i < 175; ++i)
	{
		auto& trace = CRadar::ms_RadarTrace[i];
		if (trace.m_nBlipDisplay != 0 && (trace.m_nBlipSprite == 41 || trace.m_nBlipType == 1))
		{
			CVector targetPos = trace.m_vPosition;
			float groundZ = CWorld::FindGroundZForCoord(targetPos.x, targetPos.y);
			if (groundZ > -100.0f)
				targetPos.z = groundZ + 1.5f;
			else
				targetPos.z += 2.0f;

			auto veh = FindPlayerVehicle(-1, false);
			if (veh)
			{
				veh->m_matrix->pos = targetPos;
				veh->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
			}
			else
			{
				player->m_matrix->pos = targetPos;
				player->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
			}
			return true;
		}
	}
	return false;
}

void utils::give_ammo()
{
	auto local = FindPlayerPed();
	if (!local) return;

	auto weapon = &local->m_aWeapons[local->m_nActiveWeaponSlot];
	if (weapon && weapon->m_eWeaponType != WEAPONTYPE_UNARMED)
	{
		local->GrantAmmo(weapon->m_eWeaponType, 250);
		weapon->m_nAmmoInClip = weapon->m_nTotalAmmo;
	}
}

void utils::give_money()
{
	auto local = FindPlayerPed();
	if (!local) return;

	auto local_info = local->GetPlayerInfoForThisPlayerPed();
	if (local_info)
		local_info->m_nMoney += 500;

	if (!g_isSamp)
		CPickups::GenerateNewOne(local->GetPosition(), MODEL_MONEY, PICKUP_MONEY, 500, 0, false, nullptr);
}

void utils::give_minigun()
{
	auto local = FindPlayerPed();
	if (!local) return;

	CStreaming::RequestModel(MODEL_MINIGUN, 2);
	CStreaming::LoadAllRequestedModels(false);

	local->GiveWeapon(WEAPON_MINIGUN, 500, true);
	local->SetCurrentWeapon(WEAPON_MINIGUN);

	CStreaming::SetModelIsDeletable(MODEL_MINIGUN);
}

void utils::repair_vehicle()
{
	auto veh = FindPlayerVehicle(-1, false);
	if (veh)
	{
		veh->Fix();
		veh->m_fHealth = 1000.0f;
	}
}

void utils::flip_vehicle()
{
	auto veh = FindPlayerVehicle(-1, false);
	if (veh && veh->m_matrix)
	{
		CVector pos = veh->m_matrix->pos;
		pos.z += 1.0f;
		veh->m_matrix->SetRotateZOnly(veh->m_matrix->GetHeading());
		veh->m_matrix->pos = pos;
		veh->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);
		veh->m_vecTurnSpeed.Set(0.0f, 0.0f, 0.0f);
	}
}

void utils::collect_collectibles()
{
	auto local = FindPlayerPed();
	if (!local) return;
	if (g_isSamp) return;

	// Tag all graffiti
	for (auto building : CPools::ms_pBuildingPool)
	{
		if (!building) continue;

		auto model_index = building->m_nModelIndex;
		auto info = CModelInfo::GetModelInfo(model_index);
		if (!info || !info->IsTagModel()) continue;

		static auto CTagManager_SetAlpha = [](CEntity* entity, std::uint8_t alpha) -> void {
			plugin::Call<0x49CEC0, CEntity*, std::uint8_t>(entity, alpha);
		};

		CTagManager_SetAlpha(building, 255);
	}

	// Collect oysters, horseshoes, snapshots
	for (auto i = 0u; i <= MAX_NUM_PICKUPS; i++)
	{
		auto pickup = &CPickups::aPickUps[i];
		if (!pickup) continue;

		auto type = pickup->m_nPickupType;
		if (type != PICKUP_ONCE && type != PICKUP_SNAPSHOT) continue;

		auto model_index = pickup->m_nModelIndex;
		if (model_index != MODEL_CJ_OYSTER && model_index != MODEL_CJ_HORSE_SHOE && model_index != MODEL_CAMERAPICKUP)
			continue;

		auto local_pos = local->GetPosition();
		pickup->SetPosn(local_pos.x, local_pos.y, local_pos.z);

		if (model_index == MODEL_CAMERAPICKUP)
			CPickups::PictureTaken();
	}
}

void utils::unlock_nearby_cars()
{
	for (auto vehicle : CPools::ms_pVehiclePool)
	{
		if (!vehicle) continue;
		vehicle->m_eDoorLock = DOORLOCK_UNLOCKED;
	}
}
