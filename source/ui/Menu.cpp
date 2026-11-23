#include "Menu.h"
#include "GameUtils.h"
#include "Math.h"

Menu::Menu()
	: m_isDragging(false),
	  m_dragOffset(0.0f, 0.0f),
	  m_activeTooltip(""),
	  m_statusMessage("Ready"),
	  m_statusTimer(0.0f)
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
	g_Renderer->filled_rect(x + 4.0f, y + 4.0f, w, h, CRGBA(0, 0, 0, 100));
	g_Renderer->bordered_rect(x, y, w, h, color::BgDark, color::CardBorder, 1.0f);

	// Top accent line
	g_Renderer->gradient_horizontal(x, y, w, 2.0f, color::AccentCyan, color::AccentBlue);

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
	g_Renderer->bordered_rect(x + 250.0f, y + 9.0f, 52.0f, 18.0f, color::CardBg, color::AccentCyan, 1.0f);
	g_Renderer->text_small(x + 258.0f, y + 11.0f, DT_LEFT, color::AccentCyan, "v2.0 PRO");

	// Status indicator on right
	const char* sampStatus = g_isSamp ? "SA-MP" : "SINGLEPLAYER";
	g_Renderer->text_small(x + w - 160.0f, y + 11.0f, DT_LEFT, color::TextMuted, "MODE: %s", sampStatus);

	// FPS Badge
	g_Renderer->text_small(x + w - 50.0f, y + 11.0f, DT_LEFT, color::SuccessGreen, "%d FPS", static_cast<int>(CTimer::game_FPS));
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
		"[$] ACTIONS"
	};

	const float tabH = 38.0f;
	for (int i = 0; i < 5; ++i)
	{
		float tabY = y + 10.0f + (i * (tabH + 4.0f));
		bool isCurrent = (g_Config->ui.active_tab == i);
		bool hovered = g_Input->is_hovered(x + 6.0f, tabY, w - 12.0f, tabH);

		if (hovered && g_Input->mouse_clicked())
		{
			g_Config->ui.active_tab = i;
			isCurrent = true;
		}

		if (isCurrent)
		{
			// Active tab highlight
			g_Renderer->bordered_rect(x + 6.0f, tabY, w - 12.0f, tabH, color::CardBg, color::CardBorder, 1.0f);
			g_Renderer->filled_rect(x + 6.0f, tabY, 3.0f, tabH, color::AccentCyan);
			g_Renderer->text(x + 18.0f, tabY + 11.0f, DT_LEFT, color::AccentCyan, tabNames[i]);
		}
		else
		{
			if (hovered)
			{
				g_Renderer->filled_rect(x + 6.0f, tabY, w - 12.0f, tabH, color::CardBg);
				g_Renderer->text(x + 18.0f, tabY + 11.0f, DT_LEFT, color::TextWhite, tabNames[i]);
			}
			else
			{
				g_Renderer->text(x + 18.0f, tabY + 11.0f, DT_LEFT, color::TextMuted, tabNames[i]);
			}
		}
	}

	// Hotkey reminder in sidebar bottom
	g_Renderer->text_small(x + 14.0f, y + h - 50.0f, DT_LEFT, color::TextMuted, "TOGGLE: INSERT / F11");
	g_Renderer->text_small(x + 14.0f, y + h - 35.0f, DT_LEFT, color::TextMuted, "PANIC:  DELETE");
	g_Renderer->text_small(x + 14.0f, y + h - 20.0f, DT_LEFT, color::TextMuted, "AIRBRAKE: RSHIFT");
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
		g_Renderer->text_small(x + 14.0f, y + 6.0f, DT_LEFT, color::AccentCyan, "> %s", m_activeTooltip.c_str());
	}
	else if (m_statusTimer > 0.0f)
	{
		g_Renderer->text_small(x + 14.0f, y + 6.0f, DT_LEFT, color::SuccessGreen, "[!] %s", m_statusMessage.c_str());
	}
	else
	{
		g_Renderer->text_small(x + 14.0f, y + 6.0f, DT_LEFT, color::TextMuted, "Tip: Hover over any option to view details.");
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
	g_Renderer->text(x, y, DT_LEFT, color::AccentCyan, title);
	g_Renderer->line(x, y + 18.0f, x + w, y + 18.0f, color::CardBorder);
}

