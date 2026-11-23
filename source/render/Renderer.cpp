#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace
{
#ifdef PI
#undef PI
#endif
	constexpr float PI = 3.14159265358979f;

	struct Pt { float x, y; };

	template <class T>
	void safe_release(T*& p)
	{
		if (p) { p->Release(); p = nullptr; }
	}

	// Nhân alpha của D3DCOLOR với hệ số m (0..1+)
	D3DCOLOR mul_alpha(D3DCOLOR c, float m)
	{
		int a = (int)(((c >> 24) & 0xFF) * m + 0.5f);
		a = (std::max)(0, (std::min)(255, a));
		return (c & 0x00FFFFFF) | ((DWORD)a << 24);
	}

	D3DCOLOR black_with_alpha_of(D3DCOLOR c, float m)
	{
		return mul_alpha(c & 0xFF000000, m);
	}

	std::wstring utf8_to_wide(const char* s)
	{
		if (!s || !*s) return std::wstring();

		int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, nullptr, 0);
		UINT cp = CP_UTF8;
		DWORD flags = MB_ERR_INVALID_CHARS;
		if (n <= 0) { cp = CP_ACP; flags = 0; n = MultiByteToWideChar(cp, flags, s, -1, nullptr, 0); }
		if (n <= 1) return std::wstring();

		std::wstring w((size_t)n - 1, L'\0');
		MultiByteToWideChar(cp, flags, s, -1, &w[0], n);
		return w;
	}

	std::wstring format_wide(const char* fmt, va_list args)
	{
		char buf[1024];
		vsnprintf(buf, sizeof(buf), fmt, args);
		buf[sizeof(buf) - 1] = '\0';
		return utf8_to_wide(buf);
	}

	void measure(ID3DXFont* font, const std::wstring& ws, int& w, int& h)
	{
		RECT r = { 0, 0, 0, 0 };
		font->DrawTextW(nullptr, ws.c_str(), -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP, 0xFFFFFFFF);
		w = r.right - r.left;
		h = r.bottom - r.top;
	}

	// Sinh điểm chu vi của hình chữ nhật bo góc: 4 góc, mỗi góc (segs + 1) điểm
	void build_rounded(std::vector<Pt>& out, float x, float y, float w, float h, float r, int segs)
	{
		out.clear();
		r    = (std::max)(0.0f, (std::min)(r, (std::min)(w, h) * 0.5f));
		segs = (std::max)(1, segs);

		const float cx[4] = { x + r,     x + w - r, x + w - r, x + r     };
		const float cy[4] = { y + r,     y + r,     y + h - r, y + h - r };
		const float a0[4] = { PI, float(PI * 1.5f), 0.0f, float(PI * 0.5f) };

		out.reserve(4 * (size_t)(segs + 1));
		for (int c = 0; c < 4; ++c)
		{
			for (int i = 0; i <= segs; ++i)
			{
				float a = a0[c] + (PI * 0.5f) * (float)i / (float)segs;
				out.push_back({ cx[c] + cosf(a) * r, cy[c] + sinf(a) * r });
			}
		}
	}
}

// ============================================================================
// Vòng đời
// ============================================================================

Renderer::Renderer(IDirect3DDevice9* device)
	: m_pDevice(nullptr), m_pStateBlock(nullptr), m_pFont(nullptr), m_pFontSmall(nullptr), m_bDrawing(false)
{
	m_Verts.reserve(512);
	if (!device)
		device = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
	if (device)
		init(device);
}

Renderer::~Renderer()
{
	release();
}

bool Renderer::init(IDirect3DDevice9* device)
{
	if (!device) return false;
	release();
	m_pDevice = device;
	create_state_block();
	return create_fonts();
}

bool Renderer::create_fonts(const char* face, int size, int sizeSmall, int weight)
{
	if (!m_pDevice) return false;

	safe_release(m_pFont);
	safe_release(m_pFontSmall);

	HRESULT h1 = D3DXCreateFontA(m_pDevice, size, 0, weight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, face, &m_pFont);
	HRESULT h2 = D3DXCreateFontA(m_pDevice, sizeSmall, 0, weight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, face, &m_pFontSmall);

	return SUCCEEDED(h1) && SUCCEEDED(h2);
}

