#pragma once

struct mat4
{
    f32 rows[4][4];
};

inline mat4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    mat4 m = {0};
	m.rows[0][0] =  2.0f / (r - l);
	m.rows[1][1] =  2.0f / (t - b);
	m.rows[2][2] = -2.0f / (f - n);
	m.rows[3][0] = -(r + l) / (r - l);
	m.rows[3][1] = -(t + b) / (t - b);
	m.rows[3][2] = -(f + n) / (f - n);
	m.rows[3][3] =  1.0f;
	return m;
}
