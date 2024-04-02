#include "vma_structs/bag.hpp"
#include "test_structs/vec_bag.hpp"
#include "test_structs/index_set.hpp"
#include "vma_structs/index_set_2.hpp"
#include "murmurhash.hpp"


#include <iostream>
#include <chrono>
#include <random>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>
#include <string>

#include <sys/stat.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <cassert>

#include <unistd.h>

#include <sys/mman.h>


// size_t n = 1000000000;
std::vector<size_t> nvals = {102, 65536, 1048576, 16777216, 67108864, 100000000};

//test
bool isEven(size_t num) {return num%2==0 ? true : false;}

bool isOne(size_t num) {return num == 1 ? true : false;}

void increment(size_t &num) {num++;}

void print(size_t item) {
            // printf("%f\n",std::to_string(item));
            std::cout << item << std::endl;
        }
void count_incore(void* pdata, size_t len) {
    unsigned char result[len/4096];
    mincore(pdata, len, result);
    size_t count_incore(0);
    for(size_t i = 0; i<len/4096; ++i) {
      if(result[i] & 1) {
        count_incore++;
      }
    }
    std::cout << count_incore << std::endl;
}

template <typename T>

class Naive_Index_Set {

public:
  //TODO add a free list of IDs
  size_t insert(const T& value) {
    auto [itr, flag] = m_key_id.insert({value, m_vec_iterators.size()});
    if(flag) {
      m_vec_iterators.push_back(itr);
      return m_vec_iterators.size() - 1;
    }
    return itr->second;
  }
  size_t size() const { return m_vec_iterators.size(); }
  void clear() {m_key_id.clear(); m_vec_iterators.clear(); }
  const T& lookup(size_t idx) const { return m_vec_iterators[idx]->first; }
private:
  std::map<T, size_t> m_key_id;
  std::vector<typename std::map<T, size_t>::iterator> m_vec_iterators;
};

