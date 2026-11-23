#pragma once

#include "Globals.h"

struct Config
{
	struct Combat
	{
		bool  aim_Enabled = true;
		bool  aim_TargetVisible = true;
		bool  aim_TargetOnScreen = true;
		bool  aim_Smart = true;
		float aim_SmartFov = 90.0f;
		bool  aim_DrawFov = true;
		bool  aim_GangsPriority = false;
		bool  aim_CopsPriority = false;
		bool  aim_IgnoreGroove = true;
		bool  aim_Randomize = false;
		float aim_RandomizeFactor = 0.25f;
		bool  aim_PerfectAccuracy = true;
		bool  triggerbot = false;
		bool  rapid_fire = true;
	} combat;

	struct Visuals
	{
		bool esp_Enabled = true;
		int  esp_BoxType = 1; // 0 = Off, 1 = Full 2D Box, 2 = Corner Box
		bool esp_Skeleton = true;
		bool esp_Health = true;
		bool esp_Armor = true;
		bool esp_Distance = true;
		bool esp_Weapon = true;
		bool esp_AimTarget = true;
		bool esp_Snapline = false;
		bool esp_Crosshair = true;
		bool esp_Radar = false;
		bool esp_Vehicles = false;
		bool esp_Graffiti = false;
		bool esp_Collectibles = false;
	} visuals;

	struct Movement
	{
		bool  airbrake = true;
		bool  airbrake_mode = true; // true = 6-axis camera direction, false = s0beit classic
		float airbrake_speed = 180.0f;
		bool  airbrake_active = false;
		bool  airbrake_slowmo = false;
		bool  speedhack = true;
		float speedhack_mult = 2.0f;
		bool  mega_jump = false;
		bool  infinite_stamina = true;
		bool  infinite_oxygen = true;
		bool  car_nitro = true;
		bool  car_instant_stop = true;
		bool  car_heavy = false;
		bool  car_drive_on_water = false;
	} movement;

	struct Player
	{
		bool sp_InvPlayer = true;
		bool sp_InvCar = true;
		bool sp_NeverWanted = true;
		bool sp_RussianMostWanted = false;
		bool sp_NoReload = true;
		bool sp_LegSlide = false;
		bool sp_Telekinesis = false;
		bool sp_MinigunMode = false;
		bool sp_DumbassPeds = false;
		bool auto_flip_car = true;
	} player;

	struct UI
	{
		float window_x = 80.0f;
		float window_y = 80.0f;
		float window_w = 640.0f;
		float window_h = 440.0f;
		int   active_tab = 0; // 0: Combat, 1: Visuals, 2: Movement, 3: Player, 4: Teleport & Actions
		bool  menu_open = true;
	} ui;
};
