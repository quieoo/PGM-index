#include "softfloat.hpp"
#include <cstdio>
#include <utility>

std::pair<SoftFloat, int> func1(){
    return {SoftFloat(1),2};
}

int main(){
    float x=1.01282441616;
    uint32_t _x;
    memcpy(&(_x), &x, sizeof(x));
    printf("%.20f:%x\n",x, _x);

    float xx=9747*x;
    uint32_t _xx;
    memcpy(&(_xx), &xx, sizeof(xx));
    printf("%.12f:%x\n",xx, _xx);

   
    SoftFloat b=(static_cast<SoftFloat>(2)*3-2*2)/6;    

    auto i=SoftFloat(2)+b*3;
    print_sf(&i);
    
    auto[f,s]=func1();
    print_sf(&f);

    SoftFloat ff;
    ff.from_float(1.132368);
    return 0;
}