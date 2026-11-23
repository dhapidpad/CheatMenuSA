#include "Input.h"
#include "Config.h"

Input::Input()
	: m_mousePos(0.0f, 0.0f),
	  m_leftDown(false),
	  m_leftClicked(false),
	  m_leftReleased(false),
	  m_insertWasDown(false),
	  m_f11WasDown(false),
	  m_delWasDown(false)
{
}

void Input::update()
{
	// Mouse tracking via GetCursorPos & ScreenToClient
	POINT pt;
	GetCursorPos(&pt);
	if (RsGlobal.ps && RsGlobal.ps->window)
	{
		ScreenToClient(static_cast<HWND>(RsGlobal.ps->window), &pt);
	}
	m_mousePos.x = static_cast<float>(pt.x);
	m_mousePos.y = static_cast<float>(pt.y);

	// Mouse button states
	bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	m_leftClicked = lDown && !m_leftDown;
	m_leftReleased = !lDown && m_leftDown;
	m_leftDown = lDown;

	// Menu toggle (VK_INSERT or VK_F11)
	bool insertDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
	if (insertDown && !m_insertWasDown)
	{
		if (g_Config)
			g_Config->ui.menu_open = !g_Config->ui.menu_open;
	}
	m_insertWasDown = insertDown;

	bool f11Down = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
	if (f11Down && !m_f11WasDown)
	{
		if (g_Config)
			g_Config->ui.menu_open = !g_Config->ui.menu_open;
	}
	m_f11WasDown = f11Down;

	// Panic toggle (VK_DELETE) or Eject (SHIFT + VK_DELETE)
	bool delDown = (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;
	if (delDown && !m_delWasDown)
	{
		bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
		if (shiftDown)
		{
			UnloadCheatMenu();
			return;
		}
		else
		{
			g_isPanic = !g_isPanic;
		}
	}
	m_delWasDown = delDown;
}

bool Input::is_hovered(float x, float y, float w, float h) const
{
	return m_mousePos.x >= x && m_mousePos.x <= (x + w) &&
	       m_mousePos.y >= y && m_mousePos.y <= (y + h);
}

bool Input::is_clicked(float x, float y, float w, float h) const
{
	return m_leftClicked && is_hovered(x, y, w, h);
}
