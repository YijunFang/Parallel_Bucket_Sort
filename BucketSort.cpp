#include "BucketSort.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#define PRINTTHING 0

bool aLessB(const unsigned int& x, const unsigned int& y, unsigned int pow) {
        
        // if the two numbers are the same then one is not less than the other
        if (x == y) 
            return false; 
    
        unsigned int a = x;
        unsigned int b = y;
    
        // work out the digit we are currently comparing on. 
        if (pow == 0) {
                while (a / 10 > 0) {
                        a = a / 10; 
                }   
                while (b / 10 > 0) {
                        b = b / 10;
                }
        } else {
            
  
            //here a need to be greater than 
            while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
                        a = a / 10;
                }
                while (b / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
                        b = b / 10;
                }
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

	/*	
	this lambda function put (numCores) number of vector (called buckets)
	onto buckets_all_core where each vector consist of 10 sub-vector.
	each sub-vector contain integers 
	whose most-significant-bit == sub-vector's position in upper vector
	*/
    auto seperateToCoreBucket = [&numbersToSort, &buckets_all_core, &mutex_buckets, pow]
        (unsigned int start_index,unsigned int end_index) {
        
        std::vector<std::vector<unsigned int>> buckets;
        
        for (int j = 0; j <= 9; ++j) {
            buckets.push_back(std::vector<unsigned int>()); }

        for (unsigned int i = start_index; i <= end_index; ++i) {
            unsigned int a = numbersToSort[i];
            while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) a /= 10; a=a%10;
            buckets.at(a).push_back(numbersToSort[i]); }


        std::lock_guard<std::mutex> lock(mutex_buckets);
        buckets_all_core.push_back(buckets);
    };

	//create a vector of thread which seperate the original 
	//vector into numCores number vector (ordered by most significant bit)
    for (unsigned int i = 1; i <= numCores; ++i){
        if(i == numCores) endIndex = numbersToSort.size()-1;
        else endIndex = i*eachLength-1;
        
        // std::thread newThread {seperateToCoreBucket,nextIndex, endIndex};  newThread.join();
        thread_vector.push_back(std::thread(seperateToCoreBucket,nextIndex, endIndex));
        nextIndex = endIndex+1;
    }

	//start the thread
    for(auto &t: thread_vector)
        t.join(); 
    
	//debug print
    if(PRINTTHING){
    	int bucket = 0;
        for(auto c:buckets_all_core){
		    std::cout << std::endl; std::cout <<"bucket "<< bucket<< "\n"; 
		    int flag = 0; 
		    if(c.size()!= 0) flag = 1;
		        for(auto v: c){
		            for(auto val: v) 
		                std::cout << val<<" "; 
		                if(flag){flag = 0; std::cout << std::endl; } 
		        }bucket++;
		}std::cout << "==========================="<<pow<< std::endl; 
	}

	//clear thread vector and original input vector
    thread_vector.clear();
    numbersToSort.clear();

	//this vector contain sub-vector where each sub-vector contain integers 
	//whose most-significant-bit == sub-vector's position in upper vector
    std::vector<std::vector<unsigned int>> msb_Bucket;
    for (int j = 0; j <= 9; ++j) 
        msb_Bucket.push_back(std::vector<unsigned int>()); 

    int msb = 10;
	//this lambda function put all intergers into prev-mentioned msb_Bucket
    std::function<void()> rearrange_with_msb;
    rearrange_with_msb = [numCores,pow, &numbersToSort, &msb,&rearrange_with_msb, 
    					&buckets_all_core, &msb_Bucket, &mutex_buckets]() {
        int index = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_buckets);
                index = --msb;
        }
        if (index < 0) return;
		
		//a reperence vector to avoid collision
        auto &vector_same_msb = msb_Bucket.at(index);
        for (auto fourCoreVector : buckets_all_core) {
            vector_same_msb.insert(vector_same_msb.end(), fourCoreVector.at(index).begin(), fourCoreVector.at(index).end());
        }

        if (index > 0)
            std::sort(msb_Bucket.at(index).begin(),msb_Bucket.at(index).end(), [](const unsigned int& x, const unsigned int& y){
                 return aLessB(x,y,1); 
            } );
        // mySort(numCores, vector_same_msb, pow+1);
        rearrange_with_msb();
    };

	//create a vector of thread which rearranges accordng to most significant bit
    for (unsigned int i = 0; i < numCores; ++i){
        // std::thread newThread {rearrange_with_msb};  newThread.join();
        thread_vector.push_back(std::thread(rearrange_with_msb));
    }
    
   	//start the thread 
    for(auto &t: thread_vector)
        t.join();

	//debug infortation
    if(PRINTTHING){
    	int index = 0; for(auto &c: msb_Bucket){
        std::cout << index<<"| "; int flag = 0;
                    if(c.size()!= 0) flag = 1;
                    for(auto v: c)std::cout << v<<" ";
                    if(flag){flag = 0; std::cout << std::endl; std::cout << "---------"<< std::endl; }
                    index++; 
      	} std::cout << "==========================="<< std::endl; 
    }
    thread_vector.clear();
	
	//put all numbers into original vector
    for (auto v = msb_Bucket.begin(); v != msb_Bucket.end(); ++v) 
        numbersToSort.insert(numbersToSort.end(), v->begin(), v->end());

    return;
}

void BucketSort::sort(unsigned int numCores) {
    mySort(numCores, numbersToSort, 0);
}
