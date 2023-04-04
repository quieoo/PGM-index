#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

#ifndef soft_float_v2
#define soft_float_v2 1

typedef struct { unsigned int v; } float32_t;
union ui32_f32 { unsigned int ui; float32_t f; };
struct exp16_sig32 { short exp; unsigned int sig; };
#define signF32UI( a ) ((bool) ((unsigned int) (a)>>31))
#define expF32UI( a ) ((short) ((a)>>23) & 0xFF)
#define fracF32UI( a ) ((a) & 0x007FFFFF)
#define softfloat_shiftRightJam32(a, dist) ((dist) < 31) ? (a)>>(dist) | ((unsigned int) ((a)<<(-(dist) & 31)) != 0) : ((a) != 0)
#define softfloat_shortShiftRightJam64( a, dist ) (a)>>(dist) | (((a) & (((unsigned long long) 1<<(dist)) - 1)) != 0)
#define softfloat_isSigNaNF32UI( uiA ) ((((uiA) & 0x7FC00000) == 0x7F800000) && ((uiA) & 0x003FFFFF))
#define isNaNF32UI( a ) (((~(a) & 0x7F800000) == 0) && ((a) & 0x007FFFFF))
#define packToF32UI( sign, exp, sig ) (((unsigned int) (sign)<<31) + ((unsigned int) (exp)<<23) + (sig))
#define defaultNaNF32UI 0x7FC00000

const unsigned char softfloat_countLeadingZeros8[256] = {
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

struct exp16_sig32 softfloat_normSubnormalF32Sig( unsigned int sig )
{
    signed char shiftDist=0;
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


unsigned int
 softfloat_propagateNaNF32UI( unsigned int uiA, unsigned int uiB )
{
    bool isSigNaNA;
    isSigNaNA = softfloat_isSigNaNF32UI( uiA );
    if ( isSigNaNA || softfloat_isSigNaNF32UI( uiB ) ) {
        // softfloat_raiseFlags( softfloat_flag_invalid );
        if ( isSigNaNA ) return uiA | 0x00400000;
    }
    return (isNaNF32UI( uiA ) ? uiA : uiB) | 0x00400000;
}


float32_t
 softfloat_roundPackToF32( bool sign, short exp, unsigned int sig )
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
    sig &= ~(unsigned int) (! ((sig & 0x70) ^ 0x40) & 1);
    if ( ! sig ) exp = 0;
    uZ.ui = packToF32UI( sign, exp, sig );
 uiZ:
    return uZ.f;
}


float32_t f32_mul( float32_t a, float32_t b )
{
    bool signA;
    short expA;
    unsigned int sigA;

    bool signB;
    short expB;
    unsigned int sigB;
    
    bool signZ;
    short expZ;
    unsigned int sigZ;
    
    
    unsigned int magBits;
    struct exp16_sig32 normExpSig;
    union ui32_f32 uZ;

    signA = signF32UI( a.v );
    expA  = expF32UI( a.v );
    sigA  = fracF32UI( a.v );
    signB = signF32UI( b.v );
    expB  = expF32UI( b.v );
    sigB  = fracF32UI( b.v );
    signZ = signA ^ signB;

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
    expZ = expA + expB - 0x7F;
    sigA = (sigA | 0x00800000)<<7;
    sigB = (sigB | 0x00800000)<<8;
    sigZ = softfloat_shortShiftRightJam64( (unsigned long long) sigA * sigB, 32 );
    if ( sigZ < 0x40000000 ) {
        --expZ;
        sigZ <<= 1;
    }
    // return softfloat_roundPackToF32( signZ, expZ, sigZ );
    if((unsigned int)expZ >= 0xfd){
        if(expZ < 0){
            sigZ=softfloat_shiftRightJam32(sigZ, -expZ);
            expZ=0;
        }else if((0xFD < expZ) || (0x80000000 <= sigZ + 0x40)){
            uZ.ui=packToF32UI( signZ, 0xFF, 0 ) - ! 0x40;
            return uZ.f;
        }
    }
    sigZ= (sigZ + 0x40)>>7;
    sigZ &= ~(unsigned int) (! ((sigZ & 0x70) ^ 0x40) & 1);
    if ( ! sigZ ) expZ = 0;
    uZ.ui = packToF32UI( signZ, expZ, sigZ );
    return uZ.f;


propagateNaN:
    uZ.ui = softfloat_propagateNaNF32UI( a.v, b.v );
    goto uiZ;
infArg:
    if ( ! magBits ) {
        // softfloat_raiseFlags( softfloat_flag_invalid );
        uZ.ui = defaultNaNF32UI;
    } else {
        uZ.ui = packToF32UI( signZ, 0xFF, 0 );
    }
    goto uiZ;
 zero:
    uZ.ui = packToF32UI( signZ, 0, 0 );
 uiZ:
    return uZ.f;
}


float32_t i64_to_f32( int64_t a )
{
    bool sign;
    unsigned long long absA;
    signed char shiftDist;
    union ui32_f32 u;
    unsigned int sig;

    sign = (a < 0);
    absA = sign ? -(unsigned long long) a : (unsigned long long) a;

    unsigned int a32=absA;
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
                    sign, 0x95 - shiftDist, (unsigned int) absA<<shiftDist )
                : 0;
        return u.f;
    } else {
        shiftDist += 7;
        sig = 
            (shiftDist < 0)
                ? softfloat_shortShiftRightJam64( absA, -shiftDist )
                : (unsigned int) absA<<shiftDist;
        return softfloat_roundPackToF32( sign, 0x9C - shiftDist, sig );
    }
}

