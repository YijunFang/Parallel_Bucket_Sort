#include <iostream>
#include <random>
#include <thread>
#include <chrono>

#include "BucketSort.h"

int main() {

    unsigned int totalNumbers = 5;//000000;
    //unsigned int printIndex =   2590000;

    
    std::mt19937 mt(10);
    std::uniform_int_distribution<unsigned int> dist(1, 100);
    
    BucketSort pbs;

    // insert random numbers into the sort object
    for (unsigned int i=0; i < totalNumbers; ++i) {
        pbs.numbersToSort.push_back(dist(mt));
    }

    // call sort giving the number of cores available.
    std::cout << "<======= Parallel Version =======>" << std::endl;
    auto start = std::chrono::steady_clock::now();
    pbs.sort(std::thread::hardware_concurrency());
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time to sort Single " << totalNumbers;
    std::cout<< " numbers = " << std::chrono::duration <double, std::milli> (end - start).count();
    std::cout<< " [ms]" << std::endl;

    // print certain values from the buckets
    std::cout << "Demonstrating that all the numbers that start with 1 come first" << std::endl;
    for (auto num : pbs.numbersToSort) std::cout << num<< std::endl;
    
    /*std::cout << pbs.numbersToSort[0] << " " << pbs.numbersToSort[printIndex - 10000]
        << " " << pbs.numbersToSort[printIndex] << " " << pbs.numbersToSort[pbs.numbersToSort.size() - 1]
        << std::endl;
        
    std::cout << "<======= Single  Version =======>" << std::endl;
    start = std::chrono::steady_clock::now();
    pbs.sortSingle(std::thread::hardware_concurrency());
    end = std::chrono::steady_clock::now();
    std::cout << "Time to sort Single " << totalNumbers;
    std::cout<< " numbers = " << std::chrono::duration <double, std::milli> (end - start).count();
    std::cout<< " [ms]" << std::endl;

    // print certain values from the buckets
    std::cout << "Demonstrating that all the numbers that start with 1 come first" << std::endl;
    for (auto num : pbs.numbersToSort) std::cout << num<< std::endl;
    std::cout << pbs.numbersToSort[0] << " " << pbs.numbersToSort[printIndex - 10000]
        << " " << pbs.numbersToSort[printIndex] << " " << pbs.numbersToSort[pbs.numbersToSort.size() - 1]
        << std::endl;
	*/
	
}
