#include "BucketSort.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#define PRINTTHING 0

bool aLessB(const unsigned int& x, const unsigned int& y, unsigned int pow) {
        // std::cout << "sort "<< x << " and " << y <<" at digit: " << pow << std::endl; 
        
        // if the two numbers are the same then one is not less than the other
        if (x == y) 
            return false; 
    
        unsigned int a = x;
        unsigned int b = y;
    
        // work out the digit we are currently comparing on. 
        if (pow == 0) {
        
            // std::cout << "inti: a = "<< a << " , b = " << b <<" at digit: " << pow << std::endl; 
                while (a / 10 > 0) {
                        a = a / 10; 
                }   
                while (b / 10 > 0) {
                        b = b / 10;
                }
            // std::cout << "now: a = "<< a << " , b = " << b <<" at digit: " << pow <<"\n"<< std::endl; 
        } else {
            // std::cout << "inti: a = "<< a << " , b = " << b <<" at digit: " << pow << std::endl; 
            
            //make sure a and b are pow+1 digits
            //check if: 3 / 10 >= 10
            // std::cout << "check if: "<< a<<" / 10 >= "<< std::round(std::pow(10,pow))<< std::endl;
            // std::cout << "check if: "<< b<<" / 10 >= "<< std::round(std::pow(10,pow))<< std::endl;
            
            //here a need to be greater than 
            while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
                
                        a = a / 10;
                }
                while (b / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
                        b = b / 10;
                }
            // std::cout << "now: a = "<< a << " , b = " << b <<" at digit: " << pow <<"\n"<< std::endl; 
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



void mySort(unsigned int numCores, std::vector<unsigned int>& numbersToSort, unsigned int pow){
    if(numbersToSort.size()<10){
        std::sort(numbersToSort.begin(),numbersToSort.end(), [](const unsigned int& x, const unsigned int& y){
            return aLessB(x,y,0); } ); return; }


    //store each vector into bucketsOfCore
    std::vector<std::vector<std::vector<unsigned int>>> buckets_all_core ;
    std::vector<std::thread> thread_vector;
    std::mutex mutex_buckets;
    //seperate the whole vector into four parts
    unsigned int eachLength = numbersToSort.size()/numCores ;
    unsigned int nextIndex = 0;
    unsigned int endIndex = 0;

    auto seperateToCoreBucket = 
        [&numbersToSort, &buckets_all_core, &mutex_buckets, pow](unsigned int start_index,unsigned int end_index) {
        std::vector<std::vector<unsigned int>> buckets;
        // std::cout << start_index<< "  "<< end_index<< std::endl;
        for (int j = 0; j <= 9; ++j) {
            buckets.push_back(std::vector<unsigned int>()); }

        // auto start = std::chrono::steady_clock::now();
        for (unsigned int i = start_index; i <= end_index; ++i) {
            unsigned int a = numbersToSort[i];
            while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) a /= 10; a=a%10;
            buckets.at(a).push_back(numbersToSort[i]); }


        std::lock_guard<std::mutex> lock(mutex_buckets);
        buckets_all_core.push_back(buckets);
    };

    for (unsigned int i = 1; i <= numCores; ++i){
        if(i == numCores) endIndex = numbersToSort.size()-1;
        else endIndex = i*eachLength-1;
        
        // std::thread newThread {seperateToCoreBucket,nextIndex, endIndex};  newThread.join();
        thread_vector.push_back(std::thread(seperateToCoreBucket,nextIndex, endIndex));
        
        nextIndex = endIndex+1;
    }

    for(auto &t: thread_vector){
        t.join(); 
    }

    if(PRINTTHING){int bucket = 0;
        for(auto c:buckets_all_core){std::cout << std::endl; std::cout <<"bucket "<< bucket<< "\n"; int flag = 0; if(c.size()!= 0) flag = 1;
            for(auto v: c){// std::cout<<" this bucket contains: "<< v.size() << std::endl;
                for(auto val: v) 
                    std::cout << val<<" "; if(flag){flag = 0; std::cout << std::endl; } } bucket++; }
                    std::cout << "==========================="<<pow<< std::endl; }


    thread_vector.clear();
    numbersToSort.clear();

    std::vector<std::vector<unsigned int>> msb_Bucket;
    for (int j = 0; j <= 9; ++j) 
        msb_Bucket.push_back(std::vector<unsigned int>()); 


    // std::function<void(int)> rearrange_with_msb = 
    //     [numCores,pow, &numbersToSort, &buckets_all_core, &msb_Bucket, &mutex_buckets](int vector_index) {
    //     auto corebucket = buckets_all_core[vector_index];    
    //     int i = 0;
    //     for(auto indexVector = corebucket.begin();indexVector !=  corebucket.end() && i<10; ++indexVector){

    //         std::vector<unsigned int> newVector;

    //         // mutex_buckets.lock();
    //         newVector.insert(newVector.end(), indexVector->begin(), indexVector->end());
    //         // mutex_buckets.unlock();

    //         mutex_buckets.lock();
    //         msb_Bucket.at(i).insert(msb_Bucket.at(i).end(), newVector.begin(), newVector.end());
    //         mutex_buckets.unlock();

    //         i++;
    //     }
    // };

////////////////////////////////////////////////////
    int msb = 10;

    std::function<void()> rearrange_with_msb ;
    rearrange_with_msb = [numCores,pow, &numbersToSort, &msb,&rearrange_with_msb, &buckets_all_core, &msb_Bucket, &mutex_buckets]() {
        int index = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_buckets);
                index = --msb;
        }
        if (index < 0) return;

        auto &vector_same_msb = msb_Bucket.at(index);
        for (auto fourCoreVector : buckets_all_core) {
            vector_same_msb.insert(vector_same_msb.end(), fourCoreVector.at(index).begin(), fourCoreVector.at(index).end());
        }

        // no need to sort 0s
        if (index > 0)// std::sort(mybucket->begin(),mybucket->end());
            std::sort(msb_Bucket.at(index).begin(),msb_Bucket.at(index).end(), [](const unsigned int& x, const unsigned int& y){
                 return aLessB(x,y,1); 
            } );
        // mySort(numCores, vector_same_msb, pow+1);
        rearrange_with_msb();
    };




    // auto start = std::chrono::steady_clock::now();

    for (unsigned int i = 0; i < numCores; ++i){
        // std::thread newThread {rearrange_with_msb};  newThread.join();
        thread_vector.push_back(std::thread(rearrange_with_msb));
    }
    for(auto &t: thread_vector){
        t.join();
    }

    if(PRINTTHING){int index = 0; for(auto &c: msb_Bucket){
        std::cout << index<<"| "; int flag = 0;
                    if(c.size()!= 0) flag = 1;
                    for(auto v: c){std::cout << v<<" "; }
                    if(flag){flag = 0; std::cout << std::endl; std::cout << "---------"<< std::endl; }
                    index++; } std::cout << "==========================="<< std::endl; }


    thread_vector.clear();

    // for(auto &vector_same_msb : msb_Bucket){
    //     if (vector_same_msb .size()< 10){        
    //         std::sort(vector_same_msb.begin(),vector_same_msb.end(), [](const unsigned int& x, const unsigned int& y){
    //             return aLessB(x,y,1);} ); }
    //     else {
    //         std::cout<< "===recursion sort this: ";
    //         for(auto v: vector_same_msb){ 
    //                 std::cout << v<<" ";
    //             }std::cout << std::endl;

    //         mySort(numCores, vector_same_msb, pow+1);
    //     }
    // }


    for (auto v = msb_Bucket.begin(); v != msb_Bucket.end(); ++v) 
        numbersToSort.insert(numbersToSort.end(), v->begin(), v->end());
    
    return;

}

void BucketSort::sort(unsigned int numCores) {
    mySort(numCores, numbersToSort, 0);
}

/*

void BucketSort::sort(unsigned int numCores) {

    // stage 1 small buckets
    auto smallbuckets = std::vector<std::shared_ptr<std::vector<std::vector<unsigned int>>>>();
    std::mutex mutex;

    unsigned int workload = numbersToSort.size() / numCores;

    //push back a share+Ptr pointing to 9 buckets of number
    std::function<void(unsigned int, unsigned int, unsigned int)> put_small_buckets;
    put_small_buckets = [this, &smallbuckets, &mutex](unsigned int id, unsigned int start, unsigned int end) {

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
        if(i == numCores - 1) 
            end = this->numbersToSort.size() ;
        auto sharePtr = std::make_shared<std::thread>(put_small_buckets, i, start, end);
        containerOfThreads.push_back(sharePtr);
    }

    // do part of the processing on this processor.
    put_small_buckets(0, 0, workload);

    // join all the threads back together.
    for (auto t : containerOfThreads) t->join();

    numbersToSort.clear();

    // stage 2: small buckets into big sub buckets and sort the big sub buckets
    //vector of share_Ptr poing to vector of int
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
        numbersToSort.insert(numbersToSort.end(), (*i)