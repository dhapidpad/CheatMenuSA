#pragma once

#include "Globals.h"

namespace color
{
	// Backgrounds
	inline const CRGBA BgDark          = CRGBA(15, 17, 26, 245);
	inline const CRGBA BgHeader        = CRGBA(22, 25, 38, 255);
	inline const CRGBA BgSidebar       = CRGBA(18, 20, 31, 255);
	inline const CRGBA CardBg          = CRGBA(24, 28, 44, 230);
	inline const CRGBA CardBorder      = CRGBA(45, 52, 78, 255);
	inline const CRGBA CardHover       = CRGBA(36, 42, 66, 255);

	// Accents
	inline const CRGBA AccentCyan      = CRGBA(0, 229, 255, 255);
	inline const CRGBA AccentBlue      = CRGBA(41, 121, 255, 255);
	inline const CRGBA AccentPurple    = CRGBA(124, 77, 255, 255);
	inline const CRGBA AccentGold      = CRGBA(255, 215, 64, 255);

	// Typography
	inline const CRGBA TextWhite       = CRGBA(255, 255, 255, 255);
	inline const CRGBA TextDim         = CRGBA(160, 170, 195, 255);
	inline const CRGBA TextMuted       = CRGBA(100, 110, 135, 255);

	// Feedback / Status
	inline const CRGBA SuccessGreen    = CRGBA(0, 230, 118, 255);
	inline const CRGBA WarningYellow   = CRGBA(255, 214, 0, 255);
	inline const CRGBA DangerRed       = CRGBA(255, 23, 68, 255);

	// Classic named colors for ESP & tags
	inline const CRGBA White           = CRGBA(255, 255, 255, 255);
	inline const CRGBA Black           = CRGBA(0, 0, 0, 255);
	inline const CRGBA Silver          = CRGBA(192, 192, 192, 255);
	inline const CRGBA Pink            = CRGBA(255, 105, 180, 255);
	inline const CRGBA Yellow          = CRGBA(255, 255, 0, 255);
	inline const CRGBA Orange          = CRGBA(255, 140, 0, 255);
	inline const CRGBA Cyan            = CRGBA(0, 255, 255, 255);
	inline const CRGBA MediumVioletRed = CRGBA(199, 21, 133, 255);
	inline const CRGBA DarkSeaGreen    = CRGBA(143, 188, 143, 255);
}

namespace palette = color;
