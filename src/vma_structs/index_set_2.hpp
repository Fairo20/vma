#pragma once
#include <functional>
#include <memory>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <iostream>
#define _BSD_SOURCE
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

template <typename Item, typename Alloc = std::allocator<Item>, typename Hash = std::hash<Item>, typename equals = std::equal_to<Item>>
class Index_Set {
    using self_type  = Bag<Item, Alloc>;
    using value_type = Item;
    using hash = Hash;

    public:
        Index_Set(){
            mset = (value_type*)mmap(NULL, capacity*sizeof(value_type), mprot, mflags, -1, 0);
            if (mset == MAP_FAILED) {
                handle_error("mmap");   
            }
            printf("made mset\n");
            fflush(stdout);
            tombstones = (int*)mmap(NULL, capacity*sizeof(int), mprot, mflags, -1, 0);
            printf("made tombstones\n");
            fflush(stdout);
            for(int i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
        };
        ~Index_Set(){
            munmap(mset, capacity);
            munmap(tombstones, capacity);
        };

        void insert(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            int curr_level = 0;
            auto hsh = Hash{}(val);
            int i = hsh%curr_cap;
            int offset = 0;
            int insert_index = -1;
            int max_probe = 0;
            //level indexing: as levels increase
            // printf("begin insert for val %d\n", val);
            for(curr_level; curr_level < num_levels; curr_level++) {
                for(int j = 0; j + i < curr_cap + offset && j < curr_probe; j++) {
                    // printf("i: %d, j: %d, curr_probe: %d\n", i, j, curr_probe);
                    if(tombstones[i+j] == 0) {    //if empty
                        if(insert_index < 0) {
                            insert_index = i+j;
                            // printf("insert empty %d at %d\n", val, i+j);
                        }
                        else {
                            // printf("probed at %d and found end, exiting\n", i+j);
                        }
                        i = capacity;
                        break;
                    }
                    else if(tombstones[i+j] == 1) {   //if tombstone
                        if(insert_index < 0) {
                            insert_index = i+j;
                            max_probe++;
                            // printf("found tombstone at %d, begin probing\n", i+j);
                        }
                        else {
                            max_probe++;
                            // printf("probed at %d and not found, continuing\n", i+j);
                        }
                    }
                    else {                      //if filled
                        if(mset[i+j] == val) {        //if value already exists in set
                            i = capacity;
                            break;
                        }
                        else if (insert_index < 0){
                            max_probe++;
                            // printf("searching at %d and filled, continuing\n", i+j);
                        }
                        else {
                            max_probe++;
                            // printf("probed at %d and not found, continuing\n", i+j);
                        }
                    }
                    if(j + 1 >= curr_probe && curr_level + 1 >= num_levels) {       //resize if on last level and hit max probe
                        // printf("hit max probe and last level\n");
                        if(insert_index < 0) {
                            resize(curr_cap);
                        }
                        continue;
                    }
                    // printf("vals at end of level loop: i=%d, curr_cap=%d, curr_probe=%d, curr_level=%d\n", i, curr_cap, curr_probe, curr_level);
                }
                offset += curr_cap;
                curr_cap*=level_scale;
                i = hsh%curr_cap + offset;
                // curr_probe*=level_scale;
                // printf("exited j loop, new vals: offset = %d, curr_cap = %d, i = %d, curr_probe = %d\n", offset, curr_cap, i, curr_probe);
            }   
            // printf("exited i loop\n");
            if(insert_index >= 0) {
                mset[insert_index] = val;
                tombstones[insert_index] = 2; //replace with fingerprint
                printf("max probe for val %d: %d\n", val, max_probe);
                size++;
            }
        }

        void insert2(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            auto hsh = Hash{}(val);
            int start = 0;
            int cpd = 0;
            for(int i = hsh%curr_cap; i - start < curr_cap; i++) {
                // printf("i: %d val: %d: num_loops: %d\n", i, val, insertLoops);   
                if(tombstones[i] == 0) {
                    mset[i] = val;
                    tombstones[i] = 2; //replace with fingerprint
                    size++;
                    insertLoops++;
                    break;
                }
                else if(tombstones[i] == 1) {
                    for(int j = i; j < curr_cap; j++) {
                        if(tombstones[j] == 0) {
                            mset[i] = val;
                            tombstones[i] = 2; //replace with fingerprint
                            size++;
                            break;
                        }
                        if(mset[j] == val) {
                            break;
                        }
                        if(j == curr_probe) {
                            if(start + curr_cap >= capacity) {
                                mset[i] = val;
                                tombstones[i] = 2; //replace with fingerprint
                                size++;
                                break;
                            }
                            start += curr_cap;
                            curr_probe *= 2;
                            curr_cap *=2;
                            i = hsh%curr_cap + start;
                        }
                    }
                }
                else {          //if filled
                    if(mset[i] == val) {
                        insertLoops++;
                        break;
                    }
                    if(cpd >= curr_probe) {
                        if(start + curr_cap >= capacity) {
                            resize(curr_cap);
                            printf("old probe distance: %d new probe distance: %d\n", curr_probe, curr_probe*2);
                            // printf("old - start: %d cp: %d cc: %d i: %d\nnew - start: %d cp: %d cc: %d i: %d\nnum_loops: %d val: %d\n", start, curr_probe, curr_cap, i, start + curr_cap, curr_probe*2, curr_cap*2, hsh%(curr_cap*2), insertLoops, val);
                        }
                        start += curr_cap;
                        curr_probe *= 2; 
                        // size++;
                        curr_cap *= 2;
                        i = hsh%curr_cap + start;
                    }
                }
                cpd++;
                insertLoops++;
            }
        };

        bool find(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            auto hsh = Hash{}(val);
            int start = 0;
            for(int i = hsh%curr_cap; i < curr_cap; i++) {
                if(tombstones[i] == 0) {
                    return false;
                }
                else if(tombstones[i] == 1) {
                    for(int j = i; j < curr_cap; j++) {
                        if(mset[j] == val) {
                            return true;
                        }
                        if(j == curr_probe) {
                            if(start + curr_cap >= capacity) {
                                return false;
                            }
                            start += curr_cap;
                            curr_probe *= 2;
                            curr_cap *=2;
                            i = hsh%curr_cap + start;
                        }
                    }
                }
                else {
                    if(mset[i] == val) {
                        return true;
                    }
                    if(i == curr_probe) {
                        if(start + curr_cap >= capacity) {
                            return false;
                        }
                        start += curr_cap;
                        curr_probe *= 2;
                        curr_cap *= 2;
                        i = hsh%curr_cap + start;
                    }
                }
            }
        };
        void remove(value_type val) {
            int curr_probe = init_probe;
            int curr_cap = init_cap;
            auto hsh = Hash{}(val);
            int start = 0;
            for(int i = hsh%curr_cap; i < curr_cap; i++) {
                if(tombstones[i] == 0) {
                    break;
                }
                else if(tombstones[i] == 1) {
                    for(int j = i; j < curr_cap; j++) {
                        if(mset[j] == val) {
                            ~mset[j];
                            tombstones[j] = 1;
                            size--;
                        }
                        if(j == curr_probe) {
                            if(start + curr_cap >= capacity) {
                                break;
                            }
                            start += curr_cap;
                            curr_probe *= 2;
                            curr_cap *=2;
                            i = hsh%curr_cap + start;
                        }
                    }
                }
                else {
                    if(mset[i] == val) {
                        ~mset[i];
                        tombstones[i] = 1;
                        size--;
                    }
                    if(i == curr_probe) {
                        if(start + curr_cap >= capacity) {
                            break;
                        }
                        start += curr_cap;
                        curr_probe *= 2;
                        curr_cap *= 2;
                        i = hsh%curr_cap + start;
                    }
                }
            }
        };
        void clear() {
            for(size_t i = 0; i < capacity; i++) {
                ~mset[i];
                tombstones[i] = 0;
            }
            madvise(mset+(size_t)sysconf(_SC_PAGESIZE), capacity*sizeof(value_type) - (size_t)sysconf(_SC_PAGESIZE), MADV_DONTNEED);
            madvise(tombstones+(size_t)sysconf(_SC_PAGESIZE), capacity*sizeof(int) - (size_t)sysconf(_SC_PAGESIZE), MADV_DONTNEED);
            capacity = (size_t)sysconf(_SC_PAGESIZE);
            size = 0;
        };

        template <typename Function>
        void removeif(Function fn) {
            int temp = 0;
            int bound = size;
            for(int i = 0; i < capacity && temp < bound; i++) {
                if(tombstones[i] > 1 && fn(mset[i])){
                    tombstones[i] = 2;
                    ~mset[i];
                    size--;
                }
                if(tombstones[i] > 1) {
                    temp++;
                }
            }
        };

        template <typename Function>
        void for_each(Function fn) {
            int temp = 0;
            int bound = size;
            for(int i = 0; i < capacity && temp < bound; i++) {
                if(tombstones[i] > 1){
                    fn(mset[i]);
                    temp++;
                }
            }
        };

        int returnLoops() {return insertLoops;}

        size_t getSize() {return size;}
        size_t getCapacity() {return capacity;}

        int residency_rate() {return size*100/capacity;}

    private:
        value_type *mset;
        int *tombstones;
        // 0 is empty, 1 is tombstone, anything else is fingerprint
        // size_t capacity = (size_t)sysconf(_SC_PAGESIZE) / sizeof(value_type);
        size_t capacity = 16;
        size_t size = 0; //amount of items in set
        // int init_probe = pow((int)log2(capacity),2);
        int num_levels = 1;
        int init_probe = 4;
        int init_cap = capacity;
        int level_scale = 2; //user set amount to scale levels by
        // int curr_begin = 0;
        // int curr_probe = 4;
        int mprot = PROT_READ | PROT_WRITE;
        int mflags = MAP_PRIVATE | MAP_ANONYMOUS;
        int mremap_flags = MREMAP_MAYMOVE;
        int mfd = -1;

        int insertLoops = 0;

        void resize(size_t max_level_cap){  //change all resizes to size_t
            printf("attempting resize from %d to %d at current size %d\n", capacity, capacity+(max_level_cap*level_scale), size);
            mset = (value_type*)mremap(mset, capacity*sizeof(value_type), capacity*sizeof(value_type)+(max_level_cap*level_scale*sizeof(value_type)), mremap_flags);
            tombstones = (int*)mremap(tombstones, capacity*sizeof(int), capacity*sizeof(int)+(max_level_cap*level_scale*sizeof(int)), mremap_flags);
            // for(int i = capacity; i < capacity + (max_level_cap*level_scale); i++) {
            //     tombstones[i] = 0;
            // }
            capacity += (max_level_cap*level_scale);
            num_levels++;
        };
};