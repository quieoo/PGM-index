#include <cstdio>
#include <utility>
#include <bits/stdc++.h>
#include <cstring>
#include "softfloat_v2.h"

std::pair<SoftFloat, int> func1(){
    return {SoftFloat(1),2};
}


struct seg
{
    uint8_t base[4];

    seg()=default;
    seg(int i){
        for(int j=0;j<4;j++)
            base[j]=i;
    }
    void set(){
        for(int i=0;i<4;i++)
            base[i]=i;
    }


    void print(){
        for(int i=0;i<4;i++){
            printf("%d\n", base[i]);
        }
    }
};


int main(){
    float a=1.3335;
    float b=6.4676845;
    printf("%f\n", a*b);

    uint32_t _a, _b;
    memcpy(&_a, &a, sizeof(a));
    memcpy(&_b, &b, sizeof(b));
    float32_t fa,fb, fc;
    fa.v=_a;
    fb.v=_b;

    fc=f32_mul(fa, fb);
    
    float c;
    memcpy(&c, &(fc.v), sizeof(c));
    printf("%f\n", c);

    float32_t d=i64_to_f32(230);
    float _d;
    memcpy(&_d, &(d.v), sizeof(_d));
    printf("%f \n", _d);

    SoftFloat e;
    e.from_float(1.456789);
    print_sf(&e);
    // 0 01111111 01110100111100000010000
}