bool Menu::checkbox(float x, float y, const char* label, bool& val, const char* tooltip)
{
	const float boxSize = 16.0f;
	const float totalW = 200.0f;
	bool hovered = g_Input->is_hovered(x, y, totalW, boxSize);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	if (hovered && g_Input->mouse_clicked())
		val = !val;

	// Checkbox square
	g_Renderer->bordered_rect(x, y, boxSize, boxSize, color::CardBg, hovered ? color::AccentCyan : color::CardBorder, 1.0f);
	if (val)
	{
		g_Renderer->filled_rect(x + 3.0f, y + 3.0f, boxSize - 6.0f, boxSize - 6.0f, color::AccentCyan);
	}

	// Label
	g_Renderer->text_small(x + boxSize + 8.0f, y + 2.0f, DT_LEFT, val ? color::TextWhite : (hovered ? color::TextWhite : color::TextDim), label);
	return val;
}

bool Menu::slider_float(float x, float y, float w, const char* label, float& val, float min, float max, const char* format, const char* tooltip)
{
	const float sliderH = 8.0f;
	const float totalH = 28.0f;
	bool hovered = g_Input->is_hovered(x, y, w, totalH);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	// Label and formatted value
	char valBuf[32];
	sprintf_s(valBuf, format, val);
	g_Renderer->text_small(x, y, DT_LEFT, hovered ? color::TextWhite : color::TextDim, label);
	g_Renderer->text_small(x + w, y, DT_RIGHT, color::AccentCyan, valBuf);

	float trackY = y + 16.0f;
	bool trackHovered = g_Input->is_hovered(x, trackY - 2.0f, w, sliderH + 4.0f);

	if (trackHovered && g_Input->mouse_down())
	{
		float mouseOffset = std::clamp(g_Input->mouse_pos().x - x, 0.0f, w);
		float ratio = mouseOffset / w;
		val = min + ratio * (max - min);
	}

	// Slider track
	g_Renderer->bordered_rect(x, trackY, w, sliderH, color::CardBg, color::CardBorder, 1.0f);

	// Filled progress
	float ratio = std::clamp((val - min) / (max - min), 0.0f, 1.0f);
	if (ratio > 0.0f)
	{
		g_Renderer->gradient_horizontal(x, trackY, w * ratio, sliderH, color::AccentCyan, color::AccentBlue);
	}

	return true;
}

bool Menu::combobox(float x, float y, float w, const char* label, int& val, const std::vector<std::string>& items, const char* tooltip)
{
	const float boxH = 22.0f;
	const float totalH = 38.0f;
	bool hovered = g_Input->is_hovered(x, y, w, totalH);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	g_Renderer->text_small(x, y, DT_LEFT, color::TextDim, label);

	float boxY = y + 16.0f;
	g_Renderer->bordered_rect(x, boxY, w, boxH, color::CardBg, color::CardBorder, 1.0f);

	if (val >= 0 && val < static_cast<int>(items.size()))
	{
		g_Renderer->text_small(x + 10.0f, boxY + 4.0f, DT_LEFT, color::AccentCyan, items[val].c_str());
	}

	// Left & right cycle arrows
	bool leftClicked = g_Input->is_clicked(x + w - 36.0f, boxY, 16.0f, boxH);
	bool rightClicked = g_Input->is_clicked(x + w - 18.0f, boxY, 16.0f, boxH);

	g_Renderer->text_small(x + w - 32.0f, boxY + 4.0f, DT_LEFT, color::TextWhite, "<");
	g_Renderer->text_small(x + w - 16.0f, boxY + 4.0f, DT_LEFT, color::TextWhite, ">");

	if (leftClicked)
	{
		val = (val - 1 + static_cast<int>(items.size())) % static_cast<int>(items.size());
	}
	else if (rightClicked)
	{
		val = (val + 1) % static_cast<int>(items.size());
	}

	return true;
}

bool Menu::button(float x, float y, float w, float h, const char* label, const CRGBA& accentColor, const char* tooltip)
{
	bool hovered = g_Input->is_hovered(x, y, w, h);
	bool clicked = g_Input->is_clicked(x, y, w, h);

	if (hovered && tooltip)
		m_activeTooltip = tooltip;

	CRGBA bg = hovered ? color::CardHover : color::CardBg;
	CRGBA border = hovered ? accentColor : color::CardBorder;

	g_Renderer->bordered_rect(x, y, w, h, bg, border, 1.0f);
	g_Renderer->text(x + (w / 2.0f), y + (h / 2.0f) - 6.0f, DT_CENTER, hovered ? accentColor : color::TextWhite, label);

	return clicked;
}

