#pragma once

#include "Globals.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

// ============================================================================
//  Colors.h - Bảng màu / theme tập trung
//  Quy ước: mọi màu là CRGBA(r, g, b, a) với 0..255.
//  Lưu ý: các helper bên dưới giả định CRGBA có thành viên r, g, b, a.
//         Nếu struct của bạn đặt tên khác (R,G,B,A / red...), sửa ở khối
//         "Accessors" duy nhất bên dưới là đủ.
// ============================================================================

namespace color
{
	// ------------------------------------------------------------------
	// Backgrounds (giữ nguyên giá trị gốc)
	// ------------------------------------------------------------------
	inline const CRGBA BgDark          = CRGBA(15, 17, 26, 245);
	inline const CRGBA BgHeader        = CRGBA(22, 25, 38, 255);
	inline const CRGBA BgSidebar       = CRGBA(18, 20, 31, 255);
	inline const CRGBA CardBg          = CRGBA(24, 28, 44, 230);
	inline const CRGBA CardBorder      = CRGBA(45, 52, 78, 255);
	inline const CRGBA CardHover       = CRGBA(36, 42, 66, 255);

	// Lớp nền bổ sung (độ sâu / elevation)
	inline const CRGBA BgDeep          = CRGBA(10, 11, 18, 255);   // nền sâu nhất
	inline const CRGBA BgElevated      = CRGBA(30, 34, 54, 255);   // popup, dropdown
	inline const CRGBA CardActive      = CRGBA(44, 52, 82, 255);   // đang nhấn / chọn
	inline const CRGBA Divider         = CRGBA(38, 44, 66, 255);   // đường kẻ mảnh
	inline const CRGBA Overlay         = CRGBA(0, 0, 0, 140);      // phủ mờ modal
	inline const CRGBA Shadow          = CRGBA(0, 0, 0, 90);       // bóng đổ

	// ------------------------------------------------------------------
	// Accents (giữ nguyên) + biến thể
	// ------------------------------------------------------------------
	inline const CRGBA AccentCyan      = CRGBA(0, 229, 255, 255);
	inline const CRGBA AccentBlue      = CRGBA(41, 121, 255, 255);
	inline const CRGBA AccentPurple    = CRGBA(124, 77, 255, 255);
	inline const CRGBA AccentGold      = CRGBA(255, 215, 64, 255);

	inline const CRGBA AccentCyanDim   = CRGBA(0, 150, 168, 255);
	inline const CRGBA AccentBlueDim   = CRGBA(28, 84, 178, 255);
	inline const CRGBA AccentPurpleDim = CRGBA(84, 52, 176, 255);
	inline const CRGBA AccentPink      = CRGBA(255, 64, 129, 255);
	inline const CRGBA AccentGlowCyan  = CRGBA(0, 229, 255, 60);   // viền phát sáng
	inline const CRGBA AccentGlowBlue  = CRGBA(41, 121, 255, 60);

	// ------------------------------------------------------------------
	// Typography (giữ nguyên)
	// ------------------------------------------------------------------
	inline const CRGBA TextWhite       = CRGBA(255, 255, 255, 255);
	inline const CRGBA TextDim         = CRGBA(160, 170, 195, 255);
	inline const CRGBA TextMuted       = CRGBA(100, 110, 135, 255);
	inline const CRGBA TextDisabled    = CRGBA(70, 78, 100, 255);
	inline const CRGBA TextOnAccent    = CRGBA(8, 10, 18, 255);    // chữ trên nền accent sáng

	// ------------------------------------------------------------------
	// Feedback / Status (giữ nguyên) + biến thể
	// ------------------------------------------------------------------
	inline const CRGBA SuccessGreen    = CRGBA(0, 230, 118, 255);
	inline const CRGBA WarningYellow   = CRGBA(255, 214, 0, 255);
	inline const CRGBA DangerRed       = CRGBA(255, 23, 68, 255);
	inline const CRGBA InfoBlue        = CRGBA(64, 156, 255, 255);

	inline const CRGBA SuccessBg       = CRGBA(0, 230, 118, 28);
	inline const CRGBA WarningBg       = CRGBA(255, 214, 0, 28);
	inline const CRGBA DangerBg        = CRGBA(255, 23, 68, 28);
	inline const CRGBA InfoBg          = CRGBA(64, 156, 255, 28);

	// ------------------------------------------------------------------
	// Classic named colors (giữ nguyên)
	// ------------------------------------------------------------------
	inline const CRGBA White           = CRGBA(255, 255, 255, 255);
	inline const CRGBA Black           = CRGBA(0, 0, 0, 255);
	inline const CRGBA Silver          = CRGBA(192, 192, 192, 255);
	inline const CRGBA Pink            = CRGBA(255, 105, 180, 255);
	inline const CRGBA Yellow          = CRGBA(255, 255, 0, 255);
	inline const CRGBA Orange          = CRGBA(255, 140, 0, 255);
	inline const CRGBA Cyan            = CRGBA(0, 255, 255, 255);
	inline const CRGBA MediumVioletRed = CRGBA(199, 21, 133, 255);
	inline const CRGBA DarkSeaGreen    = CRGBA(143, 188, 143, 255);

	inline const CRGBA Red             = CRGBA(255, 0, 0, 255);
	inline const CRGBA Green           = CRGBA(0, 255, 0, 255);
	inline const CRGBA Blue            = CRGBA(0, 0, 255, 255);
	inline const CRGBA Magenta         = CRGBA(255, 0, 255, 255);
	inline const CRGBA Gray            = CRGBA(128, 128, 128, 255);
	inline const CRGBA Transparent     = CRGBA(0, 0, 0, 0);

