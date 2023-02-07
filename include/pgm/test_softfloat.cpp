#include "softfloat.hpp"
#include <cstdio>
#include <utility>

std::pair<SoftFloat, int> func1(){
    return {SoftFloat(1),2};
}

int main(){

   
    SoftFloat b=(static_cast<SoftFloat>(2)*3-2*2)/6;

    auto i=SoftFloat(2)+b*3;
    print_sf(&i);
    
    auto[f,s]=func1();
    print_sf(&f);

    SoftFloat ff;
    ff.from_float(5.9);
    return 0;
}