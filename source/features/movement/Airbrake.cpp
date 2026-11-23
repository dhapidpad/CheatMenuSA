#include "Airbrake.h"
#include "GameUtils.h"
#include "MathUtils.h"
#include "Renderer.h"

Airbrake::Airbrake()
	: m_origPos(0.0f, 0.0f, 0.0f),
	  m_fallSpeedMult(1.0f),
	  m_wasEnabled(false),
	  m_timeStart(0)
{
}

void Airbrake::process(CPlayerPed* ped, double time_diff)
{
	if (!g_Config || !g_Config->movement.airbrake)
		return;

	// Toggle key (Right Shift)
	if (utils::key_pressed(RsKeyCodes::rsRSHIFT))
	{
		g_Config->movement.airbrake_active = !g_Config->movement.airbrake_active;
	}

	// Slowmo key (Numpad 0)
	if (g_Config->movement.airbrake_active && utils::key_pressed(RsKeyCodes::rsPADINS))
	{
		g_Config->movement.airbrake_slowmo = !g_Config->movement.airbrake_slowmo;
	}

	CPhysical* targetPhysical = ped;
	auto veh = FindPlayerVehicle(-1, false);
	if (veh)
		targetPhysical = veh;

	if (!m_wasEnabled && g_Config->movement.airbrake_active)
	{
		m_origPos = targetPhysical->m_matrix->GetPosition();
		m_fallSpeedMult = 1.0f;
		m_wasEnabled = true;
	}

	if (!g_Config->movement.airbrake_active)
	{
		m_wasEnabled = false;
		g_Config->movement.airbrake_slowmo = false;
		return;
	}

	auto matrix = targetPhysical->m_matrix;
	if (!matrix) return;

	// Parachute check
	bool hasParachute = (ped->m_nActiveWeaponSlot == 11 && ped->m_aWeapons[11].m_eWeaponType == WEAPON_PARACHUTE);

	if (!hasParachute)
	{
		matrix->pos = m_origPos;
		targetPhysical->m_vecMoveSpeed.Set(0.0f, 0.0f, 0.0f);

		ped->bIsStanding = true;
		ped->bWasStanding = true;
		ped->bStayInSamePlace = true;

		if (veh)
		{
			targetPhysical->m_vecTurnSpeed.Set(0.0f, 0.0f, 0.0f);
			targetPhysical->m_vecFrictionTurnSpeed.Set(0.0f, 0.0f, 0.0f);
		}

		CVector d(0.0f, 0.0f, 0.0f);
		float speed = g_Config->movement.airbrake_speed;
		float accel = static_cast<float>(time_diff) * speed;

		if (g_Config->movement.airbrake_slowmo)
			accel /= 10.0f;

		if (utils::key_down('W')) d.x += 1.0f;
		if (utils::key_down('S')) d.x -= 1.0f;
		if (utils::key_down('A')) d.y += 1.0f;
		if (utils::key_down('D')) d.y -= 1.0f;
		if (utils::key_down(RsKeyCodes::rsSPACE))  d.z += 1.0f;
		if (utils::key_down(RsKeyCodes::rsLSHIFT)) d.z -= 1.0f;

		if (!math::vect3_near_zero(d))
		{
			CVector moveVec(0.0f, 0.0f, 0.0f);

			if (g_Config->movement.airbrake_mode)
			{
				// 6-DOF camera relative flight
				CVector cam_fwd = TheCamera.GetForward();
				CVector cam_right = TheCamera.m_mCameraMatrix.right;
				moveVec = (cam_fwd * d.x) + (cam_right * (-d.y)) + (CVector(0.0f, 0.0f, 1.0f) * d.z);
				moveVec.Normalize();
			}
			else
			{
				// Classic vehicle/ped matrix relative
				CVector vect(-d.y, d.x, d.z);
				vect.Normalize();
				moveVec = math::matrix_vec4_mult(matrix, vect, 0.0f);
			}

			matrix->pos.x += moveVec.x * accel;
			matrix->pos.y += moveVec.y * accel;
			matrix->pos.z += moveVec.z * accel;
		}
	}
	else
	{
		if (utils::key_down(RsKeyCodes::rsSPACE))  m_fallSpeedMult += static_cast<float>(time_diff) / 2.0f;
		if (utils::key_down(RsKeyCodes::rsLSHIFT)) m_fallSpeedMult -= static_cast<float>(time_diff) / 2.0f;
		m_fallSpeedMult = std::clamp(m_fallSpeedMult, 0.0f, 2.0f);

		matrix->pos.x += targetPhysical->m_vecMoveSpeed.x * static_cast<float>(time_diff) * g_Config->movement.airbrake_speed;
		matrix->pos.y += targetPhysical->m_vecMoveSpeed.y * static_cast<float>(time_diff) * g_Config->movement.airbrake_speed;
		matrix->pos.z -= (matrix->pos.z - m_origPos.z) * m_fallSpeedMult;
	}

	m_origPos = targetPhysical->m_matrix->GetPosition();
}

void Airbrake::on_draw()
{
	if (!g_Config || !g_Config->movement.airbrake_active)
		return;

	CRGBA badgeCol = g_Config->movement.airbrake_slowmo ? color::WarningYellow : color::AccentCyan;
	const char* modeText = g_Config->movement.airbrake_slowmo ? "AIRBRAKE [SLOWMO]" : "AIRBRAKE [ACTIVE]";

	g_Renderer->bordered_rect(SCREEN_WIDTH - 150.0f, 12.0f, 140.0f, 22.0f, color::CardBg, badgeCol, 1.0f);
	g_Renderer->text_small(SCREEN_WIDTH - 80.0f, 16.0f, DT_CENTER, badgeCol, modeText);
}
