#include "softfloat.hpp"
#include <vector>


template<typename Floating>
class Index{
protected:
    struct Seg;

    std::vector<Seg> segs;
public:
    void insert(Floating sl){
        segs.push_back(Seg(sl));
    }


    Floating get(){
        return segs[0].get_slope();
    }

};

template<typename Floating>
struct Index<Floating>::Seg{
    Floating slope;
    Seg(Floating f){
        slope=f;
    }
    Floating get_slope(){
        return slope;
    }
};

int main(){

    uint8_t base[4];
    int8_t exp=-12;
    uint64_t mant=107579;

    base[0]=exp;
//    memcpy(base, &exp, 1*sizeof(uint8_t));
    uint64_t _m=mant & 0x7fffff;
    printf("%x\n", _m); //0000 0001 1010 0100 0011 1011
    base[1]=_m>>16;
    base[2]=_m>>8;
    base[3]=_m;
    
    for(int i=0;i<4;i++){
        printf("%x ", base[i]);
    }
    printf("\n");

    int8_t _exp=base[0];
    uint64_t _mant=0;
    _mant=base[1]<<16 | base[2]<<8 | base[3];
    printf("%d %x\n", _exp, _mant);

    Index<float> i1;
    i1.insert(1.1);
    
    
    SoftFloat sf;
    sf.from_float(i1.get());
    

    Index<SoftFloat> i2;

    i2.insert(sf);
    SoftFloat _sf=i2.get();
    print_sf(&(_sf));

}