#define i64_fromNaN          0
#define i64_fromPosOverflow  INT64_C( 0x7FFFFFFFFFFFFFFF )
#define i64_fromNegOverflow  (-INT64_C( 0x7FFFFFFFFFFFFFFF ) - 1)
#define indexMultiwordLo( total, n ) ((total) - (n))
#define indexWordHi( total ) 0
#define indexWordLo( total ) ((total) - 1)
#define indexMultiwordHiBut( total, n ) 0
#define wordIncr -1
#define indexMultiwordLoBut( total, n ) (n)
#define indexMultiwordHi( total, n ) 0
#define indexWord( total, n ) ((total) - 1 - (n))


void
 softfloat_shortShiftRightJamM(
     uint_fast8_t size_words,
     const uint32_t *aPtr,
     uint_fast8_t dist,
     uint32_t *zPtr
 )
{
    uint_fast8_t uNegDist;
    unsigned int index, lastIndex;
    uint32_t partWordZ, wordA;

    uNegDist = -dist;
    index = indexWordLo( size_words );
    lastIndex = indexWordHi( size_words );
    wordA = aPtr[index];
    partWordZ = wordA>>dist;
    if ( partWordZ<<dist != wordA ) partWordZ |= 1;
    while ( index != lastIndex ) {
        wordA = aPtr[index + wordIncr];
        zPtr[index] = wordA<<(uNegDist & 31) | partWordZ;
        index += wordIncr;
        partWordZ = wordA>>dist;
    }
    zPtr[index] = partWordZ;

}
void
 softfloat_shiftRightJamM(
     uint_fast8_t size_words,
     const uint32_t *aPtr,
     uint32_t dist,
     uint32_t *zPtr
 )
{
    uint32_t wordJam, wordDist, *ptr;
    uint_fast8_t i, innerDist;

    wordJam = 0;
    wordDist = dist>>5;
    if ( wordDist ) {
        if ( size_words < wordDist ) wordDist = size_words;
        ptr = (uint32_t *) (aPtr + indexMultiwordLo( size_words, wordDist ));
        i = wordDist;
        do {
            wordJam = *ptr++;
            if ( wordJam ) break;
            --i;
        } while ( i );
        ptr = zPtr;
    }
    if ( wordDist < size_words ) {
        aPtr += indexMultiwordHiBut( size_words, wordDist );
        innerDist = dist & 31;
        if ( innerDist ) {
            softfloat_shortShiftRightJamM(
                size_words - wordDist,
                aPtr,
                innerDist,
                zPtr + indexMultiwordLoBut( size_words, wordDist )
            );
            if ( ! wordDist ) goto wordJam;
        } else {
            aPtr += indexWordLo( size_words - wordDist );
            ptr = zPtr + indexWordLo( size_words );
            for ( i = size_words - wordDist; i; --i ) {
                *ptr = *aPtr;
                aPtr += wordIncr;
                ptr += wordIncr;
            }
        }
        ptr = zPtr + indexMultiwordHi( size_words, wordDist );
    }
    do {
        *ptr++ = 0;
        --wordDist;
    } while ( wordDist );
 wordJam:
    if ( wordJam ) zPtr[indexWordLo( size_words )] |= 1;
}
long long
 softfloat_roundMToI64(
     bool sign, uint32_t *extSigPtr)
{
    uint64_t sig;
    uint32_t sigExtra;
    union { uint64_t ui; int64_t i; } uZ;
    int64_t z;

    sig =
        (uint64_t) extSigPtr[indexWord( 3, 2 )]<<32
            | extSigPtr[indexWord( 3, 1 )];
    sigExtra = extSigPtr[indexWordLo( 3 )];
    
    if ( sigExtra && (sign ? true : false)) {
        ++sig;
        if ( !sig ) goto invalid;
    }
    
    uZ.ui = sign ? -sig : sig;
    z = uZ.i;
    if ( z && ((z < 0) ^ sign) ) goto invalid;
    return z;

 invalid:
    return sign ? i64_fromNegOverflow : i64_fromPosOverflow;

}
long long f32_to_i64(float32_t f){
    union ui32_f32 uA;
    unsigned int uiA;
    bool sign;
    short exp;
    unsigned int sig;
    short shiftDist;
    unsigned int extSig[3];

    uA.f = f;
    uiA = uA.ui;
    sign = signF32UI( uiA );
    exp  = expF32UI( uiA );
    sig  = fracF32UI( uiA );

    shiftDist = 0xBE - exp;
    if ( shiftDist < 0 ) {
        // softfloat_raiseFlags( softfloat_flag_invalid );
        return
            (exp == 0xFF) && sig ? i64_fromNaN
                : sign ? i64_fromNegOverflow : i64_fromPosOverflow;
    }

    if ( exp ) sig |= 0x00800000;
    extSig[indexWord( 3, 2 )] = sig<<8;
    extSig[indexWord( 3, 1 )] = 0;
    extSig[indexWord( 3, 0 )] = 0;
    if ( shiftDist ) softfloat_shiftRightJamM( 3, extSig, shiftDist, extSig );

    return softfloat_roundMToI64( sign, extSig);

}
#endif