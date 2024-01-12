#include "vma_structs/bag.hpp"
#include "test_structs/vec_bag.hpp"
#include "test_structs/index_set.hpp"
#include "vma_structs/index_set_2.hpp"
#include <iostream>
#include <chrono>

int n = 10000000;

// #define bag
// #define index_set_test
// #define index_set_test_vma
#define index_set_benchmark

#ifdef index_set_benchmark
#define benchmark
// #define index_set
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
bool isEven(int num) {return num%2==0 ? true : false;}

void increment(int &num) {num++;}

void print(int item) {
            // printf("%f\n",std::to_string(item));
            std::cout << item << std::endl;
        }

int main(int argc, char** argv) {

#ifdef index_set
    Index_Set_Control<int> std_struct;
    #define std_test
#endif

#ifdef index_set_vma
    Index_Set<int> vma_struct;
    #define vma_test
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
    for(int i = 0; i < 7; i++) {
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
    for(int i = 0; i < 7; i++) {
        std_struct.insert(i*16);
    }
    std_struct.for_each(print);
    printf("loops: %d\n",std_struct.returnLoops());
    // printf("found %d? %s",25,std_struct.find(26));
    // std_struct.remove(25);
    // std_struct.for_each(print);
    #endif
#endif


#ifdef bag
    Bag<int> vma_struct;
    Vec_Bag<int> std_struct;
    #define std_test
    #define vma_test
    #define benchmark
#endif

#ifdef bagtest
    std::cout << "made bag" << std::endl;
    bag.insert(5);
    std::cout << "5" << std::endl;
    bag.insert(12);
    std::cout << "12" << std::endl;
    int *temp = (bag.fetch_if(isEven));
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
    std::vector<int> temp;
    for(int i = 0; i < n; i++) {
        temp.push_back(rand());
    }
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    #ifdef vma_test
    m_StartTime = std::chrono::system_clock::now();
    for(int i = 0; i < n; i++) {
        vma_struct.insert(temp[i]);
    }
    // vma_struct.removeif(isEven);
    // for(int i = 0; i < n; i+=2) {
    //     vma_struct.insert(i);
    // }
    // vma_struct.clear();
    m_EndTime = std::chrono::system_clock::now();
    double vma_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    printf("vma_struct time: %f with loops: %d\n", vma_struct_time, vma_struct.returnLoops());
    #endif
    #ifdef std_test
    m_StartTime = std::chrono::system_clock::now();
    for(int i = 0; i < n; i++) {
        std_struct.insert(temp[i]);
    }
    // std_struct.removeif(isEven);
    // // std_struct.for_each(print);
    // for(int i = 0; i < n; i+=2) {
    //     std_struct.insert(i);
    // }
    // std_struct.clear();
    m_EndTime = std::chrono::system_clock::now();
    double std_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    // bag.for_each(print); 
    printf("std_struct time: %f with loops: %d\n", std_struct_time, std_struct.returnLoops());
    #endif
#endif
}
