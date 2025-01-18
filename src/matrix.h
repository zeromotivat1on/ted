#pragma once

#include "vector.h"

struct mat2
{
    f32 rows[2][2];
};

struct mat3
{
    f32 rows[3][3];
};

struct mat4
{
    f32 rows[4][4];
};

void identity(mat4* mtx);
void translate(mat4* mtx, vec3 v);
void scale(mat4* mtx, vec3 v);

mat4 mat4_identity();
mat4 mat4_lookat(vec3 eye, vec3 at, vec3 up);
mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
mat4 mat4_perspective(f32 rfovy, f32 aspect, f32 n, f32 f);

mat4 to_mat4(const mat3* mtx);

mat4 operator*(const mat4& a, const mat4& b);
