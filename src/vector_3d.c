#include "vector_3d.h"

void sum(const struct vector_3d *s_1, const struct vector_3d *s_2, struct vector_3d *result)
{
    result->x = s_1->x + s_2->x;
    result->y = s_1->y + s_2->y;
    result->z = s_1->z + s_2->z;
}

void sub(const struct vector_3d *minuend, const struct vector_3d *subtrahend, struct vector_3d *result)
{
    result->x = minuend->x - subtrahend->x;
    result->y = minuend->y - subtrahend->y;
    result->z = minuend->z - subtrahend->z;
}

long int dot(const struct vector_3d *v_1, const struct vector_3d *v_2)
{
    return (v_1->x * v_2->x + v_1->y * v_2->y + v_1->z * v_2->z);
}

void cross(const struct vector_3d *v_1, const struct vector_3d *v_2, struct vector_3d *result)
{
    result->x = v_1->y * v_2->z - v_1->z * v_2->y;
    result->y = v_1->z * v_2->x - v_1->x * v_2->z;
    result->z = v_1->x * v_2->y - v_1->y * v_2->x;
}