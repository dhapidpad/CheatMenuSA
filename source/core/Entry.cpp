#include "Globals.h"
#include "Config.h"
#include "render/Renderer.h"
#include "ui/Input.h"
#include "ui/Menu.h"
#include "features/FeatureManager.h"
#include "hooks/Hooks.h"
#include "utils/GameUtils.h"

using namespace plugin;

static CdeclEvent<AddressList<0x53C23A, H_JUMP>, PRIORITY_AFTER, ArgPickNone, void()> preRenderWaterEvent;

class CheatMenuPlugin
{
public:
	CheatMenuPlugin()
	{
		g_isSamp = (GetModuleHandleA("samp.dll") != nullptr);

		Events::initRwEvent.AddAfter(on_init);
		Events::shutdownRwEvent.AddBefore(on_shutdown);

		Events::d3dLostEvent.AddAfter(on_d3d_lost);
		Events::d3dResetEvent.AddAfter(on_d3d_reset);

		Events::drawHudEvent.AddAfter(on_draw_hud);

		if (g_isSamp)
		{
			preRenderWaterEvent.AddBefore(on_game_process);
			Events::drawAfterFadeEvent.AddAfter(on_draw);
		}
		else
		{
			Events::gameProcessEvent.AddAfter(on_game_process);
			Events::drawingEvent.AddAfter(on_draw);
		}
	}

	static void on_init()
	{
		g_Renderer = std::make_shared<Renderer>();
		g_Config   = std::make_shared<Config>();
		g_Input    = std::make_shared<Input>();
		g_Menu     = std::make_shared<Menu>();
		g_Features = std::make_shared<FeatureManager>();

		g_watermarks = {
			"CHEATMENU v2.0 PRO | GSOCity & SA-MP",
			"CHEATMENU [ULTRA MODERN] | Antigravity",
			"CHEATMENU [SILENT AIM & AIRBRAKE]",
			"CHEATMENU [PERFECT ACCURACY & ESP]"
		};

		if (!g_watermarks.empty())
		{
			g_watermarkIndex = plugin::RandomNumberInRange(0u, static_cast<unsigned int>(g_watermarks.size() - 1));
		}

		if (g_isSamp)
		{
			g_Config->combat.aim_Smart          = true;
			g_Config->combat.aim_Randomize      = true;
			g_Config->player.sp_InvPlayer       = false;
			g_Config->player.sp_InvCar          = false;
			g_Config->player.sp_NeverWanted     = false;
			g_Config->combat.aim_IgnoreGroove   = false;
			g_Config->player.sp_NoReload        = false;
		}

		hooks::install();
	}

	static void on_shutdown()
	{
		hooks::uninstall();

		SafeReset(g_Features);
		SafeReset(g_Menu);
		SafeReset(g_Input);
		SafeReset(g_Config);
		SafeReset(g_Renderer);
	}

	static void on_game_process()
	{
		static std::uint64_t time_last = 0;
		g_timeCurrent = utils::time_get();
		g_timeDiff    = TIME_TO_DOUBLE(g_timeCurrent - time_last);

		if (g_isPanic || utils::gta_menu_active())
			return;

		if (g_Features)
			g_Features->on_game_process();

		time_last = g_timeCurrent;
	}

	static void on_draw()
	{
		if (g_isPanic || utils::gta_menu_active())
			return;

		if (g_Input)
			g_Input->update();

		if (g_Renderer)
		{
			g_Renderer->on_draw_start();

			if (g_Features)
				g_Features->on_draw();

			if (g_Menu)
				g_Menu->on_draw();

			g_Renderer->on_draw_end();
		}
	}

	static void on_draw_hud()
	{
		if (g_isPanic || utils::gta_menu_active())
			return;

		if (g_Renderer && g_Features)
		{
			g_Renderer->on_draw_start();
			g_Features->on_draw_hud();
			g_Renderer->on_draw_end();
		}
	}

	static void on_d3d_lost()
	{
		if (g_Renderer)
			g_Renderer->on_lost();
	}

	static void on_d3d_reset()
	{
		if (g_Renderer)
			g_Renderer->on_reset();
	}
} s_CheatMenuPluginInstance;
