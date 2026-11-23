#pragma once

#include "Globals.h"
#include "Config.h"
#include "Renderer.h"
#include "Input.h"

class Menu
{
	bool      m_isDragging;
	CVector2D m_dragOffset;
	std::string m_activeTooltip;
	std::string m_statusMessage;
	float       m_statusTimer;

public:
	Menu();
	~Menu() = default;

	void on_draw();
	void toggle() { if (g_Config) g_Config->ui.menu_open = !g_Config->ui.menu_open; }
	bool is_open() const { return g_Config && g_Config->ui.menu_open; }

	void set_status(const std::string& msg);

private:
	void render_window();
	void render_header(float x, float y, float w, float h);
	void render_sidebar(float x, float y, float w, float h);
	void render_content(float x, float y, float w, float h);
	void render_footer(float x, float y, float w, float h);
	void render_cursor();

	// Tabs
	void tab_combat(float x, float y, float w, float h);
	void tab_visuals(float x, float y, float w, float h);
	void tab_movement(float x, float y, float w, float h);
	void tab_player(float x, float y, float w, float h);
	void tab_actions(float x, float y, float w, float h);

	// Controls
	void section_header(float x, float y, float w, const char* title);
	bool checkbox(float x, float y, const char* label, bool& val, const char* tooltip = nullptr);
	bool slider_float(float x, float y, float w, const char* label, float& val, float min, float max, const char* format = "%.1f", const char* tooltip = nullptr);
	bool combobox(float x, float y, float w, const char* label, int& val, const std::vector<std::string>& items, const char* tooltip = nullptr);
	bool button(float x, float y, float w, float h, const char* label, const CRGBA& accentColor = color::AccentCyan, const char* tooltip = nullptr);
};
