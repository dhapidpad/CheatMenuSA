#pragma once

#include "Globals.h"
#include "Config.h"

class DisplayTweaks
{
	bool   m_isBorderlessApplied;
	LONG   m_origStyle;
	LONG   m_origExStyle;
	RECT   m_origRect;
	HWND   m_gameHwnd;

public:
	DisplayTweaks();
	~DisplayTweaks();

	void on_game_process();
	void toggle_borderless_fullscreen(bool enable);
	void apply_fov(float fov);
	void apply_draw_distance(float multiplier);
	void apply_fps_settings(bool disableLimiter, int targetFps);
	void apply_aspect_ratio(int mode);

private:
	HWND get_game_window();
};