	// ========================================================================
	// Accessors (chỉ sửa tại đây nếu CRGBA dùng tên thành viên khác)
	// ========================================================================
	namespace detail
	{
		inline uint8_t R(const CRGBA& c) { return static_cast<uint8_t>(c.r); }
		inline uint8_t G(const CRGBA& c) { return static_cast<uint8_t>(c.g); }
		inline uint8_t B(const CRGBA& c) { return static_cast<uint8_t>(c.b); }
		inline uint8_t A(const CRGBA& c) { return static_cast<uint8_t>(c.a); }

		inline uint8_t Clamp8(float v)
		{
			return static_cast<uint8_t>(std::clamp(v, 0.0f, 255.0f) + 0.5f);
		}
	}

	// ========================================================================
	// Helpers
	// ========================================================================

	// Đổi alpha (0..255), giữ nguyên RGB.
	inline CRGBA WithAlpha(const CRGBA& c, uint8_t alpha)
	{
		return CRGBA(detail::R(c), detail::G(c), detail::B(c), alpha);
	}

	// Đổi alpha theo tỉ lệ 0..1.
	inline CRGBA WithAlphaF(const CRGBA& c, float a01)
	{
		return WithAlpha(c, detail::Clamp8(a01 * 255.0f));
	}

	// Nhân alpha hiện tại với hệ số (dùng cho fade in/out).
	inline CRGBA MulAlpha(const CRGBA& c, float k)
	{
		return WithAlpha(c, detail::Clamp8(detail::A(c) * k));
	}

	// Nội suy tuyến tính giữa 2 màu, t = 0..1.
	inline CRGBA Lerp(const CRGBA& a, const CRGBA& b, float t)
	{
		t = std::clamp(t, 0.0f, 1.0f);
		auto mix = [t](uint8_t x, uint8_t y) { return detail::Clamp8(x + (y - x) * t); };
		return CRGBA(mix(detail::R(a), detail::R(b)),
		             mix(detail::G(a), detail::G(b)),
		             mix(detail::B(a), detail::B(b)),
		             mix(detail::A(a), detail::A(b)));
	}

	// Làm sáng / tối: amount 0..1 (trộn về trắng / đen).
	inline CRGBA Lighten(const CRGBA& c, float amount) { return Lerp(c, CRGBA(255, 255, 255, detail::A(c)), amount); }
	inline CRGBA Darken (const CRGBA& c, float amount) { return Lerp(c, CRGBA(0, 0, 0, detail::A(c)), amount); }

	// Từ mã hex: 0xRRGGBB hoặc 0xRRGGBBAA (hasAlpha = true).
	inline CRGBA FromHex(uint32_t hex, bool hasAlpha = false)
	{
		if (hasAlpha)
			return CRGBA((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
		return CRGBA((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255);
	}

	// Từ HSV: h 0..360, s 0..1, v 0..1.
	inline CRGBA FromHSV(float h, float s, float v, uint8_t a = 255)
	{
		h = std::fmod(h, 360.0f);
		if (h < 0.0f) h += 360.0f;
		const float c  = v * s;
		const float x  = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
		const float m  = v - c;
		float r = 0, g = 0, b = 0;
		if      (h < 60)  { r = c; g = x; }
		else if (h < 120) { r = x; g = c; }
		else if (h < 180) { g = c; b = x; }
		else if (h < 240) { g = x; b = c; }
		else if (h < 300) { r = x; b = c; }
		else              { r = c; b = x; }
		return CRGBA(detail::Clamp8((r + m) * 255.0f),
		             detail::Clamp8((g + m) * 255.0f),
		             detail::Clamp8((b + m) * 255.0f), a);
	}

	// Cầu vồng theo thời gian (giây), speed = vòng/giây.
	inline CRGBA Rainbow(float timeSec, float speed = 0.25f, float s = 0.85f, float v = 1.0f)
	{
		return FromHSV(timeSec * speed * 360.0f, s, v);
	}

	// Gradient 2 màu động (qua lại mượt) – hợp cho viền / thanh accent.
	inline CRGBA Pulse(const CRGBA& a, const CRGBA& b, float timeSec, float speed = 1.0f)
	{
		return Lerp(a, b, 0.5f + 0.5f * std::sin(timeSec * speed * 6.2831853f));
	}

	// Màu theo phần trăm (máu, pin, tải…): 100% xanh -> vàng -> đỏ.
	inline CRGBA Health(float pct01)
	{
		pct01 = std::clamp(pct01, 0.0f, 1.0f);
		return pct01 > 0.5f
			? Lerp(WarningYellow, SuccessGreen, (pct01 - 0.5f) * 2.0f)
			: Lerp(DangerRed, WarningYellow, pct01 * 2.0f);
	}

	// Chữ đen hay trắng dễ đọc hơn trên nền bg (theo độ sáng cảm nhận).
	inline CRGBA ContrastText(const CRGBA& bg)
	{
		const float lum = 0.2126f * detail::R(bg) + 0.7152f * detail::G(bg) + 0.0722f * detail::B(bg);
		return lum > 140.0f ? TextOnAccent : TextWhite;
	}

	// Packed 0xAARRGGBB (hữu ích cho ImGui/D3D nếu cần).
	inline uint32_t ToARGB(const CRGBA& c)
	{
		return (uint32_t(detail::A(c)) << 24) | (uint32_t(detail::R(c)) << 16) |
		       (uint32_t(detail::G(c)) << 8)  |  uint32_t(detail::B(c));
	}
}

namespace palette = color;