// ---------------- TAB 0: COMBAT ----------------
void Menu::tab_combat(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Silent Aim Settings
	section_header(col1_x, y + 14.0f, col_w, "SILENT AIMBOT");
	checkbox(col1_x, y + 42.0f, "Enable Silent Aim", g_Config->combat.aim_Enabled, "Redirects weapon bullets toward nearest target.");
	checkbox(col1_x, y + 68.0f, "Target Visible Only", g_Config->combat.aim_TargetVisible, "Only aims at targets with clear line of sight.");
	checkbox(col1_x, y + 94.0f, "Target On-Screen Only", g_Config->combat.aim_TargetOnScreen, "Targets must be within camera field of view.");
	checkbox(col1_x, y + 120.0f, "Draw FOV Circle", g_Config->combat.aim_DrawFov, "Draws visual aimbot radius circle on screen.");
	slider_float(col1_x, y + 150.0f, col_w, "Aimbot FOV Radius", g_Config->combat.aim_SmartFov, 20.0f, 400.0f, "%.0f px", "Maximum pixel distance from crosshair to target.");

	// Column 2: Targeting & Accuracy
	section_header(col2_x, y + 14.0f, col_w, "TARGETING & ACCURACY");
	checkbox(col2_x, y + 42.0f, "Smart Bone Selection", g_Config->combat.aim_Smart, "Chooses closest visible bone to crosshair.");
	checkbox(col2_x, y + 68.0f, "Prioritize Enemy Gangs", g_Config->combat.aim_GangsPriority, "Attacks enemy gang members first.");
	checkbox(col2_x, y + 94.0f, "Prioritize Cops", g_Config->combat.aim_CopsPriority, "Attacks police officers first.");
	checkbox(col2_x, y + 120.0f, "Ignore Grove Street", g_Config->combat.aim_IgnoreGroove, "Will not target friendly Grove Street allies.");
	checkbox(col2_x, y + 146.0f, "Randomize Hit Spot", g_Config->combat.aim_Randomize, "Adds subtle humanized deviation to hits.");
	slider_float(col2_x, y + 174.0f, col_w, "Randomize Factor", g_Config->combat.aim_RandomizeFactor, 0.05f, 0.80f, "%.2f", "Spread factor for hit randomization.");
	checkbox(col2_x, y + 214.0f, "Perfect Accuracy & Range", g_Config->combat.aim_PerfectAccuracy, "Eliminates bullet spread and extends range.");
}

// ---------------- TAB 1: VISUALS ----------------
void Menu::tab_visuals(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Player ESP
	section_header(col1_x, y + 14.0f, col_w, "PLAYER ESP");
	checkbox(col1_x, y + 42.0f, "Master ESP Switch", g_Config->visuals.esp_Enabled, "Toggles all player ESP features on or off.");

	std::vector<std::string> boxTypes = { "Off", "Full 2D Box", "Corner Box" };
	combobox(col1_x, y + 68.0f, col_w, "Bounding Box Style", g_Config->visuals.esp_BoxType, boxTypes, "Select 2D bounding box visual style.");

	checkbox(col1_x, y + 116.0f, "Health Bar (Gradient)", g_Config->visuals.esp_Health, "Draws smooth dynamic health bar.");
	checkbox(col1_x, y + 142.0f, "Armor Bar", g_Config->visuals.esp_Armor, "Draws silver armor bar below health.");
	checkbox(col1_x, y + 168.0f, "Distance Indicator", g_Config->visuals.esp_Distance, "Shows distance in meters to target ped.");
	checkbox(col1_x, y + 194.0f, "Highlight Aim Target", g_Config->visuals.esp_AimTarget, "Highlights current aimbot target in red/glow.");
	checkbox(col1_x, y + 220.0f, "Draw Snaplines", g_Config->visuals.esp_Snapline, "Draws tracer line from screen bottom to peds.");

	// Column 2: World & Screen Overlays
	section_header(col2_x, y + 14.0f, col_w, "WORLD & SCREEN OVERLAYS");
	checkbox(col2_x, y + 42.0f, "Custom Crosshair", g_Config->visuals.esp_Crosshair, "Draws crisp precision crosshair at weapon aim point.");
	checkbox(col2_x, y + 68.0f, "Mini-Radar Ped Blips", g_Config->visuals.esp_Radar, "Displays nearby peds on the native in-game radar.");
	checkbox(col2_x, y + 94.0f, "Unfinished Graffiti ESP", g_Config->visuals.esp_Graffiti, "Marks uncompleted gang tags in the game world.");
	checkbox(col2_x, y + 120.0f, "Collectibles ESP", g_Config->visuals.esp_Collectibles, "Shows oysters, horseshoes, and photo snapshots.");
}

