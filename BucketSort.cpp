#include "BucketSort.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>

// numcores cannot be 0
void BucketSort::sort(unsigned int numCores) {

    // stage 1 small buckets
    auto smallbuckets = std::vector<std::shared_ptr<std::vector<std::vector<unsigned int>>>>();
    std::mutex mutex;

    unsigned int workload = numbersToSort.size() / numCores;

    std::function<void(unsigned int, unsigned int, unsigned int)> put_small_buckets;
    put_small_buckets = [this, &smallbuckets, &mutex]
            (unsigned int id, unsigned int start, unsigned int end) {

        auto buckets = std::make_shared<std::vector<std::vector<unsigned int>>>(std::vector<std::vector<unsigned int>>());
        for (int j = 0; j < 10; ++j) {
            buckets->push_back(std::vector<unsigned int>());
        }

        for (unsigned int i = start; i < end; ++i) {
            unsigned int a = numbersToSort[i];
             while (a / 10 > 0) a /= 10;
			
            (*buckets)[a].push_back(numbersToSort[i]);
        }

        std::lock_guard<std::mutex> lock(mutex);
        smallbuckets.push_back(buckets);
    };


    // create a container of threads
    //vector of share_ptr to thread
    std::vector<std::shared_ptr<std::thread>> containerOfThreads;

    // create threads and add them to the container.
    for (unsigned int i = 1; i < numCores; ++i) {
        // start the thread.
        unsigned int start = workload * i;
        unsigned int end = workload * (i + 1);
        if(i == numCores - 1) end = this->numbersToSort.size() ;
        auto sharePtr = std::make_shared<std::thread>(put_small_buckets, i, start, end);
        containerOfThreads.push_back(sharePtr);
    }

    // do part of the processing on this processor.
    put_small_buckets(0, 0, workload);

    // join all the threads back together.
    for (auto t : containerOfThreads) t->join();

    numbersToSort.clear();
    // stage 2: small buckets into big sub buckets and sort the big sub buckets
    auto bigbuckets = std::vector<std::shared_ptr<std::vector<unsigned int>>>();
    for (int j = 0; j < 10; ++j) {
        bigbuckets.push_back(std::make_shared<std::vector<unsigned int>>(std::vector<unsigned int>()));
    }

    int current_index = 10;

    std::function<void()> collect_and_sort;
    collect_and_sort = [this, &smallbuckets, &current_index, &mutex, &collect_and_sort, &bigbuckets] () {
        mutex.lock();
        int index = --current_index;
        mutex.unlock();
        if (index < 0) return;
        auto mybucket = bigbuckets[index];
        for (auto i = smallbuckets.begin(); i != smallbuckets.end(); ++i) {
            mybucket->insert(mybucket->end(), (*(*i))[index].begin(), (*(*i))[index].end());
        }
        // no need to sort 0s
        if (index > 0) std::sort(mybucket->begin(),mybucket->end());
        collect_and_sort();
    };

    // create a container of threads
    containerOfThreads.clear();

    // create threads and add them to the container.
    for (unsigned int i = 1; i < numCores; ++i) {
        containerOfThreads.push_back(std::make_shared<std::thread>(collect_and_sort));
    }
    // do part of the processing on this processor.
    collect_and_sort();

    // join all the threads back together.
    for (auto t : containerOfThreads) t->join();

    // stage 3: join all number into original
    // single threaded stage 3 (tested to be faster then the parallel version)
    for (auto i = bigbuckets.begin(); i != bigbuckets.end(); ++i) {
        numbersToSort.insert(numbersToSort.end(), (*i)->begin(), (*i)->end());
    }
}

/*
void BucketSort::sortSingle(unsigned int numCores) {
    std::sort(numbersToSort.begin(),numbersToSort.end(),
    [](const unsigned int& x, const unsigned int& y){
        unsigned int a = x;
        unsigned int b = y;
        // work out the starting digit of each number
        while (a / 10 > 0) a /= 10;
        while (b / 10 > 0) b /= 10;
        // if the digits are equal than sort on value.
        if (a == b)
            return x < y;
        else
            return a < b;
    } );
}

*/

bool aLessB(const unsigned int& x, const unsigned int& y, unsigned int pow) {
    	std::cout << "sort "<< x << " and " << y <<" at digit: " << pow << std::endl; 
    	
    	// if the two numbers are the same then one is not less than the other
        if (x == y) 
        	return false; 
    
        unsigned int a = x;
        unsigned int b = y;
    
        // work out the digit we are currently comparing on. 
        if (pow == 0) {
        
        	std::cout << "inti: a = "<< a << " , b = " << b <<" at digit: " << pow << std::endl; 
                while (a / 10 > 0) {
                        a = a / 10; 
                }   
                while (b / 10 > 0) {
                        b = b / 10;
                }
        	std::cout << "now: a = "<< a << " , b = " << b <<" at digit: " << pow <<"\n"<< std::endl; 
        } else {
        	std::cout << "inti: a = "<< a << " , b = " << b <<" at digit: " << pow << std::endl; 
        	
        	//make sure a and b are pow+1 digits
        	//check if: 3 / 10 >= 10
        	std::cout << "check if: "<< a<<" / 10 >= "<< std::round(std::pow(10,pow))<< std::endl;
        	std::cout << "check if: "<< b<<" / 10 >= "<< std::round(std::pow(10,pow))<< std::endl;
        	
        	//here a need to be greater than 
        	while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
        		
                        a = a / 10;
                }
                while (b / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
                        b = b / 10;
                }
           	std::cout << "now: a = "<< a << " , b = " << b <<" at digit: " << pow <<"\n"<< std::endl; 
        }

        if (a == b)
                return aLessB(x,y,pow + 1);  // recurse if this digit is the same using the original number
        else
                return a < b;
}

//Single thread
void BucketSort::sortSingle(unsigned int numCores){
        std::sort(numbersToSort.begin(),numbersToSort.end(), [](const unsigned int& x, const unsigned int& y){
                return aLessB(x,y,0);
        } );
}



