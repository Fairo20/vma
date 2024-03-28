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
#include <algorithm>
#include "../murmurhash.hpp"
#define _BSD_SOURCE
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

template <typename Item, typename Alloc = std::allocator<Item>, typename Hash = std::hash<Item>, typename equals = std::equal_to<Item>>
class Index_Set_Control {
    using self_type  = Bag<Item, Alloc>;
    using value_type = Item;

    public:
        Index_Set_Control(){
            mset = (value_type*)malloc(capacity*sizeof(value_type));
            tombstones = (int*)malloc(capacity*sizeof(int));
            for(int i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
        }
        ~Index_Set_Control(){
            free(mset);
            free(tombstones);
        }

        void insert(value_type val) {
            auto hsh = mmh::hash{}(val);
            size_t insert_index;
            bool index_found = false;
            size_t i;
            int probe_length = 0;
            if(size >= capacity * 3 / 4) {
                resize();
            }
            i = hsh % capacity;
            while(1) {
                probe_length++;
                global_probe_max++;
                if(i >= capacity) {
                    i = 0;
                }
                if(tombstones[i] == 0) {
                    if(!index_found) {
                        insert_index = i;
                        index_found = true;
                    }
                    break;
                }
                else if(tombstones[i] == 1) {
                    if(mset[i] == val) {
                        // std::cout << "duplicate found " << val << std::endl;
                        index_found = false;
                        break;
                    }
                }
                else if(tombstones[i] == 2) {   
                    // std::cout << "tombstone i: " << i << " val: " << val << " hash: " << hsh << " residual value? " << mset[i] << std::endl;
                    if(!index_found) {
                        insert_index = i;
                        index_found = true;
                    }
                }
                i++;
            }
            if(index_found) {
                mset[insert_index] = val;
                tombstones[insert_index] = 1;
                size++;
                if(probe_length > max_probe) {
                    max_probe = probe_length;
                }
            }
        }

        bool find(value_type val) {
            auto hsh = mmh::hash{}(val);
            size_t i = hsh%capacity;
            size_t ij = i;
            for(size_t j = 0; j < max_probe; j++) {
                if(ij >= capacity) {
                    ij = 0;
                }
                if(tombstones[ij] == 0) {
                    // if(val == 28736079) {
                    //     std::cout << "incorrect intermediary tombstone at " << ij << std::endl;
                    // }
                    return false;
                }
                else if(tombstones[ij] == 1){
                    if(mset[ij] == val) {
                        return true;
                    }
                } 
                ij++;
            }
            return false;
        }

        //do i have to deconstrunct objects deleted or can i just leave them to be overwritten
        void remove(value_type val) {
            auto hsh = mmh::hash{}(val);
            size_t i = hsh % capacity;
            size_t ij = i;
            for(int j = 0; j < max_probe; j++) {
                if(ij >= capacity) {
                    ij = 0;
                }
                if(tombstones[ij] == 0) {
                    break;
                }
                else if(mset[ij] == val && tombstones[ij] == 1) {
                    tombstones[ij] = 2;
                    ~mset[ij];
                    size--;
                    // printf("proof of erasure: %d | tombstone: %d\n", mset[hsh+i], tombstones[hsh+i]);
                    break;
                }
                ij++;
            }
        }
        
        template <typename Function>
        void removeif(Function fn) {
            int temp = 0;
            for(int i = 0; i < capacity && temp < size; i++) {
                if(tombstones[i] == 1 && fn(mset[i])){
                    tombstones[i] = 2;
                    ~mset[i];
                    size--;
                }
            }
        }

        template <typename Function>
        void for_each(Function fn) {
            int temp = 0;
            for(int i = 0; i < capacity && temp < size; i++) {
                if(tombstones[i] == 1){
                    fn(mset[i]);
                    temp++;
                }
            }
        };

        void clear() {
            for(size_t i = 0; i < capacity; i++) {
                if(tombstones[i] == 1) {
                    ~mset[i];
                }
                tombstones[i] = 0;
            }
            size = 0;
        }

        int returnLoops() {return insertLoops;}

        size_t getSize() {return size;}
        size_t getCapacity() {return capacity;}

        int residency_rate() {return size*100/capacity;}

        size_t get_global_max() {return global_probe_max;}

        void correctnessCheck(std::vector<value_type> vals) {
            std::vector<value_type> missing;
            for(value_type val : vals) {
                if(!find(val)) {
                    std::cout << "Cannot find " << val << " by find function" << std::endl;
                    missing.push_back(val);
                    break;
                }
            }
            for(int i = 0; i < capacity; i++) {
                for(value_type val : missing) {
                    if(mset[i] == val) {
                        std::cout << "potential tombstone issue: " << val << " | " << mset[i] << " found at " << i << " with tombstone " << tombstones[i] << std::endl;
                    }
                }
            }
            // count_incore();
        }

        void count_incore() {
            size_t pagesize = sysconf(_SC_PAGE_SIZE);
            std::cout << capacity * sizeof(value_type) << " | " << (capacity * sizeof(value_type) + pagesize) / pagesize << std::endl << std::flush; 
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
        size_t capacity = 1024 * 1024;
        size_t size = 0;
        int max_probe = 1;
        size_t global_probe_max = 0;

        int insertLoops = 0;

        void resize() {
            // printf("attempting resize from %d to %d at current size %d\n", capacity, capacity*2, size);
            std::vector<value_type> temp_mset;
            for(int i = 0; i < capacity; i++) {
                if(tombstones[i] == 1) {
                    temp_mset.push_back(mset[i]);
                }
            }
            clear();
            capacity *= 2;
            mset = (value_type*)realloc(mset, capacity*sizeof(value_type));
            tombstones = (int*)realloc(tombstones, capacity*sizeof(int));
            for(size_t i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
            for(value_type val : temp_mset) {
                insert(val);
            }
        }
};