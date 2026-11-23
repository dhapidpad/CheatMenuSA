#include "DisplayTweaks.h"
#include "features/diagnostics/Logger.h"

DisplayTweaks::DisplayTweaks()
	: m_isBorderlessApplied(false),
	  m_origStyle(0),
	  m_origExStyle(0),
	  m_gameHwnd(nullptr)
{
	ZeroMemory(&m_origRect, sizeof(RECT));
}

DisplayTweaks::~DisplayTweaks()
{
	if (m_isBorderlessApplied && m_gameHwnd)
	{
		toggle_borderless_fullscreen(false);
	}
}

HWND DisplayTweaks::get_game_window()
{
	if (m_gameHwnd && IsWindow(m_gameHwnd))
		return m_gameHwnd;

	// Attempt via RsGlobal
	__try {
		if (RsGlobal.ps && RsGlobal.ps->window)
		{
			m_gameHwnd = reinterpret_cast<HWND>(RsGlobal.ps->window);
			return m_gameHwnd;
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {}

	// Fallback via window title / class
	m_gameHwnd = FindWindowA("Grand Theft Auto: San Andreas", nullptr);
	if (!m_gameHwnd)
		m_gameHwnd = FindWindowA(nullptr, "GTA: San Andreas");
	if (!m_gameHwnd)
		m_gameHwnd = GetActiveWindow();

	return m_gameHwnd;
}

void DisplayTweaks::toggle_borderless_fullscreen(bool enable)
{
	HWND hwnd = get_game_window();
	if (!hwnd)
	{
		Logger::Warn("Could not find Game Window handle for Borderless Fullscreen");
		return;
	}

	if (enable && !m_isBorderlessApplied)
	{
		// Save original window style and geometry
		m_origStyle = GetWindowLongA(hwnd, GWL_STYLE);
		m_origExStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
		GetWindowRect(hwnd, &m_origRect);

		// Get primary monitor resolution
		int screenW = GetSystemMetrics(SM_CXSCREEN);
		int screenH = GetSystemMetrics(SM_CYSCREEN);

		// Apply borderless popup window styles
		SetWindowLongA(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLongA(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

		// Resize to cover entire display
		SetWindowPos(hwnd, HWND_TOP, 0, 0, screenW, screenH, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		m_isBorderlessApplied = true;
		Logger::Info("Applied Borderless Fullscreen (%dx%d)", screenW, screenH);
	}
	else if (!enable && m_isBorderlessApplied)
	{
		// Restore previous style and geometry
		SetWindowLongA(hwnd, GWL_STYLE, m_origStyle);
		SetWindowLongA(hwnd, GWL_EXSTYLE, m_origExStyle);

		int origW = m_origRect.right - m_origRect.left;
		int origH = m_origRect.bottom - m_origRect.top;
		SetWindowPos(hwnd, HWND_NOTOPMOST, m_origRect.left, m_origRect.top, origW, origH, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		m_isBorderlessApplied = false;
		Logger::Info("Restored Windowed Display Mode");
	}
}

void DisplayTweaks::apply_fov(float fov)
{
	__try {
		*reinterpret_cast<float*>(0x8D5038) = fov;
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void DisplayTweaks::apply_draw_distance(float multiplier)
{
	__try {
		// Default far clip in GTA SA is around 900.0f
		*reinterpret_cast<float*>(0xB7C4F0) = 900.0f * multiplier;
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void DisplayTweaks::apply_fps_settings(bool disableLimiter, int targetFps)
{
	__try {
		// Frame limiter on/off
		*reinterpret_cast<bool*>(0xBA6794) = !disableLimiter;

		// Custom frame limit (30, 60, 90, 120, etc.)
		if (targetFps > 0)
		{
			*reinterpret_cast<unsigned char*>(0xC1704C) = static_cast<unsigned char>(targetFps);
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void DisplayTweaks::apply_aspect_ratio(int mode)
{
	__try {
		float* pAspect = reinterpret_cast<float*>(0xC3EFA4);
		switch (mode)
		{
		case 1: *pAspect = 16.0f / 9.0f; break; // 16:9 Widescreen
		case 2: *pAspect = 21.0f / 9.0f; break; // 21:9 Ultrawide
		case 3: *pAspect = 4.0f / 3.0f;  break; // 4:3 Retro
		default: break; // Default game auto-detect
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void DisplayTweaks::on_game_process()
{
	if (!g_Config) return;

	// 1. Borderless Fullscreen state sync
	if (g_Config->display.borderless_fullscreen != m_isBorderlessApplied)
	{
		toggle_borderless_fullscreen(g_Config->display.borderless_fullscreen);
	}

	// 2. Custom FOV
	if (g_Config->display.custom_fov_enabled)
	{
		apply_fov(g_Config->display.custom_fov);
	}

	// 3. Draw Distance / Far Clip
	if (g_Config->display.draw_distance_enabled)
	{
		apply_draw_distance(g_Config->display.draw_distance_mult);
	}

	// 4. FPS / Frame Limiter
	apply_fps_settings(g_Config->display.fps_limiter_disabled, g_Config->display.target_fps);

	// 5. Aspect Ratio
	if (g_Config->display.aspect_ratio_mode > 0)
	{
		apply_aspect_ratio(g_Config->display.aspect_ratio_mode);
	}
}
