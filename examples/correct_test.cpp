#include "pgm/pgm_index.hpp"
#include <algorithm>
#include <cstdio>


int main(int argc, char** argv) {
    int n=atoi(argv[1]);
    std::vector<int> data;
    for(int i=0; i<n; i++){
        data.push_back(i);
    }

    const int epsilon = 128; // space-time trade-off parameter
    pgm::PGMIndex<int, epsilon, 4, float> index(data);

    std::vector<int>_data;
    pgm::PGMIndex<int, epsilon, 4, SoftFloat> _index(_data);
    
    

    int i=131;
    auto range=_index.search(i);
    auto lo=data.begin()+range.lo;
    auto hi=data.begin()+range.hi;
    auto got=*std::lower_bound(lo, hi, i);
    if(i!=got){
        printf("failed to search %d\n", i);
    }
    
}