// ---------------- TAB 2: MOVEMENT ----------------
void Menu::tab_movement(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Airbrake (Fly Mode)
	section_header(col1_x, y + 14.0f, col_w, "AIRBRAKE (FREE FLY)");
	checkbox(col1_x, y + 42.0f, "Enable Airbrake", g_Config->movement.airbrake, "Allows free flight through air and solid objects.");
	checkbox(col1_x, y + 68.0f, "6-DOF Camera Direction", g_Config->movement.airbrake_mode, "Moves directly toward wherever the camera looks.");
	slider_float(col1_x, y + 96.0f, col_w, "Flight Speed", g_Config->movement.airbrake_speed, 40.0f, 500.0f, "%.0f", "Movement speed while airbrake is active.");

	// Hotkey guide card
	g_Renderer->bordered_rect(col1_x, y + 145.0f, col_w, 90.0f, color::CardBg, color::CardBorder, 1.0f);
	g_Renderer->text_small(col1_x + 10.0f, y + 152.0f, DT_LEFT, color::AccentCyan, "AIRBRAKE CONTROLS:");
	g_Renderer->text_small(col1_x + 10.0f, y + 168.0f, DT_LEFT, color::TextWhite, "RSHIFT: Toggle Flight On/Off");
	g_Renderer->text_small(col1_x + 10.0f, y + 184.0f, DT_LEFT, color::TextWhite, "W / S / A / D: Move Forward/Back/Sides");
	g_Renderer->text_small(col1_x + 10.0f, y + 200.0f, DT_LEFT, color::TextWhite, "SPACE: Fly Up  |  LSHIFT: Fly Down");
	g_Renderer->text_small(col1_x + 10.0f, y + 216.0f, DT_LEFT, color::WarningYellow, "NUMPAD0: Slow-Motion Precision Mode");

	// Column 2: Speedhack & Physics
	section_header(col2_x, y + 14.0f, col_w, "SPEEDHACK & ACCELERATION");
	checkbox(col2_x, y + 42.0f, "Enable Speedhack", g_Config->movement.speedhack, "Multiplies on-foot running and walking velocity.");
	slider_float(col2_x, y + 70.0f, col_w, "Velocity Multiplier", g_Config->movement.speedhack_mult, 1.5f, 6.0f, "%.1fx", "Speed multiplier applied to player movement.");
}

// ---------------- TAB 3: PLAYER & VEHICLE ----------------
void Menu::tab_player(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Player Cheats
	section_header(col1_x, y + 14.0f, col_w, "PLAYER GODMODE & ATTRIBUTES");
	checkbox(col1_x, y + 42.0f, "Godmode (Infinite Health/Armor)", g_Config->player.sp_InvPlayer, "Full invulnerability to bullets, explosions, and falls.");
	checkbox(col1_x, y + 68.0f, "Never Wanted (0 Stars)", g_Config->player.sp_NeverWanted, "Freezes police wanted level at 0.");
	checkbox(col1_x, y + 94.0f, "Infinite Ammo / No Reload", g_Config->player.sp_NoReload, "Weapon magazine never empties.");
	checkbox(col1_x, y + 120.0f, "Minigun Firing Anims", g_Config->player.sp_MinigunMode, "Dual-handed minigun firing posture for all weapons.");
	checkbox(col1_x, y + 146.0f, "Leg Slide Glitch", g_Config->player.sp_LegSlide, "Smooth ice-skating animation glitch.");
	checkbox(col1_x, y + 172.0f, "Telekinesis Pull", g_Config->player.sp_Telekinesis, "Levitates and freezes target ped directly in front of you.");
	checkbox(col1_x, y + 198.0f, "Dumbass Peds", g_Config->player.sp_DumbassPeds, "Clears ped AI tasks, freezing everyone in place.");

	// Column 2: Vehicle Cheats
	section_header(col2_x, y + 14.0f, col_w, "VEHICLE ENHANCEMENTS");
	checkbox(col2_x, y + 42.0f, "Vehicle Godmode", g_Config->player.sp_InvCar, "Player vehicle takes zero damage and auto-repairs.");

	if (button(col2_x, y + 74.0f, col_w, 28.0f, "Repair & Fix Vehicle", color::AccentCyan, "Instantly repairs engine and body of current vehicle."))
	{
		utils::repair_vehicle();
		set_status("Vehicle repaired and restored to full health!");
	}

	if (button(col2_x, y + 110.0f, col_w, 28.0f, "Flip / Upright Vehicle", color::AccentCyan, "Restores flipped or rolled vehicle to upright position."))
	{
		utils::flip_vehicle();
		set_status("Vehicle flipped upright!");
	}

	if (button(col2_x, y + 146.0f, col_w, 28.0f, "Trigger 6-Star Wanted", color::WarningYellow, "Instantly triggers maximum 6-star military response."))
	{
		g_Config->player.sp_RussianMostWanted = true;
		set_status("6-Star wanted level triggered!");
	}
}

