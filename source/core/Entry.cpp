#include "Globals.h"
#include "Config.h"
#include "render/Renderer.h"
#include "ui/Input.h"
#include "ui/Menu.h"
#include "features/FeatureManager.h"
#include "hooks/Hooks.h"
#include "utils/GameUtils.h"
#include "CAudioEngine.h"

using namespace plugin;

static CdeclEvent<AddressList<0x53C23A, H_JUMP>, PRIORITY_AFTER, ArgPickNone, void()> preRenderWaterEvent;
static bool s_isInitialized = false;
static float s_injectionBannerTimer = 0.0f;

static bool CheckIfGameAlreadyRunning()
{
	__try {
		if (*reinterpret_cast<void**>(0xC97C28) != nullptr || RsGlobal.maximumWidth > 0)
			return true;
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
	return false;
}

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

		// Support for DLL Injection while game is already running (Late / Runtime Injection)
		if (CheckIfGameAlreadyRunning())
		{
			on_init();
		}
	}

	~CheatMenuPlugin()
	{
		on_shutdown();
	}

	static void on_init()
	{
		if (s_isInitialized)
			return;
		s_isInitialized = true;

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

		s_injectionBannerTimer = 5.0f;
		__try {
			AudioEngine.ReportFrontendAudioEvent(1, 0.0f, 1.0f);
		} __except(EXCEPTION_EXECUTE_HANDLER) {}
	}

	static void on_shutdown()
	{
		if (!s_isInitialized)
			return;
		s_isInitialized = false;

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

		if (!s_isInitialized)
		{
			if (CheckIfGameAlreadyRunning())
				on_init();
			else
				return;
		}

		if (g_isPanic || utils::gta_menu_active())
			return;

		if (g_Features)
			g_Features->on_game_process();

		time_last = g_timeCurrent;
	}

	static void on_draw()
	{
		if (!s_isInitialized)
		{
			if (CheckIfGameAlreadyRunning())
				on_init();
			else
				return;
		}

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

			// Sleek HUD Toast Notification when injected
			if (s_injectionBannerTimer > 0.0f)
			{
				s_injectionBannerTimer -= static_cast<float>(g_timeDiff);
				int sw = 640, sh = 480;
				g_Renderer->get_screen_size(sw, sh);
				float bw = 460.0f, bh = 34.0f;
				float bx = (static_cast<float>(sw) - bw) * 0.5f;
				float by = 24.0f;

				float alphaFrac = (s_injectionBannerTimer < 1.0f) ? (std::max)(0.0f, s_injectionBannerTimer) : 1.0f;
				BYTE aBg = static_cast<BYTE>(230.0f * alphaFrac);
				BYTE aBorder = static_cast<BYTE>(255.0f * alphaFrac);

				g_Renderer->shadow(bx, by, bw, bh, 6.0f, 10, CRGBA(0, 0, 0, static_cast<BYTE>(120.0f * alphaFrac)));
				g_Renderer->bordered_rounded_rect(bx, by, bw, bh, 6.0f, CRGBA(10, 14, 22, aBg), CRGBA(0, 240, 255, aBorder), 1.2f);
				g_Renderer->filled_circle(bx + 16.0f, by + 17.0f, 4.0f, CRGBA(0, 255, 128, aBorder));
				g_Renderer->text(bx + 28.0f, by + 8.0f, DT_LEFT, CRGBA(0, 240, 255, aBorder), "CHEATMENU PRO v2.5 INJECTED");
				g_Renderer->text_small(bx + bw - 170.0f, by + 10.0f, DT_LEFT, CRGBA(255, 255, 255, aBorder), "Nhan [INSERT] de mo Menu");
			}

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

static HMODULE GetCurrentModuleHandle()
{
	HMODULE hMod = NULL;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&GetCurrentModuleHandle),
		&hMod
	);
	return hMod;
}

void UnloadCheatMenu()
{
	HMODULE hMod = GetCurrentModuleHandle();
	CheatMenuPlugin::on_shutdown();

	if (hMod)
	{
		CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
			Sleep(150);
			FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(param), 0);
			return 0;
		}, hMod, 0, nullptr);
	}
}
