#include "Hooks.h"
#include "Config.h"
#include "Menu.h"
#include "GameUtils.h"
#include "features/FeatureManager.h"

#define GetCurrentStackFrame() (reinterpret_cast<std::uint32_t*>(std::uint32_t(_AddressOfReturnAddress()) - sizeof(std::uint32_t*)))

void hooks::install()
{
	g_hookGetWeaponInfo          = safetyhook::create_inline(0x743C60, hooks::GetWeaponInfo);
	g_hookHandleGunShot          = safetyhook::create_inline(0x712E40, hooks::HandleGunShot);
	g_hookFireInstantHitFromCar2 = safetyhook::create_inline(0x73CBA0, hooks::FireInstantHitFromCar2);
	g_hookProcessPed             = safetyhook::create_inline(0x62A380, hooks::CTaskSimpleUseGun_ProcessPed);

	HMODULE hUser32 = GetModuleHandleA("user32.dll");
	if (hUser32)
	{
		auto pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
		if (pSetCursorPos)
			g_hookSetCursorPos = safetyhook::create_inline(pSetCursorPos, hooks::SetCursorPos);
	}

	g_originalInputEventHandler         = RsGlobal.keyboard.inputEventHandler;
	RsGlobal.keyboard.inputEventHandler = hooks::inputEventHandler;
}

void hooks::uninstall()
{
	g_hookGetWeaponInfo          = {};
	g_hookHandleGunShot          = {};
	g_hookFireInstantHitFromCar2 = {};
	g_hookProcessPed             = {};
	g_hookSetCursorPos           = {};

	if (g_originalInputEventHandler)
	{
		RsGlobal.keyboard.inputEventHandler = g_originalInputEventHandler;
		g_originalInputEventHandler         = nullptr;
	}
}

CWeaponInfo* __cdecl hooks::GetWeaponInfo(eWeaponType weaponID, unsigned char skill)
{
	const auto ret = g_hookGetWeaponInfo.call<CWeaponInfo*>(weaponID, skill);

	if (!g_Config || !g_Config->combat.aim_PerfectAccuracy)
		return ret;

	const auto return_address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
	if (return_address == 0x73FB65) // FireInstantHit
	{
		const auto ebp = GetCurrentStackFrame();
		const auto firing_entity = *reinterpret_cast<CPed**>(ebp + 0x4);
		const auto local = FindPlayerPed();

		if (weaponID != 0 && firing_entity == local)
		{
			ret->m_nFlags.bMoveAim = ret->m_nFlags.bMoveFire = 1;
			ret->m_nFlags.bHeavy = ret->m_nFlags.bSlowdown = 0;
			ret->m_fWeaponRange = 4096.0f;
			ret->m_fAccuracy = 1.0f;
		}
	}

	return ret;
}

void __cdecl hooks::HandleGunShot(CVector* start, CVector* end)
{
	const auto return_address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
	if (g_Config && g_Config->combat.aim_Enabled && (return_address == 0x7406EB || return_address == 0x740B33))
	{
		const auto ebp = GetCurrentStackFrame();
		const auto firing_entity = *reinterpret_cast<CPed**>(ebp);
		const auto local = FindPlayerPed();

		if (firing_entity == local && g_Features)
		{
			*end = g_Features->aimbot()->calculate_aim_vector(local, *start);
		}
	}

	return g_hookHandleGunShot.call<void>(start, end);
}

void __fastcall hooks::FireInstantHitFromCar2(CWeapon* ecx, void*, CVector origin, CVector target, CVehicle* vehicle, CPed* owner)
{
	const auto return_address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
	if (g_Config && g_Config->combat.aim_Enabled && return_address == 0x73F3EB)
	{
		const auto local = FindPlayerPed();
		if (owner == local && g_Features)
		{
			target = g_Features->aimbot()->calculate_aim_vector(local, origin);
		}
	}

	return g_hookFireInstantHitFromCar2.thiscall<void>(ecx, origin, target, vehicle, owner);
}

bool __fastcall hooks::CTaskSimpleUseGun_ProcessPed(CTaskSimpleUseGun* thisptr, void*, CPed* ped)
{
	const auto ret = g_hookProcessPed.thiscall<bool>(thisptr, ped);

	if (thisptr && g_Config && g_Config->player.sp_MinigunMode)
	{
		const auto local = CWorld::Players[0].m_pPed;
		if (local && ped == local)
		{
			thisptr->bLefttHand = true;
			thisptr->bRightHand = true;
		}
	}

	return ret;
}

RsEventStatus __cdecl hooks::inputEventHandler(RsEvent event, void* param)
{
	if (utils::gta_menu_active())
		return g_originalInputEventHandler(event, param);

	const auto key_status = reinterpret_cast<RsKeyStatus*>(param);
	const auto key_code = key_status->keyScanCode;

	if (event == rsKEYDOWN)
	{
		if (key_code == RsKeyCodes::rsDEL)
		{
			g_isPanic = !g_isPanic;
		}

		if (key_code == RsKeyCodes::rsINSERT || key_code == RsKeyCodes::rsF11)
		{
			if (g_Menu)
				g_Menu->toggle();
			return rsEVENTNOTPROCESSED;
		}

		if (g_Menu && g_Menu->is_open())
			return rsEVENTNOTPROCESSED;
	}

	if (g_Menu && g_Menu->is_open())
		return rsEVENTNOTPROCESSED;

	return g_originalInputEventHandler(event, param);
}

BOOL WINAPI hooks::SetCursorPos(int X, int Y)
{
	if (g_Config && g_Config->ui.menu_open)
		return TRUE;

	return g_hookSetCursorPos.stdcall<BOOL, int, int>(X, Y);
}
