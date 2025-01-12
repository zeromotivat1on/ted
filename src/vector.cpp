#include "pch.h"
#include "vector.h"

vec3 cross(vec3 a, vec3 b)
{
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
