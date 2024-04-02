#pragma once
#include <functional>
#include <memory>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
#include "../murmurhash.hpp"


#define _BSD_SOURCE
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

template <typename Item, typename Alloc = std::allocator<Item>, typename Hash = std::hash<Item>, typename equals = std::equal_to<Item>>
class Index_Set {
    using self_type  = Bag<Item, Alloc>;
    using value_type = Item;

    public:
        Index_Set(int level_scale = 4, int init_probe = 2) : level_scale(level_scale), init_probe(init_probe) {
            // std::cout << "2 level scale: " << level_scale << " init probe: " << init_probe << std::endl;
            mset = (value_type*)mmap(NULL, capacity*sizeof(value_type), mprot, mflags, 0, 0);
            if (mset == MAP_FAILED) {
                handle_error("mmap");   
            }
            // printf("made mset\n");
            fflush(stdout);
            tombstones = (int*)mmap(NULL, capacity*sizeof(int), mprot, mflags, -1, 0);
            // printf("made tombstones\n");
            fflush(stdout);
            for(int i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
        }

        ~Index_Set(){
            munmap(mset, capacity);
            munmap(tombstones, capacity);
        }

        void insert(value_type val) {
            int curr_probe = init_probe;
            uint64_t curr_cap = init_cap;
            int curr_level = 0;
            auto hsh = mmh::hash{}(val);
            uint64_t i = hsh%curr_cap;
            uint64_t fingerprint = hsh >> 34;
            if(fingerprint == 0) {
                fingerprint++;
            }
            uint64_t offset = 0;
            uint64_t insert_index;
            bool index_found = false;
            int max_probe = 0;
            //level indexing: as levels increase
            for(curr_level; curr_level < num_levels; curr_level++) {
                uint64_t temp = curr_cap + offset;
                unsigned int j = 0;
                uint64_t ij = i+j;
                for(; ij < temp && j < curr_probe; j++,ij++) {
                    global_probe_max++;
                    if(tombstones[ij] == 0) {    //if empty
                        if(!index_found) {
                            insert_index = ij;
                            index_found = true;
                        }
                        break;
                    }
                    else if(tombstones[ij] == -1) {   //if tombstone
                        if(!index_found) {
                            insert_index = ij;
                            index_found = true;
                        }
                    }
                    else {                      //if filled
                        if(tombstones[ij] == fingerprint) {  //if fingerprint similar
                            if(mset[ij] == val) {           //if value already exists in set
                                index_found = false;
                                break;
                            }
                        }
                    }
                    if(j + 1 >= curr_probe && curr_level + 1 >= num_levels) {       //resize if on last level and hit max probe
                        if(!index_found) {
                            resize(curr_cap);
                        }
                        continue;
                    }
                }
                if(!index_found) {
                    offset += curr_cap;
                    curr_cap*=level_scale;
                    hsh = rotr64(hsh, 4);
                    fingerprint = hsh >> 34;
                    if(fingerprint == 0) {
                        fingerprint++;
                    }
                    i = (hsh & (curr_cap - 1)) + offset;
                }
            }   
            if(index_found) {
                // std::cout << insert_index << " | " << capacity << std::endl << std::flush;
                mset[insert_index] = val;
                tombstones[insert_index] = fingerprint; 
                size++;
            }
        }

        bool find(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            int curr_level = 0;
            unsigned mask = (1 << 8) - 1;
            auto hsh = mmh::hash{}(val);
            unsigned int i = hsh%curr_cap;
            int fingerprint = hsh >> 34;
            if(fingerprint == 0) {
                fingerprint++;
            }
            int offset = 0;
            int max_probe = 0;
            //level indexing: as levels increase
            for(curr_level; curr_level < num_levels; curr_level++) {
                size_t temp = curr_cap + offset;
                size_t j = 0;
                size_t ij = i+j;
                for(; ij < temp && j < curr_probe; j++,ij++) {
                    global_probe_max++;
                    if(tombstones[ij] == 0) {    //if empty
                        return false;
                    }
                    else {                      //if filled
                        if(tombstones[ij] == fingerprint) {  //if fingerprint similar
                            if(mset[ij] == val) {           //if value already exists in set
                                return true;
                            }
                        }
                    }
                }
                offset += curr_cap;
                curr_cap*=level_scale;
                hsh = rotr64(hsh, 4);
                fingerprint = hsh >> 34;
                if(fingerprint == 0) {
                    fingerprint++;
                }
                i = (hsh & (curr_cap - 1)) + offset;
            }   
            return false;
        }

        void remove_index(size_t index) {
            ~mset[index];
            tombstones[index] = -1;
            size--;
        }

        void remove(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            int curr_level = 0;
            unsigned mask = (1 << 8) - 1;
            auto hsh = mmh::hash{}(val);
            unsigned int i = hsh%curr_cap;
            int fingerprint = hsh >> 34;
            if(fingerprint == 0) {
                fingerprint++;
            }
            int offset = 0;
            int insert_index = -2;
            int max_probe = 0;
            //level indexing: as levels increase
            for(curr_level; curr_level < num_levels; curr_level++) {
                size_t temp = curr_cap + offset;
                size_t j = 0;
                size_t ij = i+j;
                for(; ij < temp && j < curr_probe; j++,ij++) {
                    global_probe_max++;
                    if(tombstones[ij] == 0) {    //if empty
                        break;
                    }
                    else if(tombstones[ij] > 0) {                      //if filled
                        if(tombstones[ij] == fingerprint) {  //if fingerprint similar
                            if(mset[ij] == val) {           //if value already exists in set
                                ~mset[ij];
                                tombstones[ij] = -1;
                                size--;
                                break;
                            }
                        }
                    }
                }
                offset += curr_cap;
                curr_cap*=level_scale;
                hsh = rotr64(hsh, 4);
                fingerprint = hsh >> 34;
                if(fingerprint == 0) {
                    fingerprint++;
                }
                i = (hsh & (curr_cap - 1)) + offset;
            }   
        }

        void clear() {
            for(size_t i = 0; i < capacity; i++) {
                ~mset[i];
                tombstones[i] = 0;
            }
            madvise(&mset+(size_t)sysconf(_SC_PAGESIZE), capacity*sizeof(value_type) - (size_t)sysconf(_SC_PAGESIZE), MADV_DONTNEED);
            madvise(&tombstones+(size_t)sysconf(_SC_PAGESIZE), capacity*sizeof(int) - (size_t)sysconf(_SC_PAGESIZE), MADV_DONTNEED);
            capacity = (size_t)sysconf(_SC_PAGESIZE);
            size = 0;
        };

        template <typename Function>
        void removeif(Function fn) {
            int temp = 0;
            int bound = size;
            for(int i = 0; i < capacity && temp < bound; i++) {
                if(tombstones[i] > 0) {
                    temp++;
                    if(fn(mset[i])){
                        tombstones[i] = -1;
                        ~mset[i];
                        size--;
                    }
                }
            }
        }

        template <typename Function>
        void for_each(Function fn) {
            int temp = 0;
            int bound = size;
            for(int i = 0; i < capacity && temp < bound; i++) {
                if(tombstones[i] > 0){
                    fn(mset[i]);
                    temp++;
                }
            }
        };

        void printLevelInfo() {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            int curr_level = 0;
            int count = 0;
            int offset = 0;
            std::vector<int> totalCount;
            std::vector<int> capTrack;
            for(curr_level; curr_level < num_levels; curr_level++) {
                for(int j = 0; j < curr_cap; j++) {
                    if(tombstones[j + offset] >= 2) {
                        count++;
                    }
                    // else if(curr_level == 3) {
                    //     std::cout << "level " << curr_level << ": " << j + offset << std::endl;
                    // }
                }
                totalCount.push_back(count);
                capTrack.push_back(curr_cap);
                count = 0;
                offset += curr_cap;
                curr_cap*=level_scale;
            }
            for(int i = 0; i < totalCount.size(); i++) {
                std::cout << "level " << i << ": " << totalCount.at(i) << " / " << capTrack.at(i) << " = " << (double)totalCount.at(i) / (double)capTrack.at(i) << std::endl;
            }
            // size_t mincore_len = (capacity + sysconf(_SC_PAGESIZE) - 1) / sysconf(_SC_PAGESIZE);
            // size_t mincore_count = 0;
            // unsigned char *vec = (unsigned char*)malloc(mincore_len);
            // mincore(mset, capacity, vec);
            // for(int i = 0; i < mincore_len; i++) {
            //     if(vec[i] == 1) {
            //         mincore_count++;
            //     }
            //     else {
            //         std::cout << i << std::endl;
            //     }
            // }
            // std::cout << "mincore: " << mincore_count << " / mincore_len: " << mincore_len << " = " << mincore_count * 100/ mincore_len << "% page residency" << std::endl;
            // free(vec);
        }

        void correctnessCheck(std::vector<value_type> vals) {
            std::vector<value_type> missing;
            if(vals.size() != size) {
                std::cout << "Issue with size discrepency - check array len: " << vals.size() << " current array len: " << size << std::endl;
            }
            for(value_type val : vals) {
                if(!find(val)) {
                    std::cout << "Cannot find " << val << " by find function" << std::endl;
                    missing.push_back(val);
                }
            }
            for(size_t i = 0; i < capacity; i++) {
                for(value_type val : missing) {
                    if(mset[i] == val) {
                        auto hsh = mmh::hash{}(val);
                        int fingerprint = hsh >> 34;
                        if(fingerprint == 0) {
                            fingerprint++;
                        }
                        std::cout << "potential tombstone issue: " << val << " | " << mset[i] << " found at " << i << " with tombstone " << tombstones[i] << " and fingerprint " <<  fingerprint << std::endl;
                    }
                }
            }
            // bool found = false;
            // for(value_type val : vals) {
            //     for(size_t i = 0; i < capacity; i++) {
            //         if(val == mset[i]) {
            //             found = true;
            //             break;
            //         }
            //     }
            //     if(!found) {
            //         std::cout << val << " not found" << std::endl;
            //     }
            //     found = false;
            // }
            count_incore();
        }

        

        int returnLoops() {return insertLoops;}

        uint64_t getSize() {return size;}
        uint64_t getCapacity() {return capacity;}

        int residency_rate() {return size*100/capacity;}

        int get_num_levels() {return num_levels;}

        uint64_t get_global_max() {return global_probe_max;}


        void count_incore() {
            size_t pagesize = sysconf(_SC_PAGE_SIZE);
            // std::cout << capacity * sizeof(value_type) << " | " << (capacity * sizeof(value_type) + pagesize) / pagesize << std::endl << std::flush; 
            unsigned char *result = new unsigned char[(capacity * sizeof(value_type) + pagesize) / pagesize];
            mincore(mset, capacity * sizeof(value_type), result);
            size_t count_incore(0);
            for(size_t i = 0; i<(capacity * sizeof(value_type) + pagesize)/pagesize; ++i) {
                if(result[i] & 1) {
                    count_incore++;
                }
            }
            std::cout << count_incore << " of " << capacity*sizeof(value_type)/ pagesize << std::endl;
            delete result;
        }


    private:
        value_type *mset;
        int *tombstones;
        // 0 is empty, 1 is tombstone, anything else is fingerprint
        // size_t capacity = 1024 * (size_t)sysconf(_SC_PAGESIZE) / sizeof(value_type);
        uint64_t capacity = 1024 * 1024;
        // size_t capacity = 16;
        uint64_t size = 0; //amount of items in set
        // int init_probe = pow((int)log2(capacity),2);
        int num_levels = 1;
        uint64_t init_cap = capacity;

        int level_scale;    //user set amount to scale levels by
        int init_probe;     //user set amount to probe levels by
        // int curr_begin = 0;
        uint64_t global_probe_max = 0;    //work this into insert
        // int curr_probe = 4;
        int mprot = PROT_READ | PROT_WRITE;
        int mflags = MAP_PRIVATE | MAP_ANONYMOUS;
        int mremap_flags = MREMAP_MAYMOVE;
        int mfd = -1;

        int insertLoops = 0;

        void resize(uint64_t max_level_cap){  //change all resizes to size_t
            // printf("attempting resize from %d to %d at current size %d\n", capacity, capacity+(max_level_cap*level_scale), size);
            // std::cout << "resizing from " << capacity << " to " << capacity+(max_level_cap*level_scale) << std::flush << std::endl;
            // std::vector<value_type> temp;
            // for(int i = 0; i < capacity; i++) {
            //     if(tombstones[i] > 0) {
            //         temp.push_back(mset[i]);
            //     }
            // }
            // munmap(mset, capacity*sizeof(value_type));
            // mset = (value_type*)mmap(NULL, capacity*sizeof(value_type)+(max_level_cap*level_scale*sizeof(value_type)), mprot, mflags, -1, 0);
            // count_incore();
            mset = (value_type*)mremap(mset, capacity*sizeof(value_type), 
                                        (capacity + max_level_cap*level_scale)*sizeof(value_type), 
                                        mremap_flags);
            if(mset == (void*)-1) {
                perror("Error mremapping the file");
                return;
            }
            tombstones = (int*)mremap(tombstones, capacity*sizeof(int), capacity*sizeof(int)+(max_level_cap*level_scale*sizeof(int)), mremap_flags);
            capacity += (max_level_cap*level_scale);
            // std::cout << "new capacity = " << capacity << std::endl << std::flush;
            num_levels++;
            // count_incore();
            // for(value_type val : temp) {
            //     insert(val);
            // }
            // printf("finished resize from %d at current size %d\n", capacity, size);
        };

        inline uint64_t rotr64 ( uint64_t x, int8_t r ) {
            return (x >> r) | (x << (64 - r));
        }

        inline uint32_t rotr32 ( uint32_t x, int8_t r ) {
            return (x >> r) | (x << (32 - r));
        }

};