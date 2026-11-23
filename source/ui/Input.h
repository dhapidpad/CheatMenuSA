#pragma once

#include "Globals.h"

class Input
{
	CVector2D m_mousePos;
	bool      m_leftDown;
	bool      m_leftClicked;
	bool      m_leftReleased;

	bool      m_insertWasDown;
	bool      m_f11WasDown;
	bool      m_delWasDown;

public:
	Input();
	~Input() = default;

	void update();

	bool is_hovered(float x, float y, float w, float h) const;
	bool is_clicked(float x, float y, float w, float h) const;

	const CVector2D& mouse_pos() const { return m_mousePos; }
	bool mouse_down() const { return m_leftDown; }
	bool mouse_clicked() const { return m_leftClicked; }
	bool mouse_released() const { return m_leftReleased; }
};
