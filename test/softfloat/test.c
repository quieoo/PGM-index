#include <stdio.h>
#include "softfloat.h"

int main(){
    printf("%d\n", 0x10);

    SoftFloat sf=int2sf(50000,0);
    printf("sign: %d, mant: %ul, exp: %d\n", sf.sign, sf.mant, sf.exp);

    printf("value: %u\n", sf2int(sf));

    SoftFloat a=int2sf(2,0);
    SoftFloat b=int2sf(7,0);
    sf=div_sf(a,b);
    printf("sign: %d, mant: %llu, exp: %d\n", sf.sign, sf.mant, sf.exp);

}