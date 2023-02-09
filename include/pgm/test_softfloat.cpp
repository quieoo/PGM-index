#include "softfloat.hpp"
#include <cstdio>
#include <utility>

std::pair<SoftFloat, int> func1(){
    return {SoftFloat(1),2};
}

int main(){
    SoftFloat sf(3956);
    print_sf(&sf);

    sf.from_float(1.123);
    print_sf(&sf);

    return 0;
}