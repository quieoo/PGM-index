#include <stdio.h>
#include "softfloat.h"

int main(){
    printf("%d\n", 0x10);

    SoftFloat sf=int2sf(50000,0);
    printf("sign: %d, mant: %ul, exp: %d\n", sf.sign, sf.mant, sf.exp);

    printf("value: %u\n", sf2int(sf));
}