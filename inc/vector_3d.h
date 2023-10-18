#ifndef __VECTOR_3D_H__
#define __VECTOR_3D_H__

struct vector_3d {
    int x;
    int y;
    int z;
};

void sum(const struct vector_3d *a, const struct vector_3d *b, struct vector_3d *result);
void sub(const struct vector_3d *minuend, const struct vector_3d *subtrahend, struct vector_3d *result);
long int dot(const struct vector_3d *v_1, const struct vector_3d *v_2);
void cross(const struct vector_3d *v_1, const struct vector_3d *v_2, struct vector_3d *result);

#endif // __VECTOR_3D_H__