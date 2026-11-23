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

	struct Display
	{
		bool  borderless_fullscreen = false;
		bool  custom_fov_enabled = false;
		float custom_fov = 75.0f; // default is 70.0f
		bool  draw_distance_enabled = false;
		float draw_distance_mult = 1.5f; // 1.0x to 4.0x
		bool  fps_limiter_disabled = true;
		int   target_fps = 60; // 30, 60, 90, 120
		int   aspect_ratio_mode = 0; // 0 = Auto, 1 = 16:9, 2 = 21:9 Ultrawide, 3 = 4:3
	} display;

	struct Debug
	{
		bool  log_to_file = true;
		bool  auto_scroll_logs = true;
		int   log_filter_level = 0; // 0 = all
		bool  mcp_server_enabled = true;
		int   mcp_server_port = 13337;
	} debug;

	struct UI
	{
		float window_x = 60.0f;
		float window_y = 60.0f;
		float window_w = 700.0f;
		float window_h = 470.0f;
		int   active_tab = 0; // 0: Combat, 1: Visuals, 2: Movement, 3: Player, 4: Actions, 5: Display, 6: Offsets, 7: Logs & MCP
		bool  menu_open = true;
	} ui;
};
