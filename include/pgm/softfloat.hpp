#include <cstdint>
#include <cstdio>
#include <cstring>

#define EXP_BIAS 127
#define MANT_BITS 23
#define MANT_HIDE 0x800000
#define MANT_MASK 0x7fffff
#define MANT_UPPER 0x1000000

#pragma once

void print_sf(const void*);

struct SoftFloat{
    int32_t sign;
    uint64_t mant;
    int32_t exp;

    SoftFloat()=default;
    SoftFloat(const SoftFloat &sf):sign(sf.sign), mant(sf.mant), exp(sf.exp){};
    SoftFloat(int32_t s, uint64_t m, int32_t e):sign(s), mant(m), exp(e){};
    SoftFloat(int64_t n){
        sign=0;
        if(n<0){
            sign=1;
            n*=-1;
        }
        mant=n<<MANT_BITS;
        exp=0;
        SoftFloat _sf=normalize_right();

        sign=_sf.sign;
        mant=_sf.mant;
        exp=_sf.exp;
        //printf("int2sf\n    %d\n    ",n);
        //print_sf(this);
    }
    void from_float(float x){
        uint32_t _x;
        memcpy(&(_x), &x, sizeof(float));

        sign=(_x & 0x80000000UL) >>31;
        exp=((_x & 0x7F800000UL)>>23)-127;
        mant=_x & 0x7FFFFFUL;

//        printf("    transform float: %f to sf", x);
//        print_sf(this);
    }

    SoftFloat normalize_right() const {
        SoftFloat _sf(sign, mant, exp);
        while(_sf.mant >= MANT_UPPER){
            _sf.exp++;
            _sf.mant >>= 1;
        }
        _sf.mant &= MANT_MASK;
        return _sf;
    }

    SoftFloat normalize_left() const{
        SoftFloat _sf(sign, mant, exp);
        while(_sf.mant < MANT_HIDE){
            _sf.mant <<= 1;
            _sf.exp--;
        }
        _sf.mant &= MANT_MASK;
        return _sf;
    }

    bool operator==(const SoftFloat* sf){
        return (sign==sf->sign && mant==sf->mant && exp==sf->exp);
    }

    bool operator==(int k){
        SoftFloat sf(k);
        return (sign==sf.sign && mant==sf.mant && exp==sf.exp);
    }

    SoftFloat operator*(SoftFloat sf) const{
        SoftFloat result;

        result.sign=sf.sign ^ sign;
        result.mant = (((uint64_t)(sf.mant|MANT_HIDE) * (uint64_t)(mant|MANT_HIDE))>>MANT_BITS);
        result.exp=sf.exp+exp;

        return result.normalize_right();
    }

    SoftFloat operator*(int k) const {
        SoftFloat sf(k);
        return this->operator*(sf);
    }

    SoftFloat operator/(SoftFloat sf)const{


        //printf("div\n    ");
        //print_sf(this);
        //printf("    ");
        //print_sf(&sf);
        SoftFloat result, a(*this);
        
        a.mant=(a.mant|MANT_HIDE)<<MANT_BITS;
        
        result.sign=sf.sign ^ sign;
        result.exp=a.exp-sf.exp;
        result.mant = (a.mant) / (sf.mant| MANT_HIDE);
        //2^46/2^23
        // normalize
        // result.print();
        return result.normalize_left();
    }


    SoftFloat operator/(int k)const{
        SoftFloat sf(k);
        return this->operator/(sf);
    }

    SoftFloat operator+(SoftFloat sf) const{
        printf("+\n");
        SoftFloat result;

        if(sf.exp > exp){
            result.exp=sf.exp;
            result.sign=sf.sign;
            if(sf.sign==sign){
                result.mant=(sf.mant|MANT_HIDE) + ((mant|MANT_HIDE)>>(sf.exp-exp));
                return result.normalize_right();
            }else{
                result.mant=(sf.mant|MANT_HIDE) - ((mant|MANT_HIDE)>>(sf.exp-exp));
                return result.normalize_left();
            }
        }else if(sf.exp < exp){
            result.exp=exp;
            result.sign=sign;
            if(sf.sign==sign){
                result.mant=(mant|MANT_HIDE) + ((sf.mant|MANT_HIDE)>>(exp-sf.exp));
                return result.normalize_right();
            }else{
                result.mant=(mant|MANT_HIDE) - ((sf.mant|MANT_HIDE)>>(exp-sf.exp));
                return result.normalize_left();
            }
        }else{
            result.exp=sf.exp;
            result.sign=sf.mant>mant ? sf.sign : sign;
            if(sf.sign == sign){
                result.mant=(sf.mant | MANT_HIDE) + (mant | MANT_HIDE);
                return result.normalize_right();
            }
            else{
                if(result.sign==sf.sign){
                    result.mant=(sf.mant | MANT_HIDE) - (mant | MANT_HIDE);
                }else{
                    result.mant=(mant | MANT_HIDE) - (sf.mant | MANT_HIDE);
                }
                return result.normalize_left();
            }
        }
    }
    SoftFloat operator+(int k){
        SoftFloat sf(k);
        return this->operator+(sf);
    }

    SoftFloat operator-(SoftFloat sf) const{
        SoftFloat n_sf(sf);
        if(sf.sign==0) n_sf.sign=1;
        else n_sf.sign=0;

        return this->operator+(n_sf);
    }
    SoftFloat operator-(int k){
        SoftFloat sf(k);
        return this->operator-(sf);
    }

    operator int64_t(){
        if(exp>=0){
            int64_t _mant=mant+ MANT_HIDE;
            _mant = _mant>>(MANT_BITS-exp);

            if(sign==1) return -1 * _mant;
            else return _mant;
        }
        return 0;
    }

    bool operator<(const SoftFloat &sf)const {
        SoftFloat _sf=this->operator-(sf);
        return sf.sign==0;
    }
};

void print_sf(const void* p){
    SoftFloat *sf=(SoftFloat*)p;
    printf("sign= %d, mant= %lx, exp= %d\n",sf->sign, sf->mant, sf->exp);
}