void Renderer::release()
{
	m_Clips.clear();
	m_bDrawing = false;
	safe_release(m_pFont);
	safe_release(m_pFontSmall);
	safe_release(m_pStateBlock);
	m_pDevice = nullptr; // không sở hữu device nên không Release
}

void Renderer::create_state_block()
{
	if (m_pDevice && !m_pStateBlock)
		m_pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);
}

void Renderer::on_lost()
{
	m_Clips.clear();
	m_bDrawing = false;
	if (m_pFont)      m_pFont->OnLostDevice();
	if (m_pFontSmall) m_pFontSmall->OnLostDevice();
	safe_release(m_pStateBlock);
}

void Renderer::on_reset()
{
	if (m_pFont)      m_pFont->OnResetDevice();
	if (m_pFontSmall) m_pFontSmall->OnResetDevice();
	create_state_block();
}

void Renderer::on_draw_start()
{
	if (!m_pDevice)
	{
		auto dev = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
		if (dev)
			init(dev);
		else
			return;
	}

	create_state_block();
	if (m_pStateBlock)
		m_pStateBlock->Capture();

	IDirect3DDevice9* d = m_pDevice;

	// Blend chuẩn cho UI 2D
	d->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	d->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	d->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	d->SetRenderState(D3DRS_ZENABLE, FALSE);
	d->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	d->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	d->SetRenderState(D3DRS_LIGHTING, FALSE);
	d->SetRenderState(D3DRS_FOGENABLE, FALSE);
	d->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	d->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
	d->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
	d->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	d->SetTexture(0, nullptr);
	d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	d->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);

	d->SetPixelShader(nullptr);
	d->SetVertexShader(nullptr);
	d->SetFVF(FVF);

	m_Clips.clear();
	m_bDrawing = true;
}

void Renderer::on_draw_end()
{
	if (!m_bDrawing) return;

	m_Clips.clear();
	if (m_pStateBlock)
		m_pStateBlock->Apply();
	m_bDrawing = false;
}

void Renderer::get_screen_size(int& w, int& h) const
{
	w = h = 0;
	if (!m_pDevice) return;

	D3DVIEWPORT9 vp;
	if (SUCCEEDED(m_pDevice->GetViewport(&vp)))
	{
		w = (int)vp.Width;
		h = (int)vp.Height;
	}
}

// ============================================================================
// Clipping
// ============================================================================

void Renderer::push_clip(float x, float y, float w, float h)
{
	if (!m_pDevice) return;

	RECT r;
	r.left   = (LONG)floorf(x);
	r.top    = (LONG)floorf(y);
	r.right  = (LONG)ceilf(x + w);
	r.bottom = (LONG)ceilf(y + h);

	if (!m_Clips.empty())
	{
		const RECT& p = m_Clips.back();
		r.left   = (std::max)(r.left,   p.left);
		r.top    = (std::max)(r.top,    p.top);
		r.right  = (std::min)(r.right,  p.right);
		r.bottom = (std::min)(r.bottom, p.bottom);
	}
	r.right  = (std::max)(r.right,  r.left);
	r.bottom = (std::max)(r.bottom, r.top);

	m_Clips.push_back(r);
	apply_clip();
}

void Renderer::pop_clip()
{
	if (m_Clips.empty()) return;
	m_Clips.pop_back();
	apply_clip();
}

