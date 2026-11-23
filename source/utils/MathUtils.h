#pragma once

#include "Globals.h"

namespace math
{
	int     near_zero(float v);
	int     vect3_near_zero(const CVector& in);
	CVector extend_vec3(const CVector& start, const CVector& end, float distance = 1000.0f);
	CRGBA   from_hsb(float hue, float saturation, float brightness);

	CVector matrix_vec4_mult(const CMatrix* m, const CVector& point, float w);
	void    matrix_matrix_mult(const CMatrix* in1, const CMatrix* in2, CMatrix* out);
	void    matrix_vect3_rotate(const CMatrix* in, const CVector& vect, float t, CMatrix* out);
}
