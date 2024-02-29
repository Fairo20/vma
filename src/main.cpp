#include "vma_structs/bag.hpp"
#include "test_structs/vec_bag.hpp"
#include "test_structs/index_set.hpp"
#include "vma_structs/index_set_2.hpp"


#include <iostream>
#include <chrono>
#include <random>


// size_t n = 1000000000;
std::vector<size_t> nvals = {10000000, 100000000, 1000000000};

// #define bag
// #define index_set_test
// #define index_set_test_vma
#define index_set_benchmark

#ifdef bag
    #define std_test
    #define vma_test
    #define benchmark
    // #define benchmark_loop
#endif

#ifdef index_set_benchmark
    #define benchmark
    #define index_set
    #define index_set_vma
#endif

#ifdef index_set_test
    #define index_set_test
    #define index_set
#endif

#ifdef index_set_test_vma
    #define index_set_vma
    #define index_set_test
#endif

//test
bool isEven(size_t num) {return num%2==0 ? true : false;}

void increment(size_t &num) {num++;}

void print(size_t item) {
            // printf("%f\n",std::to_string(item));
            std::cout << item << std::endl;
        }

int main(size_t argc, char** argv) {
srand(1);
#ifdef index_set
    Index_Set_Control<size_t> std_struct;
    #define std_test
#endif

#ifdef index_set_vma
    Index_Set<size_t> vma_struct;
    #define vma_test
#endif

#ifdef bag
    Bag<size_t> vma_struct;
    // Vec_Bag<size_t> std_struct;
    std::vector<size_t> std_struct;
#endif

#ifdef index_set_test
    #ifdef vma_test
    printf("finished creation\n");
    fflush(stdout);
    // vma_struct.insert(5);
    // vma_struct.insert(5+4096);
    // vma_struct.insert(5+8192);
    // vma_struct.insert(5+16384);
    // vma_struct.insert(5+32768);
    for(size_t i = 0; i < 7; i++) {
        vma_struct.insert(i*16);
    }
    vma_struct.for_each(print);
    // printf("found %d? %s",25,vma_struct.find(26));
    printf("loops: %d\n",vma_struct.returnLoops());
    // vma_struct.remove(25);
    // vma_struct.for_each(print);
    #endif
    #ifdef std_test
    printf("finished creation\n");
    // std_struct.insert(5);
    // std_struct.insert(5+4096);
    // std_struct.insert(5+8192);
    // std_struct.insert(5+16384);
    // std_struct.insert(5+32768);
    const char temp[19] = "temp value to hash";
    // const uint64_t key[4] = {1, 2, 3, 4};
    // auto hsh = HighwayHash64(key, temp, sizeof(temp));
    // HH_ALIGNAS(32) const HHKey key = {1, 2, 3, 4};
    // char in[8] = {1};
    // HHResult64 result;  // or HHResult128 or HHResult256
    // InstructionSets::Run<HighwayHash>(key, temp, sizeof(temp), &result);
    // std::cout << result << std::endl;
    for(size_t i = 0; i < 7; i++) {
        std_struct.insert(i*16);
    }
    std_struct.for_each(print);
    printf("loops: %d\n",std_struct.returnLoops());
    // printf("found %d? %s",25,std_struct.find(26));
    // std_struct.remove(25);
    // std_struct.for_each(print);
    #endif
#endif

#ifdef bagtest
    std::cout << "made bag" << std::endl;
    bag.insert(5);
    std::cout << "5" << std::endl;
    bag.insert(12);
    std::cout << "12" << std::endl;
    size_t *temp = (bag.fetch_if(isEven));
    // std::cout << "fetch" << std::endl;
    std::cout << "addr: " << temp << " value: " << *temp << std::endl;
    bag.removeif(isEven);
    bag.insert(18);
    bag.insert(11);
    bag.insert(2);
    std::cout << "print old amount then increment" << std::endl;
    bag.for_each(print); 
    bag.for_each(increment);
    std::cout << "print new amount" << std::endl;
    bag.for_each(print); //should be 6 19 12 3
    bag.clear();
    bag.for_each(print);
    std::cout << "cleared bag" << std::endl;
    bag.insert(18);
    bag.insert(11);
    bag.for_each(print);
#endif

    //begin bag time
#ifdef benchmark
    std::vector<size_t> temp;
    int n = 0;
    for(size_t i = 0; i < nvals.at(n); i++) {
        temp.push_back(i);
        // if(i%100000 == 0) {
        //     std::cout << i << std::endl;
        // }
    }
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    #ifdef vma_test
    std::string output;
    m_StartTime = std::chrono::system_clock::now();
    for(size_t i = 0; i < nvals.at(n); i++) {
        vma_struct.insert(temp[i]);
        // printf("iteration %d: %f\n", i, temp[i]);
        // vma_struct.for_each(print);
        // std::cin >> output;
    }
    // std::cout << sbrk(0) << std::endl;
    // for(size_t i = 0; i < n; i++) {
    //     if(!(vma_struct.find(temp[i]))) {
    //         printf("issue found: %d not inserted\n", temp[i]);
    //     }
    // }
    
    // printf("residency rate: %d\n", vma_struct.residency_rate());
    // vma_struct.removeif(isEven);
    // for(size_t i = 0; i < n; i+=2) {
    //     vma_struct.insert(i);
    // }
    // printf("found vals after delete: %d\n", vma_struct.find(100001));
    // vma_struct.clear();
    m_EndTime = std::chrono::system_clock::now();
    vma_struct.printLevelInfo();
    // for(size_t i = 0; i < vma_struct.get_num_levels(); i++) {
    //     // printf("residency rate of level %d: %d / %d = %d\n", i, vma_struct.res_vec.at(i), vma_struct.res_vec_2.at(i), (double)vma_struct.res_vec.at(i) / (double)vma_struct.res_vec_2.at(i));
    //     std::cout << "residency rate of level " << i << ": " << vma_struct.res_vec.at(i) << " / " << vma_struct.res_vec_2.at(i) << " = " << (double)vma_struct.res_vec.at(i) / (double)vma_struct.res_vec_2.at(i) << std::endl;
    // }
    double vma_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    printf("vma_struct time: %f\n", vma_struct_time);
    #endif
    #ifdef std_test
    m_StartTime = std::chrono::system_clock::now();
    for(size_t i = 0; i < nvals.at(n); i++) {
        std_struct.insert(temp[i]);
        // std_struct.push_back(temp[i]);
    }
    // for(size_t i = 0; i < n; i++) {
    //     if(!(std_struct.find(temp[i]))) {
    //         printf("issue found: %d not inserted\n", temp[i]);
    //     }
    // }
    // printf("residency rate: %d\n", std_struct.residency_rate());
    // std_struct.removeif(isEven);
    // std_struct.erase(std::remove_if(std_struct.begin(), std_struct.end(), isEven), std_struct.end());
    // for(size_t i = 0; i < n; i+=2) {
    //     std_struct.insert(i);
    // }
    // std_struct.clear();
    m_EndTime = std::chrono::system_clock::now();
    double std_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    // bag.for_each(print); 
    printf("std_struct time: %f\n", std_struct_time);
    // std_struct.clear();
    // vma_struct.clear();
    #endif
    #endif

#ifdef benchmark_loop
    std::vector<size_t> temp;
    for(size_t i = 0; i < nvals.back(); i++) {
        temp.push_back(i);
    }
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    for(size_t n : nvals) {
        #ifdef vma_test
        m_StartTime = std::chrono::system_clock::now();
        std::string output;
        for(size_t i = 0; i < n; i++) {
            vma_struct.insert(temp[i]);
            // printf("iteration %d: %f\n", i, temp[i]);
            // vma_struct.for_each(print);
            // std::cin >> output;
        }
        // for(size_t i = 0; i < n; i++) {
        //     if(!(vma_struct.find(temp[i]))) {
        //         printf("issue found: %d not inserted\n", temp[i]);
        //     }
        // }
        for(size_t i = 0; i < n/100; i++) {
            vma_struct.pop_back();
        }
        // for(size_t i = 0; i < vma_struct.get_num_levels(); i++) {
        //     printf("residency rate of level %d: %d / %d\n", i, vma_struct.res_vec.at(i), vma_struct.res_vec_2.at(i));
        // }
        // printf("residency rate: %d\n", vma_struct.residency_rate());
        // vma_struct.removeif(isEven);
        // for(size_t i = 0; i < n; i+=2) {
        //     vma_struct.insert(i);
        // }
        m_EndTime = std::chrono::system_clock::now();
        double vma_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
        printf("%d vma_struct time: %f\n", n, vma_struct_time);
        vma_struct.clear();
        #endif
        #ifdef std_test
        m_StartTime = std::chrono::system_clock::now();
        for(size_t i = 0; i < n; i++) {
            std_struct.push_back(temp[i]);
        }
        // for(size_t i = 0; i < n; i++) {
        //     if(!(std_struct.find(temp[i]))) {
        //         printf("issue found: %d not inserted\n", temp[i]);
        //     }
        // }
        for(size_t i = 0; i < n/100; i++) {
            std_struct.pop_back();
        }
        // printf("residency rate: %d\n", std_struct.residency_rate());
        // std_struct.removeif(isEven);
        // std_struct.erase(std::remove_if(std_struct.begin(), std_struct.end(), isEven), std_struct.end());
        // for(size_t i = 0; i < n; i+=2) {
        //     std_struct.push_back(i);
        // }
        
        m_EndTime = std::chrono::system_clock::now();
        double std_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
        // bag.for_each(print); 
        printf("%d std_struct time: %f\n", n, std_struct_time);
        std_struct.clear(); 
    }
    #endif
#endif
}
