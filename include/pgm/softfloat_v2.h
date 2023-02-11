#include<stdint.h>
#include<string.h>
#include<stdio.h>

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

uint_fast8_t softfloat_countLeadingZeros32( uint32_t a )
{
    uint_fast8_t count;

    count = 0;
    if ( a < 0x10000 ) {
        count = 16;
        a <<= 16;
    }
    if ( a < 0x1000000 ) {
        count += 8;
        a <<= 8;
    }
    count += softfloat_countLeadingZeros8[a>>24];
    return count;

}

struct exp16_sig32 softfloat_normSubnormalF32Sig( uint_fast32_t sig )
{
    int_fast8_t shiftDist;
    struct exp16_sig32 z;

    shiftDist = softfloat_countLeadingZeros32( sig ) - 8;
    z.exp = 1 - shiftDist;
    z.sig = sig<<shiftDist;
    return z;

}
uint64_t softfloat_shortShiftRightJam64( uint64_t a, uint_fast8_t dist )
    { return a>>dist | ((a & (((uint_fast64_t) 1<<dist) - 1)) != 0); }

#define softfloat_isSigNaNF32UI( uiA ) ((((uiA) & 0x7FC00000) == 0x7F800000) && ((uiA) & 0x003FFFFF))
#define isNaNF32UI( a ) (((~(a) & 0x7F800000) == 0) && ((a) & 0x007FFFFF))

enum {
    softfloat_flag_inexact   =  1,
    softfloat_flag_underflow =  2,
    softfloat_flag_overflow  =  4,
    softfloat_flag_infinite  =  8,
    softfloat_flag_invalid   = 16
};
THREAD_LOCAL uint_fast8_t softfloat_exceptionFlags = 0;