void Renderer::apply_clip()
{
	if (!m_pDevice) return;

	if (m_Clips.empty())
	{
		m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
	else
	{
		m_pDevice->SetScissorRect(&m_Clips.back());
		m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	}
}

// ============================================================================
// Lõi vẽ
// ============================================================================

void Renderer::draw(D3DPRIMITIVETYPE type, UINT primCount, const Vertex* v)
{
	if (!m_pDevice || primCount == 0 || !v) return;

	m_pDevice->SetFVF(FVF);
	m_pDevice->SetTexture(0, nullptr);
	m_pDevice->DrawPrimitiveUP(type, primCount, v, sizeof(Vertex));
}

void Renderer::push_quad(std::vector<Vertex>& out, float x, float y, float w, float h, D3DCOLOR c)
{
	out.emplace_back(x,     y,     0.0f, c);
	out.emplace_back(x + w, y,     0.0f, c);
	out.emplace_back(x,     y + h, 0.0f, c);
	out.emplace_back(x + w, y,     0.0f, c);
	out.emplace_back(x + w, y + h, 0.0f, c);
	out.emplace_back(x,     y + h, 0.0f, c);
}

void Renderer::fill_rect_c(float x, float y, float w, float h, D3DCOLOR c)
{
	if (w <= 0.0f || h <= 0.0f) return;

	const Vertex v[4] =
	{
		{ x,     y,     0.0f, c },
		{ x + w, y,     0.0f, c },
		{ x,     y + h, 0.0f, c },
		{ x + w, y + h, 0.0f, c }
	};
	draw(D3DPT_TRIANGLESTRIP, 2, v);
}

void Renderer::border_c(float x, float y, float w, float h, D3DCOLOR c, float t)
{
	if (w <= 0.0f || h <= 0.0f || t <= 0.0f) return;

	if (t * 2.0f >= (std::min)(w, h))
	{
		fill_rect_c(x, y, w, h, c);
		return;
	}

	m_Verts.clear();
	push_quad(m_Verts, x,         y,         w, t,             c); // trên
	push_quad(m_Verts, x,         y + h - t, w, t,             c); // dưới
	push_quad(m_Verts, x,         y + t,     t, h - 2.0f * t,  c); // trái
	push_quad(m_Verts, x + w - t, y + t,     t, h - 2.0f * t,  c); // phải
	draw(D3DPT_TRIANGLELIST, (UINT)(m_Verts.size() / 3), m_Verts.data());
}

void Renderer::fill_rounded_c(float x, float y, float w, float h, float r, D3DCOLOR c, int segs)
{
	if (w <= 0.0f || h <= 0.0f) return;
	if (r < 0.5f) { fill_rect_c(x, y, w, h, c); return; }

	std::vector<Pt> pts;
	build_rounded(pts, x, y, w, h, r, segs);

	m_Verts.clear();
	m_Verts.emplace_back(x + w * 0.5f, y + h * 0.5f, 0.0f, c);
	for (const Pt& p : pts)
		m_Verts.emplace_back(p.x, p.y, 0.0f, c);
	m_Verts.emplace_back(pts[0].x, pts[0].y, 0.0f, c);

	draw(D3DPT_TRIANGLEFAN, (UINT)pts.size(), m_Verts.data());
}

void Renderer::rounded_border_c(float x, float y, float w, float h, float r, D3DCOLOR c, float t, int segs)
{
	if (w <= 0.0f || h <= 0.0f || t <= 0.0f) return;
	if (r < 0.5f) { border_c(x, y, w, h, c, t); return; }

	if (t * 2.0f >= (std::min)(w, h))
	{
		fill_rounded_c(x, y, w, h, r, c, segs);
		return;
	}

	std::vector<Pt> outer, inner;
	build_rounded(outer, x, y, w, h, r, segs);
	build_rounded(inner, x + t, y + t, w - 2.0f * t, h - 2.0f * t, (std::max)(r - t, 0.0f), segs);

	const size_t n = outer.size();
	m_Verts.clear();
	for (size_t k = 0; k <= n; ++k)
	{
		const size_t i = k % n;
		m_Verts.emplace_back(outer[i].x, outer[i].y, 0.0f, c);
		m_Verts.emplace_back(inner[i].x, inner[i].y, 0.0f, c);
	}
	draw(D3DPT_TRIANGLESTRIP, (UINT)(2 * n), m_Verts.data());
}

// ============================================================================
// Hình chữ nhật
// ============================================================================

void Renderer::filled_rect(float x, float y, float w, float h, const CRGBA& color)
{
	fill_rect_c(x, y, w, h, color.ToIntARGB());
}

void Renderer::bordered_rect(float x, float y, float w, float h, const CRGBA& bgColor, const CRGBA& borderColor, float thickness)
{
	fill_rect_c(x, y, w, h, bgColor.ToIntARGB());
	border_c(x, y, w, h, borderColor.ToIntARGB(), thickness);
}

void Renderer::border_only(float x, float y, float w, float h, const CRGBA& color, float thickness)
{
	border_c(x, y, w, h, color.ToIntARGB(), thickness);
}

void Renderer::gradient_vertical(float x, float y, float w, float h, const CRGBA& topColor, const CRGBA& bottomColor)
{
	if (w <= 0.0f || h <= 0.0f) return;

	const D3DCOLOR t = topColor.ToIntARGB();
	const D3DCOLOR b = bottomColor.ToIntARGB();
	const Vertex v[4] =
	{
		{ x,     y,     0.0f, t },
		{ x + w, y,     0.0f, t },
		{ x,     y + h, 0.0f, b },
		{ x + w, y + h, 0.0f, b }
	};
	draw(D3DPT_TRIANGLESTRIP, 2, v);
}

void Renderer::gradient_horizontal(float x, float y, float w, float h, const CRGBA& leftColor, const CRGBA& rightColor)
{
	if (w <= 0.0f || h <= 0.0f) return;

	const D3DCOLOR l = leftColor.ToIntARGB();
	const D3DCOLOR r = rightColor.ToIntARGB();
	const Vertex v[4] =
	{
		{ x,     y,     0.0f, l },
		{ x + w, y,     0.0f, r },
		{ x,     y + h, 0.0f, l },
		{ x + w, y + h, 0.0f, r }
	};
	draw(D3DPT_TRIANGLESTRIP, 2, v);
}

void Renderer::corner_box(float x, float y, float w, float h, const CRGBA& color, float cornerLen, float thickness)
{
	if (w <= 0.0f || h <= 0.0f) return;

	const D3DCOLOR c = color.ToIntARGB();
	const float L = (std::min)(cornerLen, (std::min)(w, h) * 0.5f);
	const float t = thickness;

	m_Verts.clear();
	// Trái-trên
	push_quad(m_Verts, x,         y,         L, t, c);
	push_quad(m_Verts, x,         y + t,     t, L - t, c);
	// Phải-trên
	push_quad(m_Verts, x + w - L, y,         L, t, c);
	push_quad(m_Verts, x + w - t, y + t,     t, L - t, c);
	// Trái-dưới
	push_quad(m_Verts, x,         y + h - t, L, t, c);
	push_quad(m_Verts, x,         y + h - L, t, L - t, c);
	// Phải-dưới
	push_quad(m_Verts, x + w - L, y + h - t, L, t, c);
	push_quad(m_Verts, x + w - t, y + h - L, t, L - t, c);

	draw(D3DPT_TRIANGLELIST, (UINT)(m_Verts.size() / 3), m_Verts.data());
}

// ============================================================================
// Bo góc / bóng
// ============================================================================

void Renderer::filled_rounded_rect(float x, float y, float w, float h, float radius, const CRGBA& color, int cornerSegments)
{
	fill_rounded_c(x, y, w, h, radius, color.ToIntARGB(), cornerSegments);
}

void Renderer::rounded_border(float x, float y, float w, float h, float radius, const CRGBA& color, float thickness, int cornerSegments)
{
	rounded_border_c(x, y, w, h, radius, color.ToIntARGB(), thickness, cornerSegments);
}

void Renderer::bordered_rounded_rect(float x, float y, float w, float h, float radius, const CRGBA& bgColor, const CRGBA& borderColor, float thickness, int cornerSegments)
{
	fill_rounded_c(x, y, w, h, radius, bgColor.ToIntARGB(), cornerSegments);
	rounded_border_c(x, y, w, h, radius, borderColor.ToIntARGB(), thickness, cornerSegments);
}

void Renderer::shadow(float x, float y, float w, float h, float radius, int size, const CRGBA& color)
{
	if (size <= 0) return;

	const D3DCOLOR base = color.ToIntARGB();
	for (int i = size; i >= 1; --i)
	{
		const float t = (float)i / (float)(size + 1);
		float m = (1.0f - t) * (1.0f - t) * (3.0f / (float)size);
		m = (std::min)(m, 1.0f);

		const float f = (float)i;
		fill_rounded_c(x - f, y - f, w + 2.0f * f, h + 2.0f * f, radius + f, mul_alpha(base, m), 8);
	}
}

// ============================================================================
// Đường, tròn, tam giác
// ============================================================================

void Renderer::line(float x0, float y0, float x1, float y1, const CRGBA& color, float thickness)
{
	const float dx = x1 - x0;
	const float dy = y1 - y0;
	const float len = sqrtf(dx * dx + dy * dy);
	if (len < 1e-4f) return;

	const float nx = -dy / len * thickness * 0.5f;
	const float ny =  dx / len * thickness * 0.5f;
	const D3DCOLOR c = color.ToIntARGB();

	const Vertex v[4] =
	{
		{ x0 + nx, y0 + ny, 0.0f, c },
		{ x0 - nx, y0 - ny, 0.0f, c },
		{ x1 + nx, y1 + ny, 0.0f, c },
		{ x1 - nx, y1 - ny, 0.0f, c }
	};
	draw(D3DPT_TRIANGLESTRIP, 2, v);
}

void Renderer::circle(float cx, float cy, float radius, const CRGBA& color, int segments, float thickness)
{
	if (radius <= 0.0f) return;
	segments = (std::max)(3, segments);

	const D3DCOLOR c = color.ToIntARGB();
	const float rin = (std::max)(radius - thickness, 0.0f);

	m_Verts.clear();
	for (int i = 0; i <= segments; ++i)
	{
		const float a = 2.0f * PI * (float)i / (float)segments;
		const float ca = cosf(a), sa = sinf(a);
		m_Verts.emplace_back(cx + ca * radius, cy + sa * radius, 0.0f, c);
		m_Verts.emplace_back(cx + ca * rin,    cy + sa * rin,    0.0f, c);
	}
	draw(D3DPT_TRIANGLESTRIP, (UINT)(2 * segments), m_Verts.data());
}

void Renderer::filled_circle(float cx, float cy, float radius, const CRGBA& color, int segments)
{
	if (radius <= 0.0f) return;
	segments = (std::max)(3, segments);

	const D3DCOLOR c = color.ToIntARGB();

	m_Verts.clear();
	m_Verts.emplace_back(cx, cy, 0.0f, c);
	for (int i = 0; i <= segments; ++i)
	{
		const float a = 2.0f * PI * (float)i / (float)segments;
		m_Verts.emplace_back(cx + cosf(a) * radius, cy + sinf(a) * radius, 0.0f, c);
	}
	draw(D3DPT_TRIANGLEFAN, (UINT)segments, m_Verts.data());
}

void Renderer::triangle(float x0, float y0, float x1, float y1, float x2, float y2, const CRGBA& color, float thickness)
{
	line(x0, y0, x1, y1, color, thickness);
	line(x1, y1, x2, y2, color, thickness);
	line(x2, y2, x0, y0, color, thickness);
}

void Renderer::filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2, const CRGBA& color)
{
	const D3DCOLOR c = color.ToIntARGB();
	const Vertex v[3] =
	{
		{ x0, y0, 0.0f, c },
		{ x1, y1, 0.0f, c },
		{ x2, y2, 0.0f, c }
	};
	draw(D3DPT_TRIANGLELIST, 1, v);
}

