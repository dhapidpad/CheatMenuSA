#include "Menu.h"
#include "GameUtils.h"
#include "MathUtils.h"

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
		"[$] ACTIONS & SPAWN"
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
			g_Renderer->text(x + 14.0f, tabY + 11.0f, DT_LEFT, color::AccentCyan, tabNames[i]);
		}
		else
		{
			if (hovered)
			{
				g_Renderer->filled_rect(x + 6.0f, tabY, w - 12.0f, tabH, color::CardBg);
				g_Renderer->text(x + 14.0f, tabY + 11.0f, DT_LEFT, color::TextWhite, tabNames[i]);
			}
			else
			{
				g_Renderer->text(x + 14.0f, tabY + 11.0f, DT_LEFT, color::TextMuted, tabNames[i]);
			}
		}
	}

	// Hotkey reminder in sidebar bottom
	g_Renderer->text_small(x + 10.0f, y + h - 50.0f, DT_LEFT, color::TextMuted, "TOGGLE: INSERT / F11");
	g_Renderer->text_small(x + 10.0f, y + h - 35.0f, DT_LEFT, color::TextMuted, "PANIC:  DELETE");
	g_Renderer->text_small(x + 10.0f, y + h - 20.0f, DT_LEFT, color::TextMuted, "AIRBRAKE: RSHIFT");
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
		g_Renderer->text_small(x + w - 130.0f, y + 6.0f, DT_LEFT, color::TextMuted, g_watermarks[g_watermarkIndex].c_str());
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
