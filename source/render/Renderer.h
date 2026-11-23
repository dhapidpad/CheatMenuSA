#pragma once

#include "Globals.h"
#include "Colors.h"

#include <vector>
#include <cstdarg>

// Cờ căn lề, có thể kết hợp bằng toán tử |
enum RenderAlign : int
{
	ALIGN_LEFT     = 0,
	ALIGN_CENTER_X = 1 << 0,
	ALIGN_RIGHT    = 1 << 1,
	ALIGN_CENTER_Y = 1 << 2,
	ALIGN_BOTTOM   = 1 << 3,
	ALIGN_CENTER   = ALIGN_CENTER_X | ALIGN_CENTER_Y
};

enum class TextStyle
{
	Normal,
	Shadow,   // đổ bóng 1px
	Outline   // viền đen 1px
};

class Renderer
{
public:
	explicit Renderer(IDirect3DDevice9* device = nullptr);
	~Renderer();

	Renderer(const Renderer&)            = delete;
	Renderer& operator=(const Renderer&) = delete;

	// ---- Vòng đời ----
	bool init(IDirect3DDevice9* device);
	bool create_fonts(const char* face = "Segoe UI", int size = 14, int sizeSmall = 12, int weight = FW_NORMAL);
	void release();

	void on_lost();
	void on_reset();
	void on_draw_start();   // lưu state + set state vẽ 2D
	void on_draw_end();     // khôi phục state

	// ---- Thông tin ----
	void get_screen_size(int& w, int& h) const;

	// ---- Clipping (scissor), hỗ trợ lồng nhau ----
	void push_clip(float x, float y, float w, float h);
	void pop_clip();

	// ---- Hình chữ nhật ----
	void filled_rect(float x, float y, float w, float h, const CRGBA& color);
	void bordered_rect(float x, float y, float w, float h, const CRGBA& bgColor, const CRGBA& borderColor, float thickness = 1.0f);
	void border_only(float x, float y, float w, float h, const CRGBA& color, float thickness = 1.0f);
	void gradient_vertical(float x, float y, float w, float h, const CRGBA& topColor, const CRGBA& bottomColor);
	void gradient_horizontal(float x, float y, float w, float h, const CRGBA& leftColor, const CRGBA& rightColor);
	void corner_box(float x, float y, float w, float h, const CRGBA& color, float cornerLen = 5.0f, float thickness = 1.0f);

	// ---- Hình chữ nhật bo góc ----
	void filled_rounded_rect(float x, float y, float w, float h, float radius, const CRGBA& color, int cornerSegments = 8);
	void rounded_border(float x, float y, float w, float h, float radius, const CRGBA& color, float thickness = 1.0f, int cornerSegments = 8);
	void bordered_rounded_rect(float x, float y, float w, float h, float radius, const CRGBA& bgColor, const CRGBA& borderColor, float thickness = 1.0f, int cornerSegments = 8);
	void shadow(float x, float y, float w, float h, float radius, int size, const CRGBA& color); // bóng mềm phía sau panel

	// ---- Đường, tròn, tam giác ----
	void line(float x0, float y0, float x1, float y1, const CRGBA& color, float thickness = 1.0f);
	void circle(float cx, float cy, float radius, const CRGBA& color, int segments = 36, float thickness = 1.0f); // viền
	void filled_circle(float cx, float cy, float radius, const CRGBA& color, int segments = 36);
	void triangle(float x0, float y0, float x1, float y1, float x2, float y2, const CRGBA& color, float thickness = 1.0f);
	void filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2, const CRGBA& color);

	// ---- Widget nhỏ ----
	void progress_bar(float x, float y, float w, float h, float fraction, const CRGBA& bg, const CRGBA& fg, float radius = 0.0f);
	void cursor(float x, float y);

	// ---- Chữ (UTF-8, hỗ trợ tiếng Việt, ký tự '\n') ----
	void text(float x, float y, int align, const CRGBA& color, const char* fmt, ...);
	void text_small(float x, float y, int align, const CRGBA& color, const char* fmt, ...);
	void text_shadow(float x, float y, int align, const CRGBA& color, const char* fmt, ...);
	void text_outline(float x, float y, int align, const CRGBA& color, const char* fmt, ...);
	void text_ex(bool smallFont, TextStyle style, float x, float y, int align, const CRGBA& color, const char* fmt, ...);
	void text_size(const char* str, int& w, int& h, bool smallFont = false);

private:
	struct Vertex
	{
		Vertex(float _x, float _y, float _z, D3DCOLOR _color)
			: x(_x - 0.5f), y(_y - 0.5f), z(_z), rhw(1.0f), color(_color) { } // -0.5f: căn pixel D3D9

		float    x, y, z, rhw;
		D3DCOLOR color;
	};

	static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

	// Lõi vẽ (làm việc trực tiếp với D3DCOLOR)
	void draw(D3DPRIMITIVETYPE type, UINT primCount, const Vertex* v);
	static void push_quad(std::vector<Vertex>& out, float x, float y, float w, float h, D3DCOLOR c);
	void fill_rect_c(float x, float y, float w, float h, D3DCOLOR c);
	void border_c(float x, float y, float w, float h, D3DCOLOR c, float t);
	void fill_rounded_c(float x, float y, float w, float h, float r, D3DCOLOR c, int segs);
	void rounded_border_c(float x, float y, float w, float h, float r, D3DCOLOR c, float t, int segs);

	void apply_clip();
	void text_v(bool smallFont, TextStyle style, float x, float y, int align, const CRGBA& color, const char* fmt, va_list args);
	void create_state_block();

	IDirect3DDevice9*     m_pDevice;
	IDirect3DStateBlock9* m_pStateBlock;
	ID3DXFont*            m_pFont;
	ID3DXFont*            m_pFontSmall;

	std::vector<Vertex>   m_Verts;   // buffer dùng lại, tránh cấp phát mỗi frame
	std::vector<RECT>     m_Clips;
	bool                  m_bDrawing;
};