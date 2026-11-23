#include "MathUtils.h"

int math::near_zero(float v)
{
	if (!std::isfinite(v)) return 1;
	return std::fabs(v) < FLOAT_EPSILON;
}

int math::vect3_near_zero(const CVector& in)
{
	return near_zero(in.x) && near_zero(in.y) && near_zero(in.z);
}

CVector math::extend_vec3(const CVector& start, const CVector& end, float distance)
{
	auto diff = end - start;
	float len = diff.Magnitude();
	if (len > 0.0001f)
	{
		diff /= len;
		return start + (diff * distance);
	}
	return end;
}

CRGBA math::from_hsb(float hue, float saturation, float brightness)
{
	float h = (hue >= 1.0f) ? 0.0f : hue * 6.0f;
	float f = h - static_cast<int>(h);
	float p = brightness * (1.0f - saturation);
	float q = brightness * (1.0f - saturation * f);
	float t = brightness * (1.0f - (saturation * (1.0f - f)));

	auto to_byte = [](float val) -> unsigned char {
		return static_cast<unsigned char>(std::clamp(val * 255.0f, 0.0f, 255.0f));
	};

	if (h < 1.0f)      return CRGBA(to_byte(brightness), to_byte(t), to_byte(p));
	else if (h < 2.0f) return CRGBA(to_byte(q), to_byte(brightness), to_byte(p));
	else if (h < 3.0f) return CRGBA(to_byte(p), to_byte(brightness), to_byte(t));
	else if (h < 4.0f) return CRGBA(to_byte(p), to_byte(q), to_byte(brightness));
	else if (h < 5.0f) return CRGBA(to_byte(t), to_byte(p), to_byte(brightness));
	else               return CRGBA(to_byte(brightness), to_byte(p), to_byte(q));
}

CVector math::matrix_vec4_mult(const CMatrix* m, const CVector& point, float w)
{
	CVector out;
	out.x = m->at.x * point.z + m->right.x * point.x + m->up.x * point.y + w * m->pos.x;
	out.y = m->at.y * point.z + m->right.y * point.x + m->up.y * point.y + w * m->pos.y;
	out.z = m->at.z * point.z + m->right.z * point.x + m->up.z * point.y + w * m->pos.z;
	return out;
}

void math::matrix_matrix_mult(const CMatrix* in1, const CMatrix* in2, CMatrix* out)
{
	float res[16], _in1[16], _in2[16];
	memcpy(_in1, in1, sizeof(float) * 16);
	memcpy(_in2, in2, sizeof(float) * 16);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			res[i * 4 + j] = _in1[i * 4 + 0] * _in2[0 + j] +
			                 _in1[i * 4 + 1] * _in2[4 + j] +
			                 _in1[i * 4 + 2] * _in2[8 + j] +
			                 _in1[i * 4 + 3] * _in2[12 + j];
		}
	}
	memcpy(out, res, sizeof(float) * 16);
}

void math::matrix_vect3_rotate(const CMatrix* in, const CVector& vect, float t, CMatrix* out)
{
	const float x = vect.x, y = vect.y, z = vect.z;
	const float sin_t = sinf(t), cos_t = cosf(t);
	float res[16];
	const float matrix[16] = {
		cos_t + (1.0f - cos_t) * x * x,
		(1.0f - cos_t) * x * y - sin_t * z,
		(1.0f - cos_t) * x * z + sin_t * y,
		0.0f,
		(1.0f - cos_t) * y * x + sin_t * z,
		cos_t + (1.0f - cos_t) * y * y,
		(1.0f - cos_t) * y * z - sin_t * x,
		0.0f,
		(1.0f - cos_t) * z * x - sin_t * y,
		(1.0f - cos_t) * z * y + sin_t * x,
		cos_t + (1.0f - cos_t) * z * z,
		0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	matrix_matrix_mult(in, (CMatrix*)matrix, (CMatrix*)res);
	memcpy(&res[4 * 3], &in[4 * 3], sizeof(float) * 4);
	memcpy(out, res, sizeof(float) * 16);
}
