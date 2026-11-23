#include "Menu.h"
#include "GameUtils.h"
#include "MathUtils.h"
#include "features/diagnostics/Logger.h"
#include "features/diagnostics/OffsetScanner.h"
#include "features/FeatureManager.h"
#include "features/display/DisplayTweaks.h"
#include "bridge/McpBridge.h"
#include "CAudioEngine.h"

static void PlayMenuSound(int id = 3)
{
	__try {
		AudioEngine.ReportFrontendAudioEvent(id, 0.0f, 1.0f);
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
}

Menu::Menu()
	: m_isDragging(false),
	  m_dragOffset(0.0f, 0.0f),
	  m_activeTooltip(""),
	  m_statusMessage("Ready"),
	  m_statusTimer(0.0f),
	  m_tabIndicatorY(0.0f)
{
}

void Menu::set_status(const std::string& msg)
{
	m_statusMessage = msg;
	m_statusTimer = 180.0f; // roughly 3 seconds at 60 FPS
}

void Menu::on_draw()
{
	if (!is_open()) return;

	m_activeTooltip.clear();
	if (m_statusTimer > 0.0f)
		m_statusTimer -= 1.0f;

	render_window();
	render_cursor();
}

void Menu::render_window()
{
	float x = g_Config->ui.window_x;
	float y = g_Config->ui.window_y;
	float w = g_Config->ui.window_w;
	float h = g_Config->ui.window_h;

	// Clamp window to screen bounds
	if (x < 0.0f) g_Config->ui.window_x = x = 0.0f;
	if (y < 0.0f) g_Config->ui.window_y = y = 0.0f;
	if (x + w > SCREEN_WIDTH) g_Config->ui.window_x = x = std::max(0.0f, SCREEN_WIDTH - w);
	if (y + h > SCREEN_HEIGHT) g_Config->ui.window_y = y = std::max(0.0f, SCREEN_HEIGHT - h);

	// Handle window dragging via header
	const float headerH = 36.0f;
	const float footerH = 24.0f;
	const float sidebarW = 150.0f;

	if (g_Input->is_clicked(x, y, w, headerH))
	{
		m_isDragging = true;
		m_dragOffset = g_Input->mouse_pos() - CVector2D(x, y);
	}

	if (m_isDragging)
	{
		if (g_Input->mouse_down())
		{
			g_Config->ui.window_x = g_Input->mouse_pos().x - m_dragOffset.x;
			g_Config->ui.window_y = g_Input->mouse_pos().y - m_dragOffset.y;
			x = g_Config->ui.window_x;
			y = g_Config->ui.window_y;
		}
		else
		{
			m_isDragging = false;
		}
	}

	// Main window drop shadow & background
	g_Renderer->shadow(x, y, w, h, 8.0f, 14, color::Shadow);
	g_Renderer->bordered_rounded_rect(x, y, w, h, 8.0f, color::BgDark, color::CardBorder, 1.2f);

	// Top accent gradient line
	g_Renderer->gradient_horizontal(x + 8.0f, y + 2.0f, w - 16.0f, 2.0f, color::AccentCyan, color::AccentBlue);

	// Sub-sections
	render_header(x, y + 2.0f, w, headerH);
	render_sidebar(x, y + headerH + 2.0f, sidebarW, h - headerH - footerH - 2.0f);
	render_content(x + sidebarW, y + headerH + 2.0f, w - sidebarW, h - headerH - footerH - 2.0f);
	render_footer(x, y + h - footerH, w, footerH);
}

void Menu::render_header(float x, float y, float w, float h)
{
	// Header background
	g_Renderer->filled_rect(x, y, w, h, color::BgHeader);
	g_Renderer->line(x, y + h, x + w, y + h, color::CardBorder);

	// Cheat Title & Logo
	g_Renderer->text(x + 14.0f, y + 9.0f, DT_LEFT, color::AccentCyan, "CHEATMENU");
	g_Renderer->text(x + 104.0f, y + 9.0f, DT_LEFT, color::TextWhite, "[SA-MP & OFFLINE]");

	// Version Badge
	g_Renderer->bordered_rounded_rect(x + 252.0f, y + 8.0f, 96.0f, 20.0f, 4.0f, color::CardBg, color::AccentCyan, 1.0f);
	g_Renderer->text_small(x + 300.0f, y + 11.0f, DT_CENTER, color::AccentCyan, "PRO v2.5 AAA");

	// Status indicator on right
	const char* sampStatus = g_isSamp ? "SA-MP ONLINE" : "OFFLINE GSOCITY";
	g_Renderer->filled_circle(x + w - 195.0f, y + 17.0f, 3.5f, g_isSamp ? color::SuccessGreen : color::AccentCyan);
	g_Renderer->text_small(x + w - 186.0f, y + 11.0f, DT_LEFT, color::TextDim, "%s", sampStatus);

	// Live FPS Badge
	g_Renderer->bordered_rounded_rect(x + w - 68.0f, y + 8.0f, 56.0f, 20.0f, 4.0f, color::CardBg, color::SuccessGreen, 1.0f);
	g_Renderer->text_small(x + w - 40.0f, y + 11.0f, DT_CENTER, color::SuccessGreen, "%d FPS", static_cast<int>(CTimer::game_FPS));
}

void Menu::render_sidebar(float x, float y, float w, float h)
{
	// Sidebar background
	g_Renderer->filled_rect(x, y, w, h, color::BgSidebar);
	g_Renderer->line(x + w, y, x + w, y + h, color::CardBorder);

	const char* tabNames[] = {
		"[+] COMBAT",
		"[*] VISUALS",
		"[>] MOVEMENT",
		"[#] PLAYER & CAR",
		"[$] ACTIONS & SPAWN",
		"[~] DISPLAY & GFX",
		"[?] DO OFFSET",
		"[!] LOGS & MCP"
	};

	const float tabH = 34.0f;
	float targetIndicatorY = y + 8.0f + (g_Config->ui.active_tab * (tabH + 3.0f));

	// Smooth Lerp Transition for Active Tab
	if (m_tabIndicatorY <= 0.0f)
		m_tabIndicatorY = targetIndicatorY;
	else
		m_tabIndicatorY += (targetIndicatorY - m_tabIndicatorY) * 0.28f;

	// Render Smooth Glowing Pill for Active Tab
	g_Renderer->bordered_rounded_rect(x + 6.0f, m_tabIndicatorY, w - 12.0f, tabH, 5.0f, color::CardBg, color::CardBorder, 1.0f);
	g_Renderer->filled_rounded_rect(x + 6.0f, m_tabIndicatorY, 3.5f, tabH, 2.0f, color::AccentCyan);

	for (int i = 0; i < 8; ++i)
	{
		float tabY = y + 8.0f + (i * (tabH + 3.0f));
		bool isCurrent = (g_Config->ui.active_tab == i);
		bool hovered = g_Input->is_hovered(x + 6.0f, tabY, w - 12.0f, tabH);

		if (hovered && g_Input->mouse_clicked())
		{
			g_Config->ui.active_tab = i;
			isCurrent = true;
			PlayMenuSound(3);
		}

		if (isCurrent)
		{
			g_Renderer->text(x + 16.0f, tabY + 9.0f, DT_LEFT, color::AccentCyan, tabNames[i]);
		}
		else
		{
			if (hovered)
			{
				g_Renderer->filled_rounded_rect(x + 6.0f, tabY, w - 12.0f, tabH, 4.0f, color::CardHover);
				g_Renderer->text(x + 16.0f, tabY + 9.0f, DT_LEFT, color::TextWhite, tabNames[i]);
			}
			else
			{
				g_Renderer->text(x + 16.0f, tabY + 9.0f, DT_LEFT, color::TextMuted, tabNames[i]);
			}
		}
	}

	// Hotkey reminder in sidebar bottom
	g_Renderer->bordered_rounded_rect(x + 8.0f, y + h - 68.0f, w - 16.0f, 60.0f, 4.0f, color::BgDark, color::CardBorder, 1.0f);
	g_Renderer->text_small(x + 14.0f, y + h - 62.0f, DT_LEFT, color::AccentCyan, "HOTKEYS:");
	g_Renderer->text_small(x + 14.0f, y + h - 46.0f, DT_LEFT, color::TextWhite, "INSERT / F11: Menu");
	g_Renderer->text_small(x + 14.0f, y + h - 30.0f, DT_LEFT, color::WarningYellow, "RSHIFT: Airbrake");
	g_Renderer->text_small(x + 14.0f, y + h - 16.0f, DT_LEFT, color::DangerRed, "DELETE: Panic Hide");
}

void Menu::render_content(float x, float y, float w, float h)
{
	switch (g_Config->ui.active_tab)
	{
	case 0: tab_combat(x, y, w, h); break;
	case 1: tab_visuals(x, y, w, h); break;
	case 2: tab_movement(x, y, w, h); break;
	case 3: tab_player(x, y, w, h); break;
	case 4: tab_actions(x, y, w, h); break;
	case 5: tab_display(x, y, w, h); break;
	case 6: tab_offsets(x, y, w, h); break;
	case 7: tab_logs(x, y, w, h); break;
	default: break;
	}
}

void Menu::render_footer(float x, float y, float w, float h)
{
	g_Renderer->filled_rect(x, y, w, h, color::BgHeader);
	g_Renderer->line(x, y, x + w, y, color::CardBorder);

	// Display tooltip or current status
	if (!m_activeTooltip.empty())
	{
		g_Renderer->filled_circle(x + 12.0f, y + (h * 0.5f), 3.0f, color::AccentCyan);
		g_Renderer->text_small(x + 20.0f, y + 6.0f, DT_LEFT, color::AccentCyan, "HUONG DAN:");
		g_Renderer->text_small(x + 94.0f, y + 6.0f, DT_LEFT, color::TextWhite, "%s", m_activeTooltip.c_str());
	}
	else if (m_statusTimer > 0.0f)
	{
		g_Renderer->filled_circle(x + 12.0f, y + (h * 0.5f), 3.0f, color::SuccessGreen);
		g_Renderer->text_small(x + 20.0f, y + 6.0f, DT_LEFT, color::SuccessGreen, "THONG BAO:");
		g_Renderer->text_small(x + 94.0f, y + 6.0f, DT_LEFT, color::TextWhite, "%s", m_statusMessage.c_str());
	}
	else
	{
		g_Renderer->filled_circle(x + 12.0f, y + (h * 0.5f), 3.0f, color::SuccessGreen);
		g_Renderer->text_small(x + 20.0f, y + 6.0f, DT_LEFT, color::TextMuted, "TRANG THAI: SAN SANG (READY) | RE CHUOT VAO MUC BAT KY DE XEM HUONG DAN CHI TIET");
	}

	// Watermark right
	if (!g_watermarks.empty())
	{
		g_Renderer->text_small(x + w - 120.0f, y + 6.0f, DT_LEFT, color::TextMuted, g_watermarks[g_watermarkIndex].c_str());
	}
}

void Menu::render_cursor()
{
	g_Renderer->cursor(g_Input->mouse_pos().x, g_Input->mouse_pos().y);
}

void Menu::section_header(float x, float y, float w, const char* title)
{
	g_Renderer->filled_circle(x + 4.0f, y + 6.0f, 3.0f, color::AccentCyan);
	g_Renderer->text(x + 12.0f, y, DT_LEFT, color::AccentCyan, title);
	g_Renderer->line(x + 12.0f, y + 18.0f, x + w, y + 18.0f, color::CardBorder, 1.0f);
}

bool Menu::checkbox(float x, float y, const char* label, bool& val, const char* tooltip)
{
	const float switchW = 34.0f;
	const float switchH = 16.0f;
	const float totalW = 280.0f;
	const float totalH = 20.0f;

	bool hovered = g_Input->is_hovered(x, y, totalW, totalH);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	if (hovered && g_Input->mouse_clicked())
	{
		val = !val;
		PlayMenuSound(1);
	}

	// Toggle Switch Pill Track
	if (val)
	{
		g_Renderer->filled_rounded_rect(x, y + 2.0f, switchW, switchH, 8.0f, color::AccentCyan);
		g_Renderer->rounded_border(x, y + 2.0f, switchW, switchH, 8.0f, color::AccentBlue, 1.0f);
		// Thumb on the right
		g_Renderer->filled_circle(x + switchW - 9.0f, y + 10.0f, 5.5f, color::TextWhite);
		g_Renderer->text_small(x + switchW + 6.0f, y + 3.0f, DT_LEFT, color::AccentCyan, "ON");
	}
	else
	{
		g_Renderer->filled_rounded_rect(x, y + 2.0f, switchW, switchH, 8.0f, color::CardBg);
		g_Renderer->rounded_border(x, y + 2.0f, switchW, switchH, 8.0f, hovered ? color::TextDim : color::CardBorder, 1.0f);
		// Thumb on the left
		g_Renderer->filled_circle(x + 9.0f, y + 10.0f, 5.5f, color::TextMuted);
		g_Renderer->text_small(x + switchW + 6.0f, y + 3.0f, DT_LEFT, color::TextMuted, "OFF");
	}

	// Label
	CRGBA labelColor = hovered ? color::TextWhite : (val ? color::TextWhite : color::TextDim);
	g_Renderer->text_small(x + switchW + 36.0f, y + 3.0f, DT_LEFT, labelColor, label);

	return val;
}

bool Menu::slider_float(float x, float y, float w, const char* label, float& val, float min, float max, const char* format, const char* tooltip)
{
	const float sliderH = 6.0f;
	const float totalH = 32.0f;
	bool hovered = g_Input->is_hovered(x, y, w, totalH);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	// Label and formatted value pill
	char valBuf[32];
	sprintf_s(valBuf, format, val);
	g_Renderer->text_small(x, y, DT_LEFT, hovered ? color::TextWhite : color::TextDim, label);

	// Value Badge Pill
	g_Renderer->bordered_rounded_rect(x + w - 52.0f, y - 2.0f, 52.0f, 16.0f, 4.0f, color::CardBg, color::AccentCyan, 1.0f);
	g_Renderer->text_small(x + w - 26.0f, y, DT_CENTER, color::AccentCyan, valBuf);

	float trackY = y + 18.0f;
	bool trackHovered = g_Input->is_hovered(x, trackY - 4.0f, w, sliderH + 8.0f);

	if (trackHovered && g_Input->mouse_down())
	{
		float mouseOffset = std::clamp(g_Input->mouse_pos().x - x, 0.0f, w);
		float ratio = mouseOffset / w;
		val = min + ratio * (max - min);
	}

	// Slider track
	g_Renderer->bordered_rounded_rect(x, trackY, w, sliderH, 3.0f, color::BgDark, color::CardBorder, 1.0f);

	// Filled progress
	float ratio = std::clamp((val - min) / (max - min), 0.0f, 1.0f);
	if (ratio > 0.0f)
	{
		g_Renderer->gradient_horizontal(x, trackY, w * ratio, sliderH, color::AccentCyan, color::AccentBlue);
	}

	// Glowing Knob / Thumb
	float thumbX = x + (w * ratio);
	float thumbR = trackHovered ? 7.0f : 5.5f;
	g_Renderer->filled_circle(thumbX, trackY + 3.0f, thumbR, color::TextWhite);
	g_Renderer->circle(thumbX, trackY + 3.0f, thumbR + 1.5f, color::AccentCyan, 18, 1.2f);

	return true;
}

bool Menu::combobox(float x, float y, float w, const char* label, int& val, const std::vector<std::string>& items, const char* tooltip)
{
	const float boxH = 24.0f;
	const float totalH = 42.0f;
	bool hovered = g_Input->is_hovered(x, y, w, totalH);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	g_Renderer->text_small(x, y, DT_LEFT, hovered ? color::TextWhite : color::TextDim, label);

	float boxY = y + 16.0f;
	g_Renderer->bordered_rounded_rect(x, boxY, w, boxH, 4.0f, color::CardBg, hovered ? color::AccentCyan : color::CardBorder, 1.0f);

	if (val >= 0 && val < static_cast<int>(items.size()))
	{
		g_Renderer->filled_circle(x + 10.0f, boxY + 12.0f, 3.0f, color::AccentCyan);
		g_Renderer->text_small(x + 18.0f, boxY + 5.0f, DT_LEFT, color::AccentCyan, items[val].c_str());
	}

	// Left & right cycle arrows
	bool leftHover = g_Input->is_hovered(x + w - 44.0f, boxY + 2.0f, 18.0f, boxH - 4.0f);
	bool rightHover = g_Input->is_hovered(x + w - 22.0f, boxY + 2.0f, 18.0f, boxH - 4.0f);

	g_Renderer->bordered_rounded_rect(x + w - 44.0f, boxY + 2.0f, 18.0f, boxH - 4.0f, 3.0f, leftHover ? color::CardHover : color::BgDark, color::CardBorder, 1.0f);
	g_Renderer->bordered_rounded_rect(x + w - 22.0f, boxY + 2.0f, 18.0f, boxH - 4.0f, 3.0f, rightHover ? color::CardHover : color::BgDark, color::CardBorder, 1.0f);

	g_Renderer->text_small(x + w - 38.0f, boxY + 5.0f, DT_LEFT, leftHover ? color::AccentCyan : color::TextWhite, "<");
	g_Renderer->text_small(x + w - 16.0f, boxY + 5.0f, DT_LEFT, rightHover ? color::AccentCyan : color::TextWhite, ">");

	if (leftHover && g_Input->mouse_clicked())
	{
		val = (val - 1 + static_cast<int>(items.size())) % static_cast<int>(items.size());
		PlayMenuSound(3);
	}
	else if (rightHover && g_Input->mouse_clicked())
	{
		val = (val + 1) % static_cast<int>(items.size());
		PlayMenuSound(3);
	}

	return true;
}

bool Menu::button(float x, float y, float w, float h, const char* label, const CRGBA& accentColor, const char* tooltip)
{
	bool hovered = g_Input->is_hovered(x, y, w, h);
	bool clicked = g_Input->is_clicked(x, y, w, h);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	float pressOffset = (hovered && g_Input->mouse_down()) ? 1.0f : 0.0f;
	CRGBA bg = hovered ? color::CardHover : color::CardBg;
	CRGBA border = hovered ? accentColor : color::CardBorder;

	g_Renderer->bordered_rounded_rect(x, y + pressOffset, w, h, 4.0f, bg, border, 1.2f);
	g_Renderer->text(x + (w / 2.0f), y + pressOffset + (h / 2.0f) - 6.0f, DT_CENTER, hovered ? accentColor : color::TextWhite, label);

	if (clicked)
	{
		PlayMenuSound(3);
	}

	return clicked;
}

void Menu::render_esp_preview(float x, float y, float w, float h)
{
	g_Renderer->bordered_rounded_rect(x, y, w, h, 6.0f, CRGBA(10, 12, 19, 240), color::CardBorder, 1.0f);

	// Preview title
	g_Renderer->filled_circle(x + 12.0f, y + 14.0f, 3.0f, color::AccentCyan);
	g_Renderer->text_small(x + 20.0f, y + 8.0f, DT_LEFT, color::TextWhite, "MO PHONG ESP TRUC QUAN (LIVE)");

	float cx = x + (w * 0.5f);
	float cy = y + 90.0f;

	// Mannequin Head
	g_Renderer->circle(cx, cy - 35.0f, 7.0f, color::AccentCyan, 18, 1.2f);

	// Skeleton Bones (if enabled)
	if (g_Config->visuals.esp_Skeleton)
	{
		// Spine & Neck
		g_Renderer->line(cx, cy - 28.0f, cx, cy - 10.0f, color::AccentCyan, 1.5f);
		g_Renderer->line(cx, cy - 10.0f, cx, cy + 8.0f, color::AccentCyan, 1.5f);

		// Arms
		g_Renderer->line(cx, cy - 18.0f, cx - 18.0f, cy - 10.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx - 18.0f, cy - 10.0f, cx - 22.0f, cy + 4.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx, cy - 18.0f, cx + 18.0f, cy - 10.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx + 18.0f, cy - 10.0f, cx + 22.0f, cy + 4.0f, color::AccentCyan, 1.2f);

		// Legs
		g_Renderer->line(cx, cy + 8.0f, cx - 12.0f, cy + 28.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx - 12.0f, cy + 28.0f, cx - 14.0f, cy + 44.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx, cy + 8.0f, cx + 12.0f, cy + 28.0f, color::AccentCyan, 1.2f);
		g_Renderer->line(cx + 12.0f, cy + 28.0f, cx + 14.0f, cy + 44.0f, color::AccentCyan, 1.2f);
	}

	float boxW = 54.0f;
	float boxH = 92.0f;
	float boxX = cx - (boxW * 0.5f);
	float boxY = cy - 45.0f;

	// Box ESP
	if (g_Config->visuals.esp_BoxType == 1)
	{
		g_Renderer->border_only(boxX, boxY, boxW, boxH, color::AccentCyan, 1.2f);
	}
	else if (g_Config->visuals.esp_BoxType == 2)
	{
		g_Renderer->corner_box(boxX, boxY, boxW, boxH, color::AccentCyan, 8.0f, 1.5f);
	}

	// Health Bar
	if (g_Config->visuals.esp_Health)
	{
		g_Renderer->filled_rect(boxX - 6.0f, boxY, 3.0f, boxH, color::BgDark);
		g_Renderer->gradient_vertical(boxX - 6.0f, boxY + 12.0f, 3.0f, boxH - 12.0f, color::SuccessGreen, color::WarningYellow);
	}

	// Armor Bar
	if (g_Config->visuals.esp_Armor)
	{
		g_Renderer->filled_rect(boxX - 11.0f, boxY, 3.0f, boxH, color::BgDark);
		g_Renderer->filled_rect(boxX - 11.0f, boxY + 30.0f, 3.0f, boxH - 30.0f, color::AccentBlue);
	}

	// Weapon Tag
	if (g_Config->visuals.esp_Weapon)
	{
		g_Renderer->text_small(cx, boxY - 14.0f, DT_CENTER, color::WarningYellow, "M4 Carbine");
	}

	// Distance Tag
	if (g_Config->visuals.esp_Distance)
	{
		g_Renderer->text_small(cx, boxY + boxH + 4.0f, DT_CENTER, color::TextWhite, "28m");
	}

	// Snapline
	if (g_Config->visuals.esp_Snapline)
	{
		g_Renderer->line(cx, y + h - 18.0f, cx, boxY + boxH, color::AccentCyan, 1.0f);
	}

	// Subtitle
	g_Renderer->text_small(x + (w * 0.5f), y + h - 14.0f, DT_CENTER, color::TextMuted, "[THOI GIAN THUC: CAP NHAT THEO TOGGLE]");
}

// ---------------- TAB 0: COMBAT ----------------
void Menu::tab_combat(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Silent Aim Settings
	section_header(col1_x, y + 14.0f, col_w, "SILENT AIMBOT & AUTOMATION");
	checkbox(col1_x, y + 40.0f, "Enable Silent Aim", g_Config->combat.aim_Enabled, "Redirects weapon bullets toward nearest target.");
	checkbox(col1_x, y + 64.0f, "Target Visible Only", g_Config->combat.aim_TargetVisible, "Only aims at targets with clear line of sight.");
	checkbox(col1_x, y + 88.0f, "Target On-Screen Only", g_Config->combat.aim_TargetOnScreen, "Targets must be within camera field of view.");
	checkbox(col1_x, y + 112.0f, "Draw FOV Circle", g_Config->combat.aim_DrawFov, "Draws visual aimbot radius circle on screen.");
	slider_float(col1_x, y + 138.0f, col_w, "Aimbot FOV Radius", g_Config->combat.aim_SmartFov, 20.0f, 400.0f, "%.0f px", "Maximum pixel distance from crosshair to target.");

	checkbox(col1_x, y + 180.0f, "Triggerbot (Auto Attack)", g_Config->combat.triggerbot, "Automatically fires when an enemy crosses crosshair.");
	checkbox(col1_x, y + 204.0f, "Rapid Fire (Full Auto)", g_Config->combat.rapid_fire, "Shoots semi-automatic weapons at maximum cyclic rate.");

	// Column 2: Targeting & Accuracy
	section_header(col2_x, y + 14.0f, col_w, "TARGETING & ACCURACY");
	checkbox(col2_x, y + 40.0f, "Smart Bone Selection", g_Config->combat.aim_Smart, "Chooses closest visible bone to crosshair.");
	checkbox(col2_x, y + 64.0f, "Prioritize Enemy Gangs", g_Config->combat.aim_GangsPriority, "Attacks enemy gang members first.");
	checkbox(col2_x, y + 88.0f, "Prioritize Cops", g_Config->combat.aim_CopsPriority, "Attacks police officers first.");
	checkbox(col2_x, y + 112.0f, "Ignore Grove Street", g_Config->combat.aim_IgnoreGroove, "Will not target friendly Grove Street allies.");
	checkbox(col2_x, y + 136.0f, "Randomize Hit Spot", g_Config->combat.aim_Randomize, "Adds subtle humanized deviation to hits.");
	slider_float(col2_x, y + 162.0f, col_w, "Randomize Factor", g_Config->combat.aim_RandomizeFactor, 0.05f, 0.80f, "%.2f", "Spread factor for hit randomization.");
	checkbox(col2_x, y + 204.0f, "Perfect Accuracy & Range", g_Config->combat.aim_PerfectAccuracy, "Eliminates bullet spread and extends range.");
}

// ---------------- TAB 1: VISUALS ----------------
void Menu::tab_visuals(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Player ESP
	section_header(col1_x, y + 14.0f, col_w, "PLAYER & SKELETON ESP");
	checkbox(col1_x, y + 40.0f, "Master ESP Switch", g_Config->visuals.esp_Enabled, "Toggles all player ESP features on or off.");

	std::vector<std::string> boxTypes = { "Off", "Full 2D Box", "Corner Box" };
	combobox(col1_x, y + 64.0f, col_w, "Bounding Box Style", g_Config->visuals.esp_BoxType, boxTypes, "Select 2D bounding box visual style.");

	checkbox(col1_x, y + 110.0f, "Skeleton ESP (3D Bones)", g_Config->visuals.esp_Skeleton, "Draws accurate wireframe skeletal joints on peds.");
	checkbox(col1_x, y + 134.0f, "Health Bar (Gradient)", g_Config->visuals.esp_Health, "Draws smooth dynamic health bar.");
	checkbox(col1_x, y + 158.0f, "Armor Bar", g_Config->visuals.esp_Armor, "Draws silver armor bar below health.");
	checkbox(col1_x, y + 182.0f, "Distance Indicator", g_Config->visuals.esp_Distance, "Shows distance in meters to target ped.");
	checkbox(col1_x, y + 206.0f, "Weapon Held Display", g_Config->visuals.esp_Weapon, "Displays weapon currently carried by ped.");
	checkbox(col1_x, y + 230.0f, "Highlight Aim Target", g_Config->visuals.esp_AimTarget, "Highlights current aimbot target in red/glow.");
	checkbox(col1_x, y + 254.0f, "Draw Snaplines", g_Config->visuals.esp_Snapline, "Draws tracer line from screen bottom to peds.");

	// Column 2: World & Screen Overlays
	section_header(col2_x, y + 14.0f, col_w, "WORLD & RADAR OVERLAYS");
	checkbox(col2_x, y + 40.0f, "Custom Crosshair", g_Config->visuals.esp_Crosshair, "Draws crisp precision crosshair at weapon aim point.");
	checkbox(col2_x, y + 64.0f, "Mini-Radar Ped Blips", g_Config->visuals.esp_Radar, "Displays nearby peds on the native in-game radar.");
	checkbox(col2_x, y + 88.0f, "Nearby Vehicles ESP", g_Config->visuals.esp_Vehicles, "Shows positions, models, and HP of nearby cars.");
	checkbox(col2_x, y + 112.0f, "Unfinished Graffiti ESP", g_Config->visuals.esp_Graffiti, "Marks uncompleted gang tags in the game world.");
	checkbox(col2_x, y + 136.0f, "Collectibles ESP", g_Config->visuals.esp_Collectibles, "Shows oysters, horseshoes, and photo snapshots.");

	// Interactive Live ESP Mannequin Preview Widget
	render_esp_preview(col2_x, y + 168.0f, col_w, 200.0f);
}

// ---------------- TAB 2: MOVEMENT ----------------
void Menu::tab_movement(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Airbrake (Fly Mode)
	section_header(col1_x, y + 14.0f, col_w, "AIRBRAKE (FREE FLY)");
	checkbox(col1_x, y + 40.0f, "Enable Airbrake", g_Config->movement.airbrake, "Allows free flight through air and solid objects.");
	checkbox(col1_x, y + 64.0f, "6-DOF Camera Direction", g_Config->movement.airbrake_mode, "Moves directly toward wherever the camera looks.");
	slider_float(col1_x, y + 90.0f, col_w, "Flight Speed", g_Config->movement.airbrake_speed, 40.0f, 500.0f, "%.0f", "Movement speed while airbrake is active.");

	// Hotkey guide card
	g_Renderer->bordered_rect(col1_x, y + 134.0f, col_w, 86.0f, color::CardBg, color::CardBorder, 1.0f);
	g_Renderer->text_small(col1_x + 10.0f, y + 140.0f, DT_LEFT, color::AccentCyan, "AIRBRAKE CONTROLS:");
	g_Renderer->text_small(col1_x + 10.0f, y + 154.0f, DT_LEFT, color::TextWhite, "RSHIFT: Toggle Flight On/Off");
	g_Renderer->text_small(col1_x + 10.0f, y + 168.0f, DT_LEFT, color::TextWhite, "W / S / A / D: Move  |  SPACE / LSHIFT: Up/Down");
	g_Renderer->text_small(col1_x + 10.0f, y + 182.0f, DT_LEFT, color::WarningYellow, "NUMPAD0: Slow-Motion Precision Mode");

	// Column 2: Speedhack & Physics
	section_header(col2_x, y + 14.0f, col_w, "PHYSICS & SPRINT");
	checkbox(col2_x, y + 40.0f, "Enable Speedhack", g_Config->movement.speedhack, "Multiplies on-foot running and walking velocity.");
	slider_float(col2_x, y + 66.0f, col_w, "Velocity Multiplier", g_Config->movement.speedhack_mult, 1.5f, 6.0f, "%.1fx", "Speed multiplier applied to player movement.");
	checkbox(col2_x, y + 110.0f, "Super Mega Jump", g_Config->movement.mega_jump, "Jump 10x higher into the air (Hold Space).");
	checkbox(col2_x, y + 134.0f, "Infinite Sprint Stamina", g_Config->movement.infinite_stamina, "Never get tired or exhausted while running.");
	checkbox(col2_x, y + 158.0f, "Infinite Oxygen (Never Drown)", g_Config->movement.infinite_oxygen, "Breathe underwater indefinitely.");
	checkbox(col2_x, y + 182.0f, "Car Rocket Nitro (ALT/SHIFT)", g_Config->movement.car_nitro, "Hold Left Alt or Shift for extreme forward boost.");
	checkbox(col2_x, y + 206.0f, "Emergency Brake ('X' Key)", g_Config->movement.car_instant_stop, "Instantly freezes vehicle in place.");
	checkbox(col2_x, y + 230.0f, "Super Heavy Ramming Car", g_Config->movement.car_heavy, "Send other vehicles flying into the horizon.");
	checkbox(col2_x, y + 254.0f, "Drive On Water Surface", g_Config->movement.car_drive_on_water, "Drive cars across lakes and oceans without sinking.");
}

// ---------------- TAB 3: PLAYER & VEHICLE ----------------
void Menu::tab_player(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Player Cheats
	section_header(col1_x, y + 14.0f, col_w, "PLAYER GODMODE & ATTRIBUTES");
	checkbox(col1_x, y + 40.0f, "Godmode (Infinite Health/Armor)", g_Config->player.sp_InvPlayer, "Full invulnerability to bullets, explosions, and falls.");
	checkbox(col1_x, y + 64.0f, "Never Wanted (0 Stars)", g_Config->player.sp_NeverWanted, "Freezes police wanted level at 0.");
	checkbox(col1_x, y + 88.0f, "Infinite Ammo / No Reload", g_Config->player.sp_NoReload, "Weapon magazine never empties.");
	checkbox(col1_x, y + 112.0f, "Minigun Firing Anims", g_Config->player.sp_MinigunMode, "Dual-handed minigun firing posture for all weapons.");
	checkbox(col1_x, y + 136.0f, "Leg Slide Glitch", g_Config->player.sp_LegSlide, "Smooth ice-skating animation glitch.");
	checkbox(col1_x, y + 160.0f, "Telekinesis Pull", g_Config->player.sp_Telekinesis, "Levitates and freezes target ped directly in front of you.");
	checkbox(col1_x, y + 184.0f, "Dumbass Peds", g_Config->player.sp_DumbassPeds, "Clears ped AI tasks, freezing everyone in place.");

	// Column 2: Vehicle & World Cheats
	section_header(col2_x, y + 14.0f, col_w, "VEHICLE & ENVIRONMENT");
	checkbox(col2_x, y + 40.0f, "Vehicle Godmode", g_Config->player.sp_InvCar, "Player vehicle takes zero damage and auto-repairs.");
	checkbox(col2_x, y + 64.0f, "Auto-Flip Overturned Vehicle", g_Config->player.auto_flip_car, "Automatically flips car upright when upside down.");

	if (button(col2_x, y + 92.0f, col_w, 24.0f, "Repair & Fix Vehicle", color::AccentCyan, "Instantly repairs engine and body of current vehicle."))
	{
		utils::repair_vehicle();
		set_status("Vehicle repaired and restored to full health!");
	}

	if (button(col2_x, y + 120.0f, col_w, 24.0f, "Flip / Upright Vehicle", color::AccentCyan, "Restores flipped or rolled vehicle to upright position."))
	{
		utils::flip_vehicle();
		set_status("Vehicle flipped upright!");
	}

	if (button(col2_x, y + 148.0f, col_w, 24.0f, "Trigger 6-Star Wanted", color::WarningYellow, "Instantly triggers maximum 6-star military response."))
	{
		g_Config->player.sp_RussianMostWanted = true;
		set_status("6-Star wanted level triggered!");
	}

	// Environment controls
	float btnHalfW = (col_w - 6.0f) / 2.0f;
	if (button(col2_x, y + 180.0f, btnHalfW, 24.0f, "Time: 12:00 (Noon)", color::AccentCyan, "Sets game clock to 12:00 PM."))
	{
		utils::set_game_time(12, 0);
		set_status("Game time set to 12:00 Noon!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 180.0f, btnHalfW, 24.0f, "Time: 00:00 (Night)", color::AccentCyan, "Sets game clock to midnight."))
	{
		utils::set_game_time(0, 0);
		set_status("Game time set to 00:00 Midnight!");
	}

	if (button(col2_x, y + 208.0f, btnHalfW, 24.0f, "Weather: Sunny", color::SuccessGreen, "Forces sunny and clear skies."))
	{
		utils::set_game_weather(0);
		set_status("Weather set to Sunny!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 208.0f, btnHalfW, 24.0f, "Weather: Foggy", color::WarningYellow, "Forces foggy weather."))
	{
		utils::set_game_weather(9);
		set_status("Weather set to Foggy!");
	}
}

// ---------------- TAB 4: TELEPORT & ACTIONS ----------------
void Menu::tab_actions(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Map Teleport & Famous Landmarks
	section_header(col1_x, y + 14.0f, col_w, "MAP & LANDMARK TELEPORT");

	if (button(col1_x, y + 40.0f, col_w, 28.0f, ">> TELEPORT TO MAP MARKER <<", color::AccentCyan, "Teleports player (or vehicle) directly to map waypoint."))
	{
		if (utils::teleport_to_marker())
			set_status("Teleported successfully to marker position!");
		else
			set_status("No active map marker found! Place a waypoint on map first.");
	}

	float btnHalfW = (col_w - 6.0f) / 2.0f;

	if (button(col1_x, y + 74.0f, btnHalfW, 24.0f, "Grove Street", color::SuccessGreen, "Teleport to CJ's house in Grove Street."))
	{
		utils::teleport_to_pos(CVector(2493.19f, -1670.85f, 13.33f));
		set_status("Teleported to Grove Street!");
	}
	if (button(col1_x + btnHalfW + 6.0f, y + 74.0f, btnHalfW, 24.0f, "Mount Chiliad", color::AccentCyan, "Teleport to peak of Mount Chiliad."))
	{
		utils::teleport_to_pos(CVector(-2337.0f, -1627.0f, 483.7f));
		set_status("Teleported to Mount Chiliad Peak!");
	}

	if (button(col1_x, y + 102.0f, btnHalfW, 24.0f, "Area 69 Base", color::DangerRed, "Teleport to Area 69 military base."))
	{
		utils::teleport_to_pos(CVector(213.0f, 1904.0f, 17.6f));
		set_status("Teleported to Area 69 Base!");
	}
	if (button(col1_x + btnHalfW + 6.0f, y + 102.0f, btnHalfW, 24.0f, "LS Airport", color::AccentCyan, "Teleport to Los Santos International Airport."))
	{
		utils::teleport_to_pos(CVector(1966.0f, -2179.0f, 13.5f));
		set_status("Teleported to Los Santos Airport!");
	}

	if (button(col1_x, y + 130.0f, btnHalfW, 24.0f, "SF Airport", color::AccentCyan, "Teleport to San Fierro Airport."))
	{
		utils::teleport_to_pos(CVector(-1422.0f, -287.0f, 14.1f));
		set_status("Teleported to San Fierro Airport!");
	}
	if (button(col1_x + btnHalfW + 6.0f, y + 130.0f, btnHalfW, 24.0f, "LV Strip", color::AccentGold, "Teleport to Las Venturas Casino Strip."))
	{
		utils::teleport_to_pos(CVector(2028.0f, 1008.0f, 10.8f));
		set_status("Teleported to Las Venturas Strip!");
	}

	// Quick Spawns in Column 1 bottom
	if (button(col1_x, y + 162.0f, btnHalfW, 24.0f, "Ammo (+250)", color::AccentCyan, "Grants 250 ammo to held weapon."))
	{
		utils::give_ammo();
		set_status("Granted 250 ammo!");
	}
	if (button(col1_x + btnHalfW + 6.0f, y + 162.0f, btnHalfW, 24.0f, "Cash (+$500)", color::SuccessGreen, "Grants $500 cash to CJ."))
	{
		utils::give_money();
		set_status("Granted $500 cash!");
	}

	if (button(col1_x, y + 190.0f, col_w, 24.0f, "Collect All Oysters / 100 Tags", color::AccentCyan, "Sprays all 100 tags and grabs oysters/horseshoes."))
	{
		utils::collect_collectibles();
		set_status("All tags painted and collectibles retrieved!");
	}

	if (button(col1_x, y + 218.0f, col_w, 24.0f, "Unlock All Nearby Vehicles", color::AccentCyan, "Sets all nearby locked cars to unlocked."))
	{
		utils::unlock_nearby_cars();
		set_status("All nearby vehicles unlocked!");
	}

	// Column 2: Vehicle Spawner
	section_header(col2_x, y + 14.0f, col_w, "VEHICLE SPAWNER");

	if (button(col2_x, y + 40.0f, btnHalfW, 24.0f, "Infernus", color::AccentCyan, "Spawns Infernus supercar."))
	{
		if (utils::spawn_vehicle(411)) set_status("Spawned Infernus!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 40.0f, btnHalfW, 24.0f, "NRG-500", color::AccentCyan, "Spawns NRG-500 racing motorcycle."))
	{
		if (utils::spawn_vehicle(522)) set_status("Spawned NRG-500!");
	}

	if (button(col2_x, y + 68.0f, btnHalfW, 24.0f, "Hydra Jet", color::AccentBlue, "Spawns military Hydra fighter jet."))
	{
		if (utils::spawn_vehicle(520)) set_status("Spawned Hydra Jet!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 68.0f, btnHalfW, 24.0f, "Rhino Tank", color::DangerRed, "Spawns heavy Rhino military tank."))
	{
		if (utils::spawn_vehicle(432)) set_status("Spawned Rhino Tank!");
	}

	if (button(col2_x, y + 96.0f, btnHalfW, 24.0f, "Hunter Heli", color::AccentBlue, "Spawns Hunter military attack helicopter."))
	{
		if (utils::spawn_vehicle(425)) set_status("Spawned Hunter Chopper!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 96.0f, btnHalfW, 24.0f, "Monster Truck", color::WarningYellow, "Spawns Monster Truck."))
	{
		if (utils::spawn_vehicle(444)) set_status("Spawned Monster Truck!");
	}

	if (button(col2_x, y + 124.0f, btnHalfW, 24.0f, "Vortex Hover", color::AccentCyan, "Spawns Vortex amphibious hovercraft."))
	{
		if (utils::spawn_vehicle(539)) set_status("Spawned Vortex Hovercraft!");
	}
	if (button(col2_x + btnHalfW + 6.0f, y + 124.0f, btnHalfW, 24.0f, "Sanchez Bike", color::SuccessGreen, "Spawns Sanchez off-road dirt bike."))
	{
		if (utils::spawn_vehicle(468)) set_status("Spawned Sanchez!");
	}

	if (button(col2_x, y + 154.0f, col_w, 24.0f, "Spawn Heavy Minigun", color::AccentBlue, "Loads and grants minigun with 500 rounds."))
	{
		utils::give_minigun();
		set_status("Minigun loaded and equipped!");
	}

	if (button(col2_x, y + 184.0f, col_w, 28.0f, "PANIC SWITCH (HIDE ALL)", color::DangerRed, "Instantly disables cheats and hides menu (DELETE key)."))
	{
		g_isPanic = true;
		g_Config->ui.menu_open = false;
	}
}

void Menu::tab_display(float x, float y, float w, float h)
{
	float col_w = (w - 36.0f) * 0.5f;
	float col1_x = x + 12.0f;
	float col2_x = col1_x + col_w + 12.0f;

	// Column 1: Display & Window Mode
	section_header(col1_x, y + 14.0f, col_w, "DISPLAY & WINDOW MODE");

	checkbox(col1_x, y + 42.0f, "Borderless Fullscreen (Khong vien)", g_Config->display.borderless_fullscreen, "Chuyen game sang che do khong vien toan man hinh (ho tro Alt+Tab muot ma khong crash).");

	std::vector<std::string> aspectModes = { "Auto / Mac dinh", "16:9 Widescreen", "21:9 Ultrawide", "4:3 Classic" };
	combobox(col1_x, y + 74.0f, col_w, "Ti le man hinh (Aspect Ratio)", g_Config->display.aspect_ratio_mode, aspectModes, "Sua loi meo hinh, keo dai nhan vat tren man hinh rong.");

	checkbox(col1_x, y + 124.0f, "Tat gioi han FPS (Unlock FPS)", g_Config->display.fps_limiter_disabled, "Tat frame limiter mac dinh cua game de trai nghiem 60+ FPS.");

	std::vector<std::string> fpsModes = { "30 FPS (Chuan)", "60 FPS (Muot ma)", "90 FPS", "120 FPS", "Uncapped (Cuc dai)" };
	static int fpsIdx = 1;
	if (combobox(col1_x, y + 156.0f, col_w, "Muc tieu FPS (Target FPS)", fpsIdx, fpsModes, "Cai dat so khung hinh muc tieu."))
	{
		if (fpsIdx == 0) g_Config->display.target_fps = 30;
		else if (fpsIdx == 1) g_Config->display.target_fps = 60;
		else if (fpsIdx == 2) g_Config->display.target_fps = 90;
		else if (fpsIdx == 3) g_Config->display.target_fps = 120;
		else g_Config->display.target_fps = 0;
	}

	// Column 2: Camera FOV & Render Distance
	section_header(col2_x, y + 14.0f, col_w, "CAMERA & GRAPHICS ENGINE");

	checkbox(col2_x, y + 42.0f, "Tuy chinh goc nhin (Custom FOV)", g_Config->display.custom_fov_enabled, "Bat goc nhin rong hien dai.");
	slider_float(col2_x, y + 74.0f, col_w, "Goc nhin FOV (Camera Angle)", g_Config->display.custom_fov, 50.0f, 120.0f, "%.1f deg", "Chinh do rong tam nhin camera (mac dinh ~70 deg).");

	checkbox(col2_x, y + 124.0f, "Tang tam nhin xa (Draw Distance)", g_Config->display.draw_distance_enabled, "Mo rong khoang cach hien thi map, bo suong mu gioi han.");
	slider_float(col2_x, y + 156.0f, col_w, "He so tam nhin (Distance Multiplier)", g_Config->display.draw_distance_mult, 1.0f, 4.0f, "%.1fx", "Nhan boi khoang cach load canh vat (Far Clip).");

	if (button(col2_x, y + 210.0f, col_w, 28.0f, "Ap dung thiet lap do hoa", color::AccentCyan, "Kich hoat cac thay doi do hoa ngay lap tuc."))
	{
		set_status("Thiet lap do hoa & man hinh da duoc cap nhat!");
	}
}

void Menu::tab_offsets(float x, float y, float w, float h)
{
	float topY = y + 12.0f;
	float btnW = 180.0f;

	if (button(x + 12.0f, topY, btnW, 26.0f, "QUET MEMORY OFFSET", color::AccentCyan, "Quet toan bo cac con tro, Pool va dia chi cau truc GTA SA."))
	{
		OffsetScanner::ScanAllCoreOffsets();
		set_status("Da hoan tat quet Memory Offset!");
	}

	if (button(x + 200.0f, topY, btnW, 26.0f, "XUAT RA FILE TXT", color::AccentBlue, "Xuat danh sach offset ra file offsets_dump.txt"))
	{
		OffsetScanner::ExportToFile("offsets_dump.txt");
		set_status("Da xuat danh sach ra file offsets_dump.txt");
	}

	// Header Table
	float tableY = topY + 36.0f;
	float tableW = w - 24.0f;
	float tableH = h - 60.0f;

	g_Renderer->filled_rect(x + 12.0f, tableY, tableW, 22.0f, color::BgHeader);
	g_Renderer->bordered_rect(x + 12.0f, tableY, tableW, tableH, color::CardBg, color::CardBorder, 1.0f);

	g_Renderer->text_small(x + 18.0f, tableY + 5.0f, DT_LEFT, color::AccentCyan, "CAU TRUC / OFFSET");
	g_Renderer->text_small(x + 190.0f, tableY + 5.0f, DT_LEFT, color::AccentCyan, "DIA CHI (HEX)");
	g_Renderer->text_small(x + 280.0f, tableY + 5.0f, DT_LEFT, color::AccentCyan, "GIA TRI HIEN TAI");
	g_Renderer->text_small(x + 400.0f, tableY + 5.0f, DT_LEFT, color::AccentCyan, "MO TA");

	// Items
	const auto& results = OffsetScanner::GetResults();
	if (results.empty())
	{
		g_Renderer->text(x + 150.0f, tableY + 60.0f, DT_LEFT, color::TextMuted, "Chua thuc hien quet. Hay bam 'QUET MEMORY OFFSET' o tren!");
	}
	else
	{
		size_t maxItems = std::min(results.size(), size_t(11));
		for (size_t i = 0; i < maxItems; ++i)
		{
			float rowY = tableY + 24.0f + (i * 22.0f);
			if (i % 2 == 1)
			{
				g_Renderer->filled_rect(x + 13.0f, rowY, tableW - 2.0f, 21.0f, color::BgSidebar);
			}

			const auto& o = results[i];
			g_Renderer->text_small(x + 18.0f, rowY + 4.0f, DT_LEFT, o.found ? color::TextWhite : color::TextMuted, o.name.c_str());
			g_Renderer->text_small(x + 190.0f, rowY + 4.0f, DT_LEFT, color::AccentCyan, o.addressHex.c_str());
			g_Renderer->text_small(x + 280.0f, rowY + 4.0f, DT_LEFT, color::WarningYellow, o.valueStr.c_str());
			g_Renderer->text_small(x + 400.0f, rowY + 4.0f, DT_LEFT, color::TextMuted, o.description.c_str());
		}
	}
}

void Menu::tab_logs(float x, float y, float w, float h)
{
	float topY = y + 10.0f;

	// MCP Bridge Status Badge
	bool mcpActive = (g_Features && g_Features->mcp_bridge() && g_Features->mcp_bridge()->is_running());
	int clients = mcpActive ? g_Features->mcp_bridge()->client_count() : 0;
	int port = mcpActive ? g_Features->mcp_bridge()->port() : 13337;

	if (mcpActive)
	{
		g_Renderer->bordered_rect(x + 12.0f, topY, 230.0f, 26.0f, color::CardBg, color::SuccessGreen, 1.0f);
		g_Renderer->text_small(x + 18.0f, topY + 7.0f, DT_LEFT, color::SuccessGreen, "MCP SERVER: 127.0.0.1:%d (%d Client)", port, clients);
	}
	else
	{
		g_Renderer->bordered_rect(x + 12.0f, topY, 150.0f, 26.0f, color::CardBg, color::HealthRed, 1.0f);
		g_Renderer->text_small(x + 18.0f, topY + 7.0f, DT_LEFT, color::HealthRed, "MCP SERVER: OFFLINE");
	}

	if (button(x + 250.0f, topY, 110.0f, 26.0f, "XOA LOG", color::WarningYellow, "Xoa toan bo nhat ky."))
	{
		Logger::Clear();
		set_status("Da xoa log!");
	}

	if (button(x + 368.0f, topY, 130.0f, 26.0f, "LUU FILE LOG", color::AccentCyan, "Ghi nhat ky ra file CheatMenu.log"))
	{
		Logger::ExportToFile("CheatMenu.log");
		set_status("Da luu log ra CheatMenu.log");
	}

	// Console Window
	float boxY = topY + 34.0f;
	float boxW = w - 24.0f;
	float boxH = h - 68.0f;

	g_Renderer->bordered_rect(x + 12.0f, boxY, boxW, boxH, CRGBA(8, 10, 15, 240), color::CardBorder, 1.0f);

	auto entries = Logger::GetEntries();
	if (entries.empty())
	{
		g_Renderer->text_small(x + 24.0f, boxY + 20.0f, DT_LEFT, color::TextMuted, "Chua co log nao duoc ghi.");
	}
	else
	{
		// Render the most recent lines that fit
		size_t maxLines = 13;
		size_t startIdx = (entries.size() > maxLines) ? (entries.size() - maxLines) : 0;

		for (size_t i = startIdx; i < entries.size(); ++i)
		{
			float lineY = boxY + 10.0f + ((i - startIdx) * 18.0f);
			const auto& e = entries[i];

			g_Renderer->text_small(x + 18.0f, lineY, DT_LEFT, color::TextMuted, "[%s]", e.time.c_str());
			g_Renderer->text_small(x + 72.0f, lineY, DT_LEFT, Logger::LevelToColor(e.level), "[%s]", Logger::LevelToString(e.level));
			g_Renderer->text_small(x + 130.0f, lineY, DT_LEFT, color::TextWhite, "%s", e.message.c_str());
		}
	}

	// Bottom info
	g_Renderer->text_small(x + 14.0f, y + h - 18.0f, DT_LEFT, color::TextMuted, "MCP Protocol: TCP JSON gui toi 127.0.0.1:13337 | Ho tro: ping, teleport, spawn_vehicle, scan_offsets, set_cheat");
}