int main(int argc, char* argv[]) {
srand(1);

// #define bag
// #define index_set_test
// #define index_set_test_vma
#define index_set_benchmark
// #define mremap_test

#ifdef bag
    #define std_test
    #define vma_test
    #define benchmark
    // #define benchmark_loop
#endif

#ifdef index_set_benchmark
    #define benchmark
    #define delete_test
    size_t delete_window = (size_t)atoi(argv[5]);
    char data = argv[3][0];
    #define uset
    #define index_set
    #define index_set_vma
    #define n_index_set
    // if(argv[3][0] == 's') {
    //     #define sorted
    //     std::cout << "sorted" << std::endl;
    // }
    // else if(argv[3][0] == 'r') {
    //     #define random
    //     std::cout << "random" << std::endl;
    // }
    // else if(argv[3][0] == 'd') {
    //     #define half_duplicates
    //     std::cout << "duplicates" << std::endl;
    // }
#endif

#ifdef index_set_test
    #define index_set_test
    #define index_set
#endif

#ifdef index_set_test_vma
    #define index_set_vma
    #define index_set_test
#endif




#ifdef index_set
    Index_Set_Control<size_t> std_struct;
    #define std_test
#endif

#ifdef index_set_vma
    // Index_Set<size_t> vma_struct;
    // Index_Set<size_t>* vma_struct = new Index_Set<size_t>(std::stoi(argv[1]), std::stoi(argv[2]));
    Index_Set<size_t> vma_struct(std::stoi(argv[1]), std::stoi(argv[2]));
    // std::cout << "level scale: " << std::stoi(argv[1]) << " init probe: " << std::stoi(argv[2]) << std::endl;
    #define vma_test
#endif

#ifdef n_index_set
    Naive_Index_Set<size_t> naive_struct;
#endif

#ifdef uset
    std::unordered_set<int, mmh::hash<>> std_uset;
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
    int n = std::stoi(argv[4]);
    // unsigned int gen_size = (unsigned int)-1;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, RAND_MAX); // define the range
    // std::cout << "finished gen making" << std::endl;
    for(size_t i = 1; i <= nvals.at(n); i++) {
        if(data == 's')
            temp.push_back(i);
        if(data == 'r')
            temp.push_back((size_t)distr(gen));
        if(data == 'd') {
            if(i == (nvals.at(n)/2)) {
                for(size_t i = 1; i < nvals.at(n)/2; i++) {
                    temp.push_back(i);
                }
                break;
            }
            temp.push_back(i);
        }
        // if(i%100000 == 0) {
        //     std::cout << i << std::endl;
        // }
    }
    std::vector<size_t>::const_iterator first = temp.end() - delete_window;
    std::vector<size_t>::const_iterator last = temp.end();
    std::vector<size_t> delete_temp(first,last);
    // std::cout << "finished data making" << std::endl;

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
        // std::cout << i << std::endl;
        #ifdef delete_test
        if(i >= delete_window) {
            // vma_struct.remove_index(i-delete_window);
            vma_struct.remove(temp[i-delete_window]);
        }
        // std::cout << i << " and removing " << i - delete_window << std::endl;
        #endif
    }
    // std::cout << sbrk(0) << std::endl;
    
    
    // printf("residency rate: %d\n", vma_struct.residency_rate());
    // vma_struct.removeif(isEven);
    // vma_struct.removeif(isOne);
    // if(vma_struct.find(1)) {
    //     std::cout << "there's an issue with this " << std::endl;
    // }
    // for(size_t i = 0; i < n; i+=2) {
    //     vma_struct.insert(i);
    // }
    // printf("found vals after delete: %d\n", vma_struct.find(100001));
    // vma_struct.clear();
    m_EndTime = std::chrono::system_clock::now();
    // vma_struct.count_incore();int
    // #ifdef delete_test
    // vma_struct.correctnessCheck(delete_temp);
    // #else
    // vma_struct.correctnessCheck(temp);
    // #endif
    // vma_struct.printLevelInfo();
    vma_struct.clear();
    // for(size_t i = 0; i < n; i++) {
    //     if(!(vma_struct.find(temp[i]))) {
    //         printf("issue found: %d not inserted\n", temp[i]);
    //     }
    // }
    // std::cout << vma_struct.get_global_max() << std::endl;
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
        #ifdef delete_test
        if(i >= delete_window) {
            std_struct.remove(temp[i-delete_window]);
        }
        // std::cout << "i: " << i << " max probe: " << std_struct.get_max_probe() << std::endl;
        #endif
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

    // std::vector<size_t> problems = std_struct.get_problems();
    // for(int j = 0; j < temp.size(); j++) {
        // std::cout << temp[j] << std::endl;
        // if(std::find(problems.begin(), problems.end(), temp[j]) != problems.end())
        //     std::cout << "val: " << temp[j] << " index: " << j << std::endl;
    // }
    // std::cout << problems.size() << std::endl;

    // #ifdef delete_test
    // std_struct.correctnessCheck(delete_temp);
    // #else
    // std_struct.correctnessCheck(temp);
    // #endif
    // std_struct.count_incore();
    std_struct.clear();
    // std::cout << std_struct.get_global_max() << std::endl;
    double std_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    // bag.for_each(print); 
    printf("std_struct time: %f\n", std_struct_time);
    #endif
    #ifndef delete_test
    #ifdef uset
    m_StartTime = std::chrono::system_clock::now();
    auto it = std_uset.begin();
    for(size_t i = 0; i < nvals.at(n); i++) {
        std_uset.insert(temp[i]);
        #ifdef delete_test
        if(i > delete_window) {
            std_uset.erase(it);
            it++;
        }
        #endif
    }
    m_EndTime = std::chrono::system_clock::now();
    double std_uset_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    printf("unordered set time: %f\n", std_uset_time);
    #endif
    #ifdef n_index_set
    m_StartTime = std::chrono::system_clock::now();
    for(size_t i = 0; i < nvals.at(n); i++) {
        naive_struct.insert(temp[i]);
    }
    m_EndTime = std::chrono::system_clock::now();
    double n_struct_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
    printf("naive index set time: %f\n", n_struct_time);
    #endif
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
    #ifdef vma_test
    for(size_t n : nvals) {
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
#ifdef mremap_test
void *pdata = mmap(0, 1024*1024,
                             PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    assert(pdata);
    count_incore(pdata, 1024*1024);
    int* pi = (int*)pdata;
    // *pi = 1;
    for(int i = 0; i < 1024*1024/sizeof(int); i++) {
        pi[i] = i;
    }
    count_incore(pdata, 1024*1024);
    pdata = mremap(pdata, 1024*1024, 2*1024*1024, MREMAP_MAYMOVE);
    assert(pdata);
    count_incore(pdata, 2*1024*1024);
#endif
return 0;
}
