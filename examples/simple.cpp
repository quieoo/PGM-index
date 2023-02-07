/*
 * This example shows how to index and query a vector of random integers with the PGM-index.
 * Compile with:
 *   g++ simple.cpp -std=c++17 -I../include -o simple
 * Run with:
 *   ./simple
 */

#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "pgm/pgm_index.hpp"
#include <algorithm>
#include <chrono>

int main(int argc, char** argv) {

    int n=atoi(argv[1]);
    // Generate some random data
    std::cout<<"generate data..."<<std::endl;
    std::vector<int> data(n);
    std::generate(data.begin(), data.end(), std::rand);
    std::sort(data.begin(), data.end());

    std::cout<<"build index..."<<std::endl;
    std::chrono::steady_clock::time_point build0 = std::chrono::steady_clock::now();
    // Construct the PGM-index
    const int epsilon = 128; // space-time trade-off parameter
    pgm::PGMIndex<int, epsilon> index(data);
    std::chrono::steady_clock::time_point build1 = std::chrono::steady_clock::now();
    std::cout<<"    use "<<std::chrono::duration_cast<std::chrono::microseconds> (build1 - build0).count() << "[us]"<<std::endl;

    std::cout<<"run query test..."<<std::endl;
    // Query the PGM-index
    int success=0;
    std::chrono::steady_clock::time_point query0 = std::chrono::steady_clock::now();
    for(int i=0; i<data.size(); i++){
        auto q=data[i];
        auto range=index.search(q);
        auto lo=data.begin()+range.lo;
        auto hi=data.begin()+range.hi;
        auto got=*std::lower_bound(lo, hi, q);
        if(q==got){
            success++;
        }
    }
    std::chrono::steady_clock::time_point query1= std::chrono::steady_clock::now();
    std::cout<<"    query success "<<success<<"/"<<n<<", with "<<std::chrono::duration_cast<std::chrono::microseconds> (query1-query0).count() << "[us]"<<std::endl;


    return 0;
}