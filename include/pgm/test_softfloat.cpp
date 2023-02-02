#include "softfloat.hpp"
#include <cstdio>

int main(){
    SoftFloat sf(50);
    printf("sign: %d, mant: %lu, exp: %d\n", sf.sign, sf.mant, sf.exp);
    sf.normalize();
    printf("sign: %d, mant: %lu, exp: %d\n", sf.sign, sf.mant, sf.exp);
    
    // operator==
    if(sf==50){
        printf("equal\n");
    }

    // operator=
    SoftFloat sff(500);
    sf=sff;
    printf("sign: %d, mant: %lu, exp: %d\n", sf.sign, sf.mant, sf.exp);

    // operator*
    SoftFloat mul=sf*10;
    printf("sign: %d, mant: %lu, exp: %d\n", mul.sign, mul.mant, mul.exp);

    // operator()
    printf("%ld\n", int64_t(mul));
    
    printf("%d\n",int64_t(sf*10)+100);
    
    return 0;



}