// ---------------- TAB 4: TELEPORT & ACTIONS ----------------
void Menu::tab_actions(float x, float y, float w, float h)
{
	float col1_x = x + 16.0f;
	float col2_x = x + (w / 2.0f) + 8.0f;
	float col_w = (w / 2.0f) - 24.0f;

	// Column 1: Map Teleport
	section_header(col1_x, y + 14.0f, col_w, "INSTANT TELEPORTATION");

	if (button(col1_x, y + 44.0f, col_w, 36.0f, ">> TELEPORT TO MAP MARKER <<", color::AccentCyan, "Teleports player (or vehicle) directly to map waypoint."))
	{
		if (utils::teleport_to_marker())
			set_status("Teleported successfully to marker position!");
		else
			set_status("No active map marker found! Place a waypoint on map first.");
	}

	g_Renderer->text_small(col1_x, y + 90.0f, DT_LEFT, color::TextMuted, "How to use: Open pause map (ESC -> MAP),");
	g_Renderer->text_small(col1_x, y + 104.0f, DT_LEFT, color::TextMuted, "Right-click anywhere to place a red target blip,");
	g_Renderer->text_small(col1_x, y + 118.0f, DT_LEFT, color::TextMuted, "then click the teleport button above.");

	// Column 2: Instant Action Buttons
	section_header(col2_x, y + 14.0f, col_w, "GAME ACTIONS & WEAPONS");

	if (button(col2_x, y + 42.0f, col_w, 28.0f, "Give Ammo (+250)", color::AccentCyan, "Gives 250 bullets to current weapon."))
	{
		utils::give_ammo();
		set_status("Granted 250 ammo to held weapon!");
	}

	if (button(col2_x, y + 76.0f, col_w, 28.0f, "Give Cash (+$500)", color::SuccessGreen, "Spawns and awards $500 cash."))
	{
		utils::give_money();
		set_status("Granted $500 cash to CJ!");
	}

	if (button(col2_x, y + 110.0f, col_w, 28.0f, "Spawn Heavy Minigun", color::AccentBlue, "Loads and grants minigun with 500 rounds."))
	{
		utils::give_minigun();
		set_status("Minigun loaded and equipped!");
	}

	if (button(col2_x, y + 144.0f, col_w, 28.0f, "Collect All Oysters / Tags", color::AccentCyan, "Paints all 100 graffiti tags and grabs collectibles."))
	{
		utils::collect_collectibles();
		set_status("All graffiti tags sprayed and collectibles retrieved!");
	}

	if (button(col2_x, y + 178.0f, col_w, 28.0f, "Unlock All Nearby Vehicles", color::AccentCyan, "Sets all nearby locked vehicles to unlocked."))
	{
		utils::unlock_nearby_cars();
		set_status("All nearby vehicles unlocked!");
	}

	if (button(col2_x, y + 212.0f, col_w, 28.0f, "PANIC SWITCH (HIDE ALL)", color::DangerRed, "Instantly disables cheats and hides menu (DELETE key)."))
	{
		g_isPanic = true;
		g_Config->ui.menu_open = false;
	}
}
