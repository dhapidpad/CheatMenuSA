#pragma once

#include "Globals.h"
#include "Config.h"

namespace utils
{
	bool is_bone_valid(std::size_t bone);
	bool is_weapon_valid(unsigned char slot);
	bool gta_menu_active();
	bool key_pressed(int key_code);
	bool key_down(int key_code);

	CVector   get_bone_position(CPed* pPed, std::size_t bone, bool transformed = false);
	bool      world_to_screen(const CVector& vecWorld, CVector2D& vecScreen);
	bool      world_to_screen(const CVector& vecWorld);
	CVector2D vec_world_to_screen(const CVector& vecWorld);
	CPed*     find_ped_to_attack();
	std::uint64_t time_get();

	bool teleport_to_marker();
	void give_ammo();
	void give_money();
	void give_minigun();
	void repair_vehicle();
	void flip_vehicle();
	void collect_collectibles();
	void unlock_nearby_cars();
}
