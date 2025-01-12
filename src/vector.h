#pragma once

struct vec2
{
    union
    {
        f32 data[2];
        struct { f32 x; f32 y; };
        struct { f32 r; f32 g; };
    };
};

struct vec3
{
    union
    {
        f32 data[3];
        struct { f32 x; f32 y; f32 z; };
        struct { f32 r; f32 g; f32 b; };
    };
};

struct vec4
{
    union
    {
        f32 data[4];
        struct { f32 x; f32 y; f32 z; f32 w; };
        struct { f32 r; f32 g; f32 b; f32 a; };
    };
};

vec3 cross(vec3 a, vec3 b);

#define impl_vec_len(f, vec, N)                 \
    inline f32 f(vec a)                         \
    {                                           \
        f32 r = 0.0f;                           \
        for (s8 i = 0; i < N; ++i)              \
        {                                       \
            r += a.data[i] * a.data[i];         \
        }                                       \
        return r;                               \
    }

impl_vec_len(length, vec2, 2);
impl_vec_len(length, vec3, 3);
impl_vec_len(length, vec4, 4);

#define impl_vec_norm(f, vec, N, f_len)         \
    inline vec f(vec a)                         \
    {                                           \
        const f32 len_inv = 1 / f_len(a);       \
        for (s8 i = 0; i < N; ++i)              \
        {                                       \
            a.data[i] *= len_inv;               \
        }                                       \
        return a;                               \
    }

impl_vec_norm(normalize, vec2, 2, length);
impl_vec_norm(normalize, vec3, 3, length);
impl_vec_norm(normalize, vec4, 4, length);

#define impl_vec_dot(f, vec, N)                 \
    inline f32 f(vec a, vec b)                  \
    {                                           \
        f32 r = 0.0f;                           \
        for (s8 i = 0; i < N; ++i)              \
        {                                       \
            r += a.data[i] * b.data[i];         \
        }                                       \
        return r;                               \
    }

impl_vec_dot(dot, vec2, 2);
impl_vec_dot(dot, vec3, 3);
impl_vec_dot(dot, vec4, 4);

#define impl_vec_math_op(op, vec, N)            \
    inline vec operator##op(vec a, vec b)       \
    {                                           \
        vec r;                                  \
        for (s8 i = 0; i < N; ++i)              \
        {                                       \
            r.data[i] = a.data[i] op b.data[i]; \
        }                                       \
        return r;                               \
    }

impl_vec_math_op(+, vec2, 2);
impl_vec_math_op(-, vec2, 2);
impl_vec_math_op(*, vec2, 2);
impl_vec_math_op(/, vec2, 2);

impl_vec_math_op(+, vec3, 3);
impl_vec_math_op(-, vec3, 3);
impl_vec_math_op(*, vec3, 3);
impl_vec_math_op(/, vec3, 3);

impl_vec_math_op(+, vec4, 4);
impl_vec_math_op(-, vec4, 4);
impl_vec_math_op(*, vec4, 4);
impl_vec_math_op(/, vec4, 4);
