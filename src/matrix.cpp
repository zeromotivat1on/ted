#include "pch.h"
#include "matrix.h"
#include "quat.h"
#include "vector.h"
#include <math.h>
#include <string.h>

void translate(mat4* mtx, vec3 v)
{
    mtx->rows[3][0] = v.x;
    mtx->rows[3][1] = v.y;
    mtx->rows[3][2] = v.z;
}

void scale(mat4* mtx, vec3 v)
{
	mtx->rows[0][0] = v.x;
	mtx->rows[1][1] = v.y;
	mtx->rows[2][2] = v.z;
}

void rotate(mat4* mtx, const mat4* rot)
{
    const mat4 res = *mtx * *rot;
    *mtx = res;
}

void rotate(mat4* mtx, quat q)
{
    const mat4 rot = to_mat4(q);
    rotate(mtx, &rot);
}

mat4 mat4_identity()
{
    mat4 mtx = {0};
    mtx.rows[0][0] = 1.0f;
	mtx.rows[1][1] = 1.0f;
	mtx.rows[2][2] = 1.0f;
	mtx.rows[3][3] = 1.0f;
    return mtx;
}

mat4 mat4_transform(vec3 t, quat r, vec3 s)
{
    mat4 mtx = mat4_identity();
    scale(&mtx, s);
    rotate(&mtx, r);
    translate(&mtx, t);
    return mtx;
}

mat4 mat4_lookat(vec3 eye, vec3 at, vec3 up)
{
    const vec3 f = normalize(at - eye);
	const vec3 r = normalize(cross(f, up));
	const vec3 u = cross(r, f);

	mat4 mtx;

	mtx.rows[0][0] = r.x;
	mtx.rows[1][0] = r.y;
	mtx.rows[2][0] = r.z;
	mtx.rows[3][0] = -dot(r, eye);

	mtx.rows[0][1] = u.x;
	mtx.rows[1][1] = u.y;
	mtx.rows[2][1] = u.z;
	mtx.rows[3][1] = -dot(u, eye);

	mtx.rows[0][2] = -f.x;
	mtx.rows[1][2] = -f.y;
	mtx.rows[2][2] = -f.z;
	mtx.rows[3][2] = dot(f, eye);

	mtx.rows[0][3] = 0.0f;
	mtx.rows[1][3] = 0.0f;
	mtx.rows[2][3] = 0.0f;
	mtx.rows[3][3] = 1.0f;

	return mtx;
}

mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    mat4 mtx = {0};
	mtx.rows[0][0] =  2.0f / (r - l);
	mtx.rows[1][1] =  2.0f / (t - b);
	mtx.rows[2][2] = -2.0f / (f - n);
	mtx.rows[3][0] = -(r + l) / (r - l);
	mtx.rows[3][1] = -(t + b) / (t - b);
	mtx.rows[3][2] = -(f + n) / (f - n);
	mtx.rows[3][3] =  1.0f;
    return mtx;
}

mat4 mat4_perspective(f32 rfovy, f32 aspect, f32 n, f32 f)
{
	const f32 tan_half_fovy = tanf(rfovy * 0.5f);

	mat4 mtx = {0};
	mtx.rows[0][0] = 1.0f / (aspect * tan_half_fovy);
	mtx.rows[1][1] = 1.0f / tan_half_fovy;
	mtx.rows[2][2] = -(f + n) / (f - n);
	mtx.rows[2][3] = -1.0f;
	mtx.rows[3][2] = -(2.0f * f * n) / (f - n);

	return mtx;
}

mat4 to_mat4(const mat3* mtx)
{
    return mat4 {
		mtx->rows[0][0], mtx->rows[0][1], mtx->rows[0][2], 0.0f,
		mtx->rows[1][0], mtx->rows[1][1], mtx->rows[1][2], 0.0f,
		mtx->rows[2][0], mtx->rows[2][1], mtx->rows[2][2], 0.0f,
		0.0f,		     0.0f,		      0.0f,		       1.0f
	};
}

mat3 to_mat3(quat q)
{
    mat3 mtx;
    
    const f32 x2 = q.x + q.x;
    const f32 y2 = q.y + q.y;
    const f32 z2 = q.z + q.z;

    const f32 xx = q.x * x2;
    const f32 xy = q.x * y2;
    const f32 xz = q.x * z2;

    const f32 yy = q.y * y2;
    const f32 yz = q.y * z2;
    const f32 zz = q.z * z2;

    const f32 wx = q.w * x2;
    const f32 wy = q.w * y2;
    const f32 wz = q.w * z2;

    mtx.rows[0][0] = 1.0f - (yy + zz);
    mtx.rows[1][0] = xy - wz;
    mtx.rows[2][0] = xz + wy;

    mtx.rows[0][1] = xy + wz;
    mtx.rows[1][1] = 1.0f - (xx + zz);
    mtx.rows[2][1] = yz - wx;

    mtx.rows[0][2] = xz - wy;
    mtx.rows[1][2] = yz + wx;
    mtx.rows[2][2] = 1.0f - (xx + yy);

    return mtx;
}

mat4 to_mat4(quat q)
{
    const mat3 mtx = to_mat3(q);
    return to_mat4(&mtx);
}

mat4 operator*(const mat4& a, const mat4& b)
{
    mat4 res = {0};
    f32* res_ptr = (f32*)&res;
    f32* a_ptr = (f32*)&a;
    f32* b_ptr = (f32*)&b;
    
    for (s32 i = 0; i < 4; ++i)
	{
		for (s32 j = 0; j < 4; ++j)
		{
			*res_ptr = a_ptr[0] * b_ptr[0 * 4 + j] +
                       a_ptr[1] * b_ptr[1 * 4 + j] +
                       a_ptr[2] * b_ptr[2 * 4 + j] +
                       a_ptr[3] * b_ptr[3 * 4 + j];
			res_ptr++;
		}
		a_ptr += 4;
	}

    return res;    
}
