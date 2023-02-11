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
    uint8_t base[4];

    SoftFloat()=default;
    SoftFloat(uint64_t m, int8_t e){
        base[0]=e;
        base[1]=m>>16;
        base[2]=m>>8;
        base[3]=m;
    }
    uint64_t mant()const{
        uint64_t m;
        m=base[1]<<16 | base[2]<<8 | base[3];
        return m;
    }
    int8_t exp()const{
        return (int8_t)(base[0]);
    }


    SoftFloat normalize_right(uint64_t mant, int8_t exp) const{
        while(mant >= MANT_UPPER){
            exp++;
            mant >>= 1;
        }
        mant &= MANT_MASK;
        return SoftFloat(mant, exp);
    }



    SoftFloat(uint64_t n){
        uint64_t mant=n<<MANT_BITS;
        int8_t exp=0;
        //SoftFloat _sf=normalize_right(mant, exp);
        bool guard_bit0;
        
        while(mant >= MANT_UPPER){
            guard_bit0=mant & 0x1;
            exp++;
            mant >>= 1;
        }
        mant &= MANT_MASK;
        if(guard_bit0)
            mant++;

        base[0]=exp;
        base[1]=mant>>16;
        base[2]=mant>>8;
        base[3]=mant;
        //printf("int2sf\n    %d\n    ",n);
        //print_sf(this);
    }


    void from_float(float x){
        uint32_t _x;
        memcpy(&(_x), &x, sizeof(float));

        int8_t exp=((_x & 0x7F800000UL)>>23)-127;
        uint64_t mant=_x & 0x7FFFFFUL;
        SoftFloat _sf=SoftFloat(mant, exp);
        for(int i=0;i<4;i++){
            base[i]=_sf.base[i];
        }
        printf("    transform float: %f(%x) to sf", x, _x);
        print_sf(this);
    }
    /*
    171: 0001 0111 0001
    16f: 0001 0110
    */



    SoftFloat normalize_left(uint64_t mant, int8_t exp) const{
        while(mant < MANT_HIDE){
            mant <<= 1;
            exp--;
        }
        mant &= MANT_MASK;
        return SoftFloat(mant, exp);
    }

    bool operator==(const SoftFloat sf){
        for(int i=0; i<4;i++){
            if(base[i] != sf.base[i])
                return false;
        }
        return true;
    }

    bool operator==(int k){
        SoftFloat sf(k);
        return this->operator==(sf);
    }

/*
key= 626212262 2141363536 282741617 1736571722 1087907283 2145871997
exp=13
mant=11100011000100101101 00111010110010001100000
grs=101


*/
    SoftFloat operator*(SoftFloat sf) const{
        //printf("mul\n    ");
        //print_sf(this);
        //printf("    ");
        //print_sf(&sf);

        uint64_t m1=mant(), m2=sf.mant(), m3=0;
        int8_t e1=exp(), e2=sf.exp(), e3;
        m3=((m1|MANT_HIDE) * (m2|MANT_HIDE));

        uint64_t round_bits=m3 & 0xffffff;
        uint8_t grs=round_bits>>22; // get guard bit and round bit
        // get sticky bit (there are any "1" in all of left 21 bits)
        for(int i=0;i<21;i++){
            if(round_bits % 2){
                grs=grs<<1;
                grs++;
                break;
            }
            round_bits = round_bits >> 1;
        }
        bool round_up=false;
        /*
            grs: 00..0  roundded=0
                 00..1  0<roundded<0.25
                 01..0  roundded=0.25
                 01..1  0.25<roundded<0.5
                 10..0  roundded=0.5
                 10..1  0.5<roundded<0.75
                 11..0  roundded=0.5
                 11..1  0.75<roundded
        principle: Round to nearest, but if half-way in-between then round to nearest even
        */
       /*
       up
       011 up
       110 up
       010 down 
       111 up
       101 
       */
        if(grs==6 || grs==3 || grs==7 || grs==5)  round_up=true;    //110 011 111
        m3=m3>>23;
        if(round_up){
            m3++;
        }
        e3=e1+e2;
        return normalize_right(m3, e3);
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
        uint64_t m1=mant(), m2=sf.mant(), m3=0;
        int8_t e1=exp(), e2=sf.exp(), e3;
        e3=e1-e2;
        m3=((m1|MANT_HIDE)<<MANT_BITS) / (m2|MANT_HIDE);        
        return normalize_left(m3, e3);
    }


    SoftFloat operator/(int k)const{
        SoftFloat sf(k);
        return this->operator/(sf);
    }
    operator int64_t(){
        uint64_t m=mant();
        int8_t e=exp();
        if(e>=0){
            //round up
            /*
            uint32_t m=mant;
            bool round_up=true;
            for(int i=0;i<(MANT_BITS - exp);i++){
                if(!(m%2)){
                    round_up=false;
                    break;
                }
                m/=2;
            }*/
            uint64_t _mant=m + MANT_HIDE;
            _mant = _mant>>(MANT_BITS-e);
            //if(round_up)
             //   _mant++;
            return _mant;
        }
        return 0;
    }
};

void print_sf(const void* p){
    SoftFloat *sf=(SoftFloat*)p;
    printf("mant= %lx, exp= %d\n",sf->mant(), sf->exp());
}
