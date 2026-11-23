#include "WorldEsp.h"
#include "GameUtils.h"
#include "Renderer.h"

void WorldEsp::process_graffiti(CBuilding* building)
{
	static const std::unordered_map<unsigned short, std::pair<const char*, CRGBA>> tagInfo = {
		{ MODEL_TAG_01,     { "Ballas Tag",      color::MediumVioletRed } },
		{ MODEL_TAG_FRONT,  { "Front Yard Tag",  color::MediumVioletRed } },
		{ MODEL_TAG_KILO,   { "Kilo Tray Tag",   color::MediumVioletRed } },
		{ MODEL_TAG_RIFA,   { "San Fierro Rifa", color::AccentCyan } },
		{ MODEL_TAG_ROLLIN, { "Rollin Hts Tag",  color::MediumVioletRed } },
		{ MODEL_TAG_SEVILLE,{ "Seville Tag",     color::DarkSeaGreen } },
		{ MODEL_TAG_TEMPLE, { "Temple Drive",    color::MediumVioletRed } },
		{ MODEL_TAG_VAGOS,  { "Vagos Tag",       color::Orange } },
		{ MODEL_TAG_AZTECA, { "Aztecas Tag",     color::AccentCyan } }
	};

	auto model_index = building->m_nModelIndex;
	auto info = CModelInfo::GetModelInfo(model_index);
	if (!info || !info->IsTagModel())
		return;

	static auto CTagManager_GetAlpha = [](CEntity* entity) -> std::uint8_t {
		return plugin::CallAndReturn<std::uint8_t, 0x49CF90, CEntity*>(entity);
	};

	const std::uint8_t alpha = CTagManager_GetAlpha(building);
	if (alpha > 228) // already tagged
		return;

	CVector pos = building->GetPosition();
	CVector2D pos_s;
	if (!utils::world_to_screen(pos, pos_s))
		return;

	auto it = tagInfo.find(model_index);
	const char* name = (it != tagInfo.end()) ? it->second.first : "Graffiti Tag";
	CRGBA col = (it != tagInfo.end()) ? it->second.second : color::MediumVioletRed;

	g_Renderer->text_small(pos_s.x, pos_s.y, DT_CENTER, col, "[TAG] %s", name);
}

void WorldEsp::process_collectible(CPickup* pickup)
{
	static const std::unordered_map<unsigned short, const char*> pickupNames = {
		{ MODEL_CJ_OYSTER,     "Oyster" },
		{ MODEL_CJ_HORSE_SHOE, "Horseshoe" },
		{ MODEL_CAMERAPICKUP,  "Photo Snapshot" },
		{ MODEL_MINIGUN,       "Minigun Pickup" }
	};

	auto type = pickup->m_nPickupType;
	if (type != PICKUP_ONCE && type != PICKUP_ON_STREET_SLOW && type != PICKUP_SNAPSHOT)
		return;

	auto model_index = pickup->m_nModelIndex;
	auto it = pickupNames.find(model_index);
	if (it == pickupNames.end())
		return;

	if (model_index == MODEL_MINIGUN && pickup->m_nFlags.bDisabled)
		return;

	CVector pos = pickup->GetPosn();
	CVector2D pos_s;
	if (!utils::world_to_screen(pos, pos_s))
		return;

	g_Renderer->text_small(pos_s.x, pos_s.y, DT_CENTER, color::Pink, "[*] %s", it->second);
}

void WorldEsp::on_draw()
{
	if (!g_Config) return;

	if (g_Config->visuals.esp_Graffiti)
	{
		for (auto building : CPools::ms_pBuildingPool)
		{
			if (building)
				process_graffiti(building);
		}
	}

	if (g_Config->visuals.esp_Collectibles)
	{
		for (auto i = 0u; i <= MAX_NUM_PICKUPS; ++i)
		{
			auto pickup = &CPickups::aPickUps[i];
			if (pickup)
				process_collectible(pickup);
		}
	}
}
