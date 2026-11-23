#include "Globals.h"

std::shared_ptr<Renderer>       g_Renderer;
std::shared_ptr<Config>         g_Config;
std::shared_ptr<Menu>           g_Menu;
std::shared_ptr<Input>          g_Input;
std::shared_ptr<FeatureManager> g_Features;

bool          g_isSamp = false;
bool          g_isPanic = false;
std::uint64_t g_timeCurrent = 0;
double        g_timeDiff = 0.0;

std::vector<std::string> g_watermarks = {
	"CheatMenu SA v2.0 PRO | by rosko & antigravity",
	"GSO City Enhanced Edition",
	"Silent Aim & Vision Framework",
	"Internal ASI Framework"
};
std::size_t g_watermarkIndex = 0;

SafetyHookInline    g_hookGetWeaponInfo{};
SafetyHookInline    g_hookHandleGunShot{};
SafetyHookInline    g_hookFireInstantHitFromCar2{};
SafetyHookInline    g_hookProcessPed{};
SafetyHookInline    g_hookSetCursorPos{};
RsInputEventHandler g_originalInputEventHandler = nullptr;