// ============================================================================
// Widget nhỏ
// ============================================================================

void Renderer::progress_bar(float x, float y, float w, float h, float fraction, const CRGBA& bg, const CRGBA& fg, float radius)
{
	fraction = (std::max)(0.0f, (std::min)(1.0f, fraction));

	fill_rounded_c(x, y, w, h, radius, bg.ToIntARGB(), 6);

	const float fw = w * fraction;
	if (fw >= 1.0f)
		fill_rounded_c(x, y, fw, h, radius, fg.ToIntARGB(), 6);
}

void Renderer::cursor(float x, float y)
{
	// Mũi tên chuột: 7 điểm, tách thành 5 tam giác
	static const float P[7][2] = { {0,0}, {0,16}, {4,12}, {7,18}, {9,17}, {6,11}, {11,11} };
	static const int   T[5][3] = { {0,1,2}, {0,2,5}, {0,5,6}, {2,3,4}, {2,4,5} };

	auto draw_arrow = [&](float ox, float oy, D3DCOLOR c)
	{
		m_Verts.clear();
		for (const auto& tri : T)
			for (int k = 0; k < 3; ++k)
				m_Verts.emplace_back(x + ox + P[tri[k]][0], y + oy + P[tri[k]][1], 0.0f, c);
		draw(D3DPT_TRIANGLELIST, 5, m_Verts.data());
	};

	const D3DCOLOR black = D3DCOLOR_ARGB(255, 0, 0, 0);
	const D3DCOLOR white = D3DCOLOR_ARGB(255, 255, 255, 255);

	static const float O[8][2] = { {-1,0}, {1,0}, {0,-1}, {0,1}, {-1,-1}, {1,-1}, {-1,1}, {1,1} };
	for (const auto& o : O)
		draw_arrow(o[0], o[1], black);

	draw_arrow(0.0f, 0.0f, white);
}

