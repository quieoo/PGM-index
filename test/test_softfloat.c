#include <stdio.h>
#include <string.h>
#include "../include/pgm/softfloat_v2.h"

int main(){
    float a=13.235;
    float b=64.676845;
    printf("%f\n", a*b);

    uint32_t _a, _b;
    memcpy(&_a, &a, sizeof(a));
    memcpy(&_b, &b, sizeof(b));
    union ui32_f32 fa,fb, fc;
    fa.ui=_a;
    fb.ui=_b;

    fc.f=f32_mul(fa, fb);
    printf("%f * %f = %d\n", a, b, f32_to_i64(fc.f));
    
    // float c;
    // memcpy(&c, &(fc.v), sizeof(c));
    // printf("%f\n", c);

    // float32_t d=i64_to_f32(230);
    // float _d;
    // memcpy(&_d, &(d.v), sizeof(_d));
    // printf("%f \n", _d);

    int64_t c=12;
    float32_t c_t=i64_to_f32(c);

    printf("%d\n", f32_to_i64(c_t));

}