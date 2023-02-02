#include <stdint.h>
#define EXP_BIAS 127
#define MANT_BITS 23


struct SoftFloat{
    int32_t sign;
    uint64_t mant;
    int32_t exp;

    SoftFloat()=default;
    SoftFloat(const SoftFloat &sf):sign(sf.sign), mant(sf.mant), exp(sf.exp){};
    SoftFloat(int32_t s, uint64_t m, int32_t e):sign(s), mant(m), exp(e){};


    SoftFloat normalize() const {
        SoftFloat _sf(sign, mant, exp);
        while(_sf.mant >= 0x1000000UL){
            _sf.exp++;
            _sf.mant >>= 1;
        }
        _sf.mant &= 0x007fffffUL;
        return _sf;
    }

    SoftFloat(int64_t n){
        SoftFloat sf;
        sf.sign=0;
        if(n<0){
            sf.sign=1;
            n*=-1;
        }
        sf.mant=n<<MANT_BITS;
        sf.exp=0;
        SoftFloat _sf=sf.normalize();

        sign=_sf.sign;
        mant=_sf.mant;
        exp=_sf.exp;
    }

    bool operator==(const SoftFloat* sf){
        return (sign==sf->sign && mant==sf->mant && exp==sf->exp);
    }

    bool operator==(int k){
        SoftFloat sf(k);
        return (sign==sf.sign && mant==sf.mant && exp==sf.exp);
    }

    SoftFloat operator*(int k) const {
        SoftFloat result;

        SoftFloat sf(k);
        SoftFloat a=sf.normalize();
        SoftFloat b=normalize();

        result.sign=a.sign ^ b.sign;
        result.mant = (((uint64_t)(a.mant|0x00800000UL) * (uint64_t)(b.mant|0x00800000UL))>>MANT_BITS);
        result.exp=a.exp+b.exp;

        return result.normalize();
    }

    operator int64_t(){
        if(exp>=0){
            int64_t _mant=mant+ 0x800000;
            _mant = _mant>>(MANT_BITS-exp);

            if(sign==1) return -1 * _mant;
            else return _mant;
        }
        return 0;
    }
    void print(){
        printf("sign= %d, mant= %lu, exp= %d\n",sign, mant, exp);
    }
};