// ============================================================================
// Chữ
// ============================================================================

void Renderer::text_v(bool smallFont, TextStyle style, float x, float y, int align, const CRGBA& color, const char* fmt, va_list args)
{
	ID3DXFont* font = smallFont ? m_pFontSmall : m_pFont;
	if (!font || !fmt) return;

	const std::wstring ws = format_wide(fmt, args);
	if (ws.empty()) return;

	int w = 0, h = 0;
	measure(font, ws, w, h);

	int px = (int)x;
	int py = (int)y;

	if (align & ALIGN_CENTER_X)      px -= w / 2;
	else if (align & ALIGN_RIGHT)    px -= w;

	if (align & ALIGN_CENTER_Y)      py -= h / 2;
	else if (align & ALIGN_BOTTOM)   py -= h;

	const D3DCOLOR c = color.ToIntARGB();

	auto put = [&](int ox, int oy, D3DCOLOR col)
	{
		RECT rc = { px + ox, py + oy, px + ox, py + oy };
		font->DrawTextW(nullptr, ws.c_str(), -1, &rc, DT_NOCLIP | DT_LEFT | DT_TOP, col);
	};

	switch (style)
	{
	case TextStyle::Shadow:
		put(1, 1, black_with_alpha_of(c, 0.75f));
		break;

	case TextStyle::Outline:
	{
		const D3DCOLOR o = black_with_alpha_of(c, 1.0f);
		static const int D[8][2] = { {-1,0}, {1,0}, {0,-1}, {0,1}, {-1,-1}, {1,-1}, {-1,1}, {1,1} };
		for (const auto& d : D)
			put(d[0], d[1], o);
		break;
	}

	default:
		break;
	}

	put(0, 0, c);
}

void Renderer::text(float x, float y, int align, const CRGBA& color, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(false, TextStyle::Normal, x, y, align, color, fmt, args);
	va_end(args);
}

void Renderer::text_small(float x, float y, int align, const CRGBA& color, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(true, TextStyle::Normal, x, y, align, color, fmt, args);
	va_end(args);
}

void Renderer::text_shadow(float x, float y, int align, const CRGBA& color, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(false, TextStyle::Shadow, x, y, align, color, fmt, args);
	va_end(args);
}

void Renderer::text_outline(float x, float y, int align, const CRGBA& color, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(false, TextStyle::Outline, x, y, align, color, fmt, args);
	va_end(args);
}

void Renderer::text_ex(bool smallFont, TextStyle style, float x, float y, int align, const CRGBA& color, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(smallFont, style, x, y, align, color, fmt, args);
	va_end(args);
}

void Renderer::text_size(const char* str, int& w, int& h, bool smallFont)
{
	w = h = 0;

	ID3DXFont* font = smallFont ? m_pFontSmall : m_pFont;
	if (!font) return;

	const std::wstring ws = utf8_to_wide(str);
	if (ws.empty()) return;

	measure(font, ws, w, h);
}