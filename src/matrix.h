#pragma once

#include "vector.h"
#include "quat.h"

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
void rotate(mat4* mtx, const mat4* rot);
void rotate(mat4* mtx, quat q);

mat4 mat4_identity();
mat4 mat4_transform(vec3 t, quat r, vec3 s);
mat4 mat4_lookat(vec3 eye, vec3 at, vec3 up);
mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
mat4 mat4_perspective(f32 rfovy, f32 aspect, f32 n, f32 f);

mat4 to_mat4(const mat3* mtx);

mat3 to_mat3(quat q);
mat4 to_mat4(quat q);

mat4 operator*(const mat4& a, const mat4& b);
