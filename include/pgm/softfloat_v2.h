#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

#ifndef soft_float_v2
#define soft_float_v2 1

#ifndef THREAD_LOCAL
#define THREAD_LOCAL
#endif

typedef struct { uint32_t v; } float32_t;
union ui32_f32 { uint32_t ui; float32_t f; };
struct exp16_sig32 { int_fast16_t exp; uint_fast32_t sig; };
#define signF32UI( a ) ((bool) ((uint32_t) (a)>>31))
#define expF32UI( a ) ((int_fast16_t) ((a)>>23) & 0xFF)
#define fracF32UI( a ) ((a) & 0x007FFFFF)


const uint_least8_t softfloat_countLeadingZeros8[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct exp16_sig32 softfloat_normSubnormalF32Sig( uint_fast32_t sig )
{
    int_fast8_t shiftDist=0;
    struct exp16_sig32 z;

    if ( sig < 0x10000 ) {
        shiftDist = 16;
        sig <<= 16;
    }
    if ( sig < 0x1000000 ) {
        shiftDist += 8;
        sig <<= 8;
    }
    shiftDist += softfloat_countLeadingZeros8[sig>>24];
    shiftDist-=8;

    z.exp = 1 - shiftDist;
    z.sig = sig<<shiftDist;
    return z;
}

#define softfloat_shortShiftRightJam64( a, dist ) (a)>>(dist) | (((a) & (((uint_fast64_t) 1<<(dist)) - 1)) != 0)

#define softfloat_isSigNaNF32UI( uiA ) ((((uiA) & 0x7FC00000) == 0x7F800000) && ((uiA) & 0x003FFFFF))
#define isNaNF32UI( a ) (((~(a) & 0x7F800000) == 0) && ((a) & 0x007FFFFF))

enum {
    softfloat_flag_inexact   =  1,
    softfloat_flag_underflow =  2,
    softfloat_flag_overflow  =  4,
    softfloat_flag_infinite  =  8,
    softfloat_flag_invalid   = 16
};

uint_fast32_t
 softfloat_propagateNaNF32UI( uint_fast32_t uiA, uint_fast32_t uiB )
{
    bool isSigNaNA;
    isSigNaNA = softfloat_isSigNaNF32UI( uiA );
    if ( isSigNaNA || softfloat_isSigNaNF32UI( uiB ) ) {
        // softfloat_raiseFlags( softfloat_flag_invalid );
        if ( isSigNaNA ) return uiA | 0x00400000;
    }
    return (isNaNF32UI( uiA ) ? uiA : uiB) | 0x00400000;
}



#define packToF32UI( sign, exp, sig ) (((uint32_t) (sign)<<31) + ((uint32_t) (exp)<<23) + (sig))



#define defaultNaNF32UI 0x7FC00000
enum {
    softfloat_round_near_even   = 0,
    softfloat_round_minMag      = 1,
    softfloat_round_min         = 2,
    softfloat_round_max         = 3,
    softfloat_round_near_maxMag = 4,
    softfloat_round_odd         = 6
};
enum {
    softfloat_tininess_beforeRounding = 0,
    softfloat_tininess_afterRounding  = 1
};
#define init_detectTininess softfloat_tininess_beforeRounding

THREAD_LOCAL uint_fast8_t softfloat_roundingMode = softfloat_round_near_even;
THREAD_LOCAL uint_fast8_t softfloat_detectTininess = init_detectTininess;
#define softfloat_shiftRightJam32(a, dist) ((dist) < 31) ? (a)>>(dist) | ((uint32_t) ((a)<<(-(dist) & 31)) != 0) : ((a) != 0)

float32_t
 softfloat_roundPackToF32( bool sign, int_fast16_t exp, uint_fast32_t sig )
{
    union ui32_f32 uZ;

    if ( 0xFD <= (unsigned int) exp ) {
        if ( exp < 0 ) {
            sig = softfloat_shiftRightJam32( sig, -exp );
            exp = 0;
        } else if ( (0xFD < exp) || (0x80000000 <= sig + 0x40) ) {
            uZ.ui = packToF32UI( sign, 0xFF, 0 ) - ! 0x40;
            goto uiZ;
        }
    }
    sig = (sig + 0x40)>>7;
    sig &= ~(uint_fast32_t) (! ((sig & 0x70) ^ 0x40) & 1);
    if ( ! sig ) exp = 0;
    uZ.ui = packToF32UI( sign, exp, sig );
 uiZ:
    return uZ.f;
}


float32_t f32_mul( float32_t *a, float32_t *b )
{
    union ui32_f32 uA;
    bool signA;
    int_fast16_t expA;
    uint_fast32_t sigA;
    union ui32_f32 uB;
    bool signB;
    int_fast16_t expB;
    uint_fast32_t sigB;
    bool signZ;
    uint_fast32_t magBits;
    struct exp16_sig32 normExpSig;
    int_fast16_t expZ;
    uint_fast32_t sigZ;
    union ui32_f32 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = *a;
    signA = signF32UI( uA.ui );
    expA  = expF32UI( uA.ui );
    sigA  = fracF32UI( uA.ui );
    uB.f = *b;
    signB = signF32UI( uB.ui );
    expB  = expF32UI( uB.ui );
    sigB  = fracF32UI( uB.ui );
    signZ = signA ^ signB;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0xFF ) {
        if ( sigA || ((expB == 0xFF) && sigB) ) goto propagateNaN;
        magBits = expB | sigB;
        goto infArg;
    }
    if ( expB == 0xFF ) {
        if ( sigB ) goto propagateNaN;
        magBits = expA | sigA;
        goto infArg;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = softfloat_normSubnormalF32Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zero;
        normExpSig = softfloat_normSubnormalF32Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA + expB - 0x7F;
    sigA = (sigA | 0x00800000)<<7;
    sigB = (sigB | 0x00800000)<<8;
    sigZ = softfloat_shortShiftRightJam64( (uint_fast64_t) sigA * sigB, 32 );
    
    if ( sigZ < 0x40000000 ) {
        --expZ;
        sigZ <<= 1;
    }
    return softfloat_roundPackToF32( signZ, expZ, sigZ );

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uZ.ui = softfloat_propagateNaNF32UI( uA.ui, uB.ui );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infArg:
    if ( ! magBits ) {
        // softfloat_raiseFlags( softfloat_flag_invalid );
        uZ.ui = defaultNaNF32UI;
    } else {
        uZ.ui = packToF32UI( signZ, 0xFF, 0 );
    }
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uZ.ui = packToF32UI( signZ, 0, 0 );
 uiZ:
    return uZ.f;
}


float32_t i64_to_f32( int64_t a )
{
    bool sign;
    uint_fast64_t absA;
    int_fast8_t shiftDist;
    union ui32_f32 u;
    uint_fast32_t sig;

    sign = (a < 0);
    absA = sign ? -(uint_fast64_t) a : (uint_fast64_t) a;

    uint32_t a32=absA;
    shiftDist = 0;
    a32 = a>>32;
    if ( ! a32 ) {
        shiftDist = 32;
        a32 = a;
    }
    if ( a32 < 0x10000 ) {
        shiftDist += 16;
        a32 <<= 16;
    }
    if ( a32 < 0x1000000 ) {
        shiftDist += 8;
        a32 <<= 8;
    }
    shiftDist += softfloat_countLeadingZeros8[a32>>24];
    shiftDist -= 40;

    if ( 0 <= shiftDist ) {
        u.ui =
            a ? packToF32UI(
                    sign, 0x95 - shiftDist, (uint_fast32_t) absA<<shiftDist )
                : 0;
        return u.f;
    } else {
        shiftDist += 7;
        sig = 
            (shiftDist < 0)
                ? softfloat_shortShiftRightJam64( absA, -shiftDist )
                : (uint_fast32_t) absA<<shiftDist;
        return softfloat_roundPackToF32( sign, 0x9C - shiftDist, sig );
    }
}



int_fast64_t f32_to_i64(float32_t f){
    uint8_t _e=(f.v >> 23) & 0xff;
    int8_t e=_e-127;
    
    if((e) > 0){
        int_fast64_t m=(f.v & 0x7fffff) | 0x800000;
        return m>>(23-(e));
    }
    return 0;
}
#endif