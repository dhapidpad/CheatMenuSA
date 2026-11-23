#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// base plugin-sdk headers
#include "plugin.h"
#include "CWorld.h"
#include "CPickups.h"
#include "CHud.h"
#include "CCollision.h"
#include "CCamera.h"
#include "CMenuManager.h"
#include "CModelInfo.h"
#include "CStreaming.h"
#include "CControllerConfigManager.h"
#include "CTaskSimpleFight.h"
#include "CTaskSimpleUseGun.h"
#include "CRadar.h"
#include "CTimer.h"
#include "CCarCtrl.h"
#include "CClock.h"
#include "CWeather.h"
#include "ePedBones.h"
#include "eWeaponType.h"
#include "eModelID.h"

// system & stl
#include <intrin.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <unordered_map>

// d3d9
#include <d3d9.h>
#include <d3dx9.h>

// safetyhook
#include "safetyhook.hpp"

// macro helpers
#define InRange(val, _min, _max) (val > _min && val < _max)
#define MSEC_TO_TIME(v) ((std::uint64_t)(v) * 10)
#define TIME_TO_FLOAT(v) ((float)((double)(v) / (double)MSEC_TO_TIME(1000)))
#define TIME_TO_DOUBLE(v) ((double)((double)(v) / (double)MSEC_TO_TIME(1000)))
#define FLOAT_EPSILON 0.0001f
#define DEFAULT_SCREEN_WIDTH 640
#define DEFAULT_SCREEN_HEIGHT 448

// plugin-sdk compatibility definitions
#ifndef WEAPON_MINIGUN
#define WEAPON_MINIGUN WEAPONTYPE_MINIGUN
#endif
#ifndef WEAPON_PARACHUTE
#define WEAPON_PARACHUTE WEAPONTYPE_PARACHUTE
#endif
#ifndef WEAPON_UNARMED
#define WEAPON_UNARMED WEAPONTYPE_UNARMED
#endif
#ifndef WEAPONTYPE_UNARMED
#define WEAPONTYPE_UNARMED WEAPON_UNARMED
#endif
#define m_nActiveWeaponSlot m_nSelectedWepSlot
#define m_nTotalAmmo m_nAmmoTotal
#define Normalise Normalize

template<typename T>
inline void SafeReset(T& obj)
{
	if (obj) obj.reset();
	obj = nullptr;
}

// forward declarations
class Renderer;
class Menu;
class Input;
class FeatureManager;
struct Config;

// global instances
extern std::shared_ptr<Renderer>       g_Renderer;
extern std::shared_ptr<Config>         g_Config;
extern std::shared_ptr<Menu>           g_Menu;
extern std::shared_ptr<Input>          g_Input;
extern std::shared_ptr<FeatureManager> g_Features;

// global states
extern bool          g_isSamp;
extern bool          g_isPanic;
extern std::uint64_t g_timeCurrent;
extern double        g_timeDiff;

// watermarks
extern std::vector<std::string> g_watermarks;
extern std::size_t              g_watermarkIndex;

// hooking
extern SafetyHookInline    g_hookGetWeaponInfo;
extern SafetyHookInline    g_hookHandleGunShot;
extern SafetyHookInline    g_hookFireInstantHitFromCar2;
extern SafetyHookInline    g_hookProcessPed;
extern SafetyHookInline    g_hookSetCursorPos;
extern RsInputEventHandler g_originalInputEventHandler;

// Injection lifecycle management
extern void UnloadCheatMenu();
