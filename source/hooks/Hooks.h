#pragma once

#include "Globals.h"

namespace hooks
{
	void install();
	void uninstall();

	CWeaponInfo*  __cdecl    GetWeaponInfo(eWeaponType weaponID, unsigned char skill);
	void          __cdecl    HandleGunShot(CVector* start, CVector* end);
	void          __fastcall FireInstantHitFromCar2(CWeapon* ecx, void*, CVector origin, CVector target, CVehicle* vehicle, CPed* owner);
	bool          __fastcall CTaskSimpleUseGun_ProcessPed(CTaskSimpleUseGun* thisptr, void*, CPed* ped);
	RsEventStatus __cdecl    inputEventHandler(RsEvent event, void* param);
	BOOL          WINAPI     SetCursorPos(int X, int Y);
}