void softfloat_raiseFlags( uint_fast8_t flags )
{

    softfloat_exceptionFlags |= flags;

}
uint_fast32_t
 softfloat_propagateNaNF32UI( uint_fast32_t uiA, uint_fast32_t uiB )
{
    bool isSigNaNA;

    isSigNaNA = softfloat_isSigNaNF32UI( uiA );
    if ( isSigNaNA || softfloat_isSigNaNF32UI( uiB ) ) {
        softfloat_raiseFlags( softfloat_flag_invalid );
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

uint32_t softfloat_shiftRightJam32( uint32_t a, uint_fast16_t dist )
{

    return
        (dist < 31) ? a>>dist | ((uint32_t) (a<<(-dist & 31)) != 0) : (a != 0);

}

float32_t
 softfloat_roundPackToF32( bool sign, int_fast16_t exp, uint_fast32_t sig )
{
    uint_fast8_t roundingMode;
    bool roundNearEven;
    uint_fast8_t roundIncrement, roundBits;
    bool isTiny;
    uint_fast32_t uiZ;
    union ui32_f32 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    roundingMode = softfloat_roundingMode;
    roundNearEven = (roundingMode == softfloat_round_near_even);
    roundIncrement = 0x40;
    if ( ! roundNearEven && (roundingMode != softfloat_round_near_maxMag) ) {
        roundIncrement =
            (roundingMode
                 == (sign ? softfloat_round_min : softfloat_round_max))
                ? 0x7F
                : 0;
    }
    roundBits = sig & 0x7F;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( 0xFD <= (unsigned int) exp ) {
        if ( exp < 0 ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            isTiny =
                (softfloat_detectTininess == softfloat_tininess_beforeRounding)
                    || (exp < -1) || (sig + roundIncrement < 0x80000000);
            sig = softfloat_shiftRightJam32( sig, -exp );
            exp = 0;
            roundBits = sig & 0x7F;
            if ( isTiny && roundBits ) {
                softfloat_raiseFlags( softfloat_flag_underflow );
            }
        } else if ( (0xFD < exp) || (0x80000000 <= sig + roundIncrement) ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            softfloat_raiseFlags(
                softfloat_flag_overflow | softfloat_flag_inexact );
            uiZ = packToF32UI( sign, 0xFF, 0 ) - ! roundIncrement;
            goto uiZ;
        }
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sig = (sig + roundIncrement)>>7;
    if ( roundBits ) {
        softfloat_exceptionFlags |= softfloat_flag_inexact;
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) {
            sig |= 1;
            goto packReturn;
        }
#endif
    }
    sig &= ~(uint_fast32_t) (! (roundBits ^ 0x40) & roundNearEven);
    if ( ! sig ) exp = 0;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 packReturn:
    uiZ = packToF32UI( sign, exp, sig );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}


float32_t f32_mul( float32_t a, float32_t b )
{
    union ui32_f32 uA;
    uint_fast32_t uiA;
    bool signA;
    int_fast16_t expA;
    uint_fast32_t sigA;
    union ui32_f32 uB;
    uint_fast32_t uiB;
    bool signB;
    int_fast16_t expB;
    uint_fast32_t sigB;
    bool signZ;
    uint_fast32_t magBits;
    struct exp16_sig32 normExpSig;
    int_fast16_t expZ;
    uint_fast32_t sigZ, uiZ;
    union ui32_f32 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF32UI( uiA );
    expA  = expF32UI( uiA );
    sigA  = fracF32UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF32UI( uiB );
    expB  = expF32UI( uiB );
    sigB  = fracF32UI( uiB );
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
    uiZ = softfloat_propagateNaNF32UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infArg:
    if ( ! magBits ) {
        softfloat_raiseFlags( softfloat_flag_invalid );
        uiZ = defaultNaNF32UI;
    } else {
        uiZ = packToF32UI( signZ, 0xFF, 0 );
    }
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = packToF32UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}


uint_fast8_t softfloat_countLeadingZeros64( uint64_t a )
{
    uint_fast8_t count;
    uint32_t a32;

    count = 0;
    a32 = a>>32;
    if ( ! a32 ) {
        count = 32;
        a32 = a;
    }
    /*------------------------------------------------------------------------
    | From here, result is current count + count leading zeros of `a32'.
    *------------------------------------------------------------------------*/
    if ( a32 < 0x10000 ) {
        count += 16;
        a32 <<= 16;
    }
    if ( a32 < 0x1000000 ) {
        count += 8;
        a32 <<= 8;
    }
    count += softfloat_countLeadingZeros8[a32>>24];
    return count;

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
    shiftDist = softfloat_countLeadingZeros64( absA ) - 40;
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

#define indexMultiwordLo( total, n ) ((total) - (n))
#define indexWordHi( total ) 0
#define indexWordLo( total ) ((total) - 1)
#define indexMultiwordHiBut( total, n ) 0
#define wordIncr -1
#define indexMultiwordLoBut( total, n ) (n)
#define indexMultiwordHi( total, n ) 0

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


#define ui64_fromPosOverflow UINT64_C( 0xFFFFFFFFFFFFFFFF )
#define ui64_fromNegOverflow 0
#define ui64_fromNaN         0
#define i64_fromPosOverflow  INT64_C( 0x7FFFFFFFFFFFFFFF )
#define i64_fromNegOverflow  (-INT64_C( 0x7FFFFFFFFFFFFFFF ) - 1)
#define i64_fromNaN          0
#define indexWord( total, n ) ((total) - 1 - (n))
#ifndef softfloat_shiftRightJam96M
/*----------------------------------------------------------------------------
| This function or macro is the same as 'softfloat_shiftRightJamM' with
| 'size_words' = 3 (N = 96).
*----------------------------------------------------------------------------*/
#define softfloat_shiftRightJam96M( aPtr, dist, zPtr ) softfloat_shiftRightJamM( 3, aPtr, dist, zPtr )
#endif

int_fast64_t
 softfloat_roundMToI64(
     bool sign, uint32_t *extSigPtr, uint_fast8_t roundingMode, bool exact )
{
    uint64_t sig;
    uint32_t sigExtra;
    union { uint64_t ui; int64_t i; } uZ;
    int64_t z;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sig =
        (uint64_t) extSigPtr[indexWord( 3, 2 )]<<32
            | extSigPtr[indexWord( 3, 1 )];
    sigExtra = extSigPtr[indexWordLo( 3 )];
    if (
        (roundingMode == softfloat_round_near_maxMag)
            || (roundingMode == softfloat_round_near_even)
    ) {
        if ( 0x80000000 <= sigExtra ) goto increment;
    } else {
        if (
            sigExtra
                && (sign
                        ? (roundingMode == softfloat_round_min)
#ifdef SOFTFLOAT_ROUND_ODD
                              || (roundingMode == softfloat_round_odd)
#endif
                        : (roundingMode == softfloat_round_max))
        ) {
 increment:
            ++sig;
            if ( !sig ) goto invalid;
            if (
                (sigExtra == 0x80000000)
                    && (roundingMode == softfloat_round_near_even)
            ) {
                sig &= ~(uint_fast64_t) 1;
            }
        }
    }
    uZ.ui = sign ? -sig : sig;
    z = uZ.i;
    if ( z && ((z < 0) ^ sign) ) goto invalid;
    if ( sigExtra ) {
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) z |= 1;
#endif
        if ( exact ) softfloat_exceptionFlags |= softfloat_flag_inexact;
    }
    return z;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    return sign ? i64_fromNegOverflow : i64_fromPosOverflow;

}


int_fast64_t f32_to_i64( float32_t a, uint_fast8_t roundingMode, bool exact )
{
    union ui32_f32 uA;
    uint_fast32_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast32_t sig;
    int_fast16_t shiftDist;
#ifdef SOFTFLOAT_FAST_INT64
    uint_fast64_t sig64, extra;
    struct uint64_extra sig64Extra;
#else
    uint32_t extSig[3];
#endif

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signF32UI( uiA );
    exp  = expF32UI( uiA );
    sig  = fracF32UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    shiftDist = 0xBE - exp;
    if ( shiftDist < 0 ) {
        softfloat_raiseFlags( softfloat_flag_invalid );
        return
            (exp == 0xFF) && sig ? i64_fromNaN
                : sign ? i64_fromNegOverflow : i64_fromPosOverflow;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp ) sig |= 0x00800000;
#ifdef SOFTFLOAT_FAST_INT64
    sig64 = (uint_fast64_t) sig<<40;
    extra = 0;
    if ( shiftDist ) {
        sig64Extra = softfloat_shiftRightJam64Extra( sig64, 0, shiftDist );
        sig64 = sig64Extra.v;
        extra = sig64Extra.extra;
    }
    return softfloat_roundToI64( sign, sig64, extra, roundingMode, exact );
#else
    extSig[indexWord( 3, 2 )] = sig<<8;
    extSig[indexWord( 3, 1 )] = 0;
    extSig[indexWord( 3, 0 )] = 0;
    if ( shiftDist ) softfloat_shiftRightJam96M( extSig, shiftDist, extSig );
    return softfloat_roundMToI64( sign, extSig, roundingMode, exact );
#endif

}
const uint16_t softfloat_approxRecip_1k0s[16] = {
    0xFFC4, 0xF0BE, 0xE363, 0xD76F, 0xCCAD, 0xC2F0, 0xBA16, 0xB201,
    0xAA97, 0xA3C6, 0x9D7A, 0x97A6, 0x923C, 0x8D32, 0x887E, 0x8417
};
const uint16_t softfloat_approxRecip_1k1s[16] = {
    0xF0F1, 0xD62C, 0xBFA1, 0xAC77, 0x9C0A, 0x8DDB, 0x8185, 0x76BA,
    0x6D3B, 0x64D4, 0x5D5C, 0x56B1, 0x50B6, 0x4B55, 0x4679, 0x4211
};


uint32_t softfloat_approxRecip32_1( uint32_t a )
{
    int index;
    uint16_t eps, r0;
    uint32_t sigma0;
    uint_fast32_t r;
    uint32_t sqrSigma0;

    index = a>>27 & 0xF;
    eps = (uint16_t) (a>>11);
    r0 = softfloat_approxRecip_1k0s[index]
             - ((softfloat_approxRecip_1k1s[index] * (uint_fast32_t) eps)>>20);
    sigma0 = ~(uint_fast32_t) ((r0 * (uint_fast64_t) a)>>7);
    r = ((uint_fast32_t) r0<<16) + ((r0 * (uint_fast64_t) sigma0)>>24);
    sqrSigma0 = ((uint_fast64_t) sigma0 * sigma0)>>32;
    r += ((uint32_t) r * (uint_fast64_t) sqrSigma0)>>48;
    return r;

}
float32_t f32_div( float32_t a, float32_t b )
{
    union ui32_f32 uA;
    uint_fast32_t uiA;
    bool signA;
    int_fast16_t expA;
    uint_fast32_t sigA;
    union ui32_f32 uB;
    uint_fast32_t uiB;
    bool signB;
    int_fast16_t expB;
    uint_fast32_t sigB;
    bool signZ;
    struct exp16_sig32 normExpSig;
    int_fast16_t expZ;
#ifdef SOFTFLOAT_FAST_DIV64TO32
    uint_fast64_t sig64A;
    uint_fast32_t sigZ;
#else
    uint_fast32_t sigZ;
    uint_fast64_t rem;
#endif
    uint_fast32_t uiZ;
    union ui32_f32 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF32UI( uiA );
    expA  = expF32UI( uiA );
    sigA  = fracF32UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF32UI( uiB );
    expB  = expF32UI( uiB );
    sigB  = fracF32UI( uiB );
    signZ = signA ^ signB;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0xFF ) {
        if ( sigA ) goto propagateNaN;
        if ( expB == 0xFF ) {
            if ( sigB ) goto propagateNaN;
            goto invalid;
        }
        goto infinity;
    }
    if ( expB == 0xFF ) {
        if ( sigB ) goto propagateNaN;
        goto zero;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expB ) {
        if ( ! sigB ) {
            if ( ! (expA | sigA) ) goto invalid;
            softfloat_raiseFlags( softfloat_flag_infinite );
            goto infinity;
        }
        normExpSig = softfloat_normSubnormalF32Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = softfloat_normSubnormalF32Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA - expB + 0x7E;
    sigA |= 0x00800000;
    sigB |= 0x00800000;
#ifdef SOFTFLOAT_FAST_DIV64TO32
    if ( sigA < sigB ) {
        --expZ;
        sig64A = (uint_fast64_t) sigA<<31;
    } else {
        sig64A = (uint_fast64_t) sigA<<30;
    }
    sigZ = sig64A / sigB;
    if ( ! (sigZ & 0x3F) ) sigZ |= ((uint_fast64_t) sigB * sigZ != sig64A);
#else
    if ( sigA < sigB ) {
        --expZ;
        sigA <<= 8;
    } else {
        sigA <<= 7;
    }
    sigB <<= 8;
    sigZ = ((uint_fast64_t) sigA * softfloat_approxRecip32_1( sigB ))>>32;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sigZ += 2;
    if ( (sigZ & 0x3F) < 2 ) {
        sigZ &= ~3;
#ifdef SOFTFLOAT_FAST_INT64
        rem = ((uint_fast64_t) sigA<<31) - (uint_fast64_t) sigZ * sigB;
#else
        rem = ((uint_fast64_t) sigA<<32) - (uint_fast64_t) (sigZ<<1) * sigB;
#endif
        if ( rem & UINT64_C( 0x8000000000000000 ) ) {
            sigZ -= 4;
        } else {
            if ( rem ) sigZ |= 1;
        }
    }
#endif
    return softfloat_roundPackToF32( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF32UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF32UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infinity:
    uiZ = packToF32UI( signZ, 0xFF, 0 );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = packToF32UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}



void print_sf(const void* p);

struct SoftFloat{
    float32_t value;

    SoftFloat()=default;
    SoftFloat(int64_t n){
        value=i64_to_f32(n);
    }
    void from_float(float x){
        memcpy(&(value.v), &x, sizeof(value.v));
    }

    SoftFloat operator*(SoftFloat sf) const{
        SoftFloat c;
        c.value=f32_mul(value, sf.value);
        return c;
    }
    SoftFloat operator*(int32_t k) const{
        SoftFloat sf(k);
        return this->operator*(sf);
    }
    SoftFloat operator/(SoftFloat sf)const{
        SoftFloat c;
        c.value=f32_div(value, sf.value);
        // print_sf(this);
        // printf("/");
        // print_sf(&sf);
        // printf("=");
        // print_sf(&c);
        // printf("\n");

        return c;
    }
    SoftFloat operator/(int k)const{
        SoftFloat sf(k);
        return this->operator/(sf);
    }
    operator int64_t(){
        return f32_to_i64(value, 0, 0);
    }
};

void print_sf(const void* p){
    SoftFloat *sf=(SoftFloat *)p;
    uint32_t _v;
    memcpy(&_v, &(sf->value.v), sizeof(_v));
    printf("%x", _v);
}

#endif