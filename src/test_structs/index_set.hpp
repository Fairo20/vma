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

        //is this a true set that does not allow duplicates? if so need to search the entire probe distance before inserting
        void insert2(value_type val) {
            auto hsh = murmur_hash_64a(&val, sizeof(val), 0xa9377665cb32);
            // printf("entered put with value %d and hash %d\n", val, hsh);
            hashInsert(mset, tombstones, val, hsh, 0);
        };

        void insert(value_type val) {
            auto hsh = murmur_hash_64a(&val, sizeof(val), 0xa9377665cb32);
            int insert_index = -1;
            int i;
            if(size >= capacity * 3 / 4) {
                // std::cout << "beginning resize" << std::endl;
                resize();
            }
            i = hsh % capacity;
            // std::cout << "inserting " << val << " with size " << size << " and cap " << capacity << std::endl << std::flush;
            while(1) {
                if(i >= capacity) {
                    i = 0;
                    // std::cout << "index hit cap, cycling to 0" << std::endl;
                }
                if(tombstones[i] == 0) {
                    // std::cout << "empty" << std::endl;
                    if(insert_index < 0) {
                        insert_index = i;
                    }
                    break;
                }
                else if(tombstones[i] == 1) {
                    // std::cout << "full" << std::endl;
                    if(mset[i] == val) {
                        std::cout << "duplicate found " << val << std::endl;
                        insert_index = -1;
                        break;
                    }
                }
                else if(tombstones[i] == 2) {
                    std::cout << "tombstone i: " << i << " val: " << val << " hash: " << hsh << " residual value? " << mset[i] << std::endl;
                    if(insert_index < 0) {
                        insert_index = i;
                    }
                }
                i++;
            }
            if(insert_index >= 0) {
                // std::cout << "inserting val" << std::endl;
                mset[insert_index] = val;
                tombstones[insert_index] = 1;
                size++;
            }
        }

        bool find(value_type val) {
            auto hsh = std::hash<value_type>{}(val)%capacity;
            for(int i = 0; i < max_probe; i++) {
                if(tombstones[hsh] == 0) {
                    return false;
                }
                else {
                    if(mset[hsh] == val && tombstones[hsh] == 1) {
                        return true;
                    }
                }
            }
            return false;
        };

        //do i have to deconstrunct objects deleted or can i just leave them to be overwritten
        void remove(value_type val) {
            // printf("begin remove for val %d\n", val);
            auto hsh = std::hash<value_type>{}(val)%capacity;
            for(int i = 0; i < max_probe; i++) {
                if(tombstones[hsh + i] == 0) {
                    // printf("val not found, returning\n");
                    break;
                }
                else if(mset[hsh + i] == val && tombstones[hsh + i] == 1) {
                    // printf("val %d found at %d, removed\n", val, hsh+i);
                    tombstones[hsh + i] = 2;
                    ~mset[hsh + i];
                    size--;
                    // printf("proof of erasure: %d | tombstone: %d\n", mset[hsh+i], tombstones[hsh+i]);
                    break;
                }
            }
        };
        
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

    private:
        value_type *mset;
        int *tombstones;
        size_t capacity = (size_t)sysconf(_SC_PAGESIZE) / sizeof(value_type);
        // size_t capacity = 64;
        size_t size = 0;
        int max_probe = pow((int)log2(capacity),2);
        // int max_probe = 8;

        int insertLoops = 0;

        int hashInsert(value_type *set, int *tombstone, value_type val, auto hsh, int curr_probe) { 
            int insertIndex = -1;   
            if(hsh >= capacity) {
                hsh = 0;
            }        
            if(tombstones[hsh] == 0) {
                // printf("found empty slot at %d for %d\n",hsh,val);
                insertIndex = hsh;
                set[hsh] = val;
                tombstones[hsh] = 1;
                size++;
                if(size >= capacity * 3 / 4) {
                    resize();
                }
                // printf("inserted %d at %d\n", set[hsh], hsh);
            }
            else if(tombstones[hsh] == 2) {
                int i = 1;
                // while(i + curr_probe < max_probe) {
                //     if(tombstones[hsh + i] == 0) {
                //         // insertLoops++;
                //         break;
                //     }
                //     if(set[hsh + i] == val) {
                //         insertLoops++;
                //         return -1;
                //     }
                //     i++;
                // }
                // if(size >= capacity * 3 / 4) {
                //     resize();
                // }
                // if()
                insertIndex = hsh;
                set[hsh] = val;
                tombstones[hsh] = 1;
            }
            else if(tombstones[hsh] == 1) {
                if(set[hsh] == val) {
                    // printf("attempt to insert duplicate %d, returning\n", val);
                    insertLoops++;
                    return -1;
                }
                // if(curr_probe == max_probe) {
                //     resize();
                //     hashInsert(set, tombstone, val, hsh+1, curr_probe+1);
                // }
                else {
                    hashInsert(set, tombstone, val, hsh+1, curr_probe+1);
                }
            }
            insertLoops++;
            return 0;
        };

        void resize() {
            printf("attempting resize from %d to %d at current size %d\n", capacity, capacity*2, size);
            std::vector<value_type> temp_mset;
            // int temp = 0;
            for(int i = 0; i < capacity; i++) {
                if(tombstones[i] == 1) {
                    // if(std::find(temp_mset.begin(), temp_mset.end(), mset[i]) != temp_mset.end()) {
                    //     std::cout << "FAILURE: " << mset[i] << std::endl;
                    // }
                    temp_mset.push_back(mset[i]);
                    // temp++;
                    // std::cout << "index " << temp_index << std::endl;
                    // std::cout << "inserting " << mset[i] << " into temp array" << std::endl;
                }
            }
            // if(temp != size) {
            //     std::cout << "temp: " << temp << " | size: " << size << " | actual: " << capacity * 3 / 4 << std::endl;
            // }
            clear();
            // free(mset);
            // free(tombstones);
            capacity *= 2;
            mset = (value_type*)realloc(mset, capacity*sizeof(value_type));
            tombstones = (int*)realloc(tombstones, capacity*sizeof(int));
            for(size_t i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
            for(value_type val : temp_mset) {
                insert(val);
                // std::cout << "inserting " << temp_mset[i] << " into resized array with size " << size << std::endl;
            }
        }

        void resize2() {
            printf("attempting resize from %d to %d at current size %d\n", capacity, capacity*2, size);
            value_type *temp_mset = (value_type*)malloc(capacity*2*sizeof(value_type));
            int *temp_tombs = (int*)malloc(capacity*2*sizeof(int));
            for(int i = 0; i < capacity; i++) {
                temp_tombs[i] = 0;
            }
            size_t temp_size = size;
            size_t temp = 0;
            size = 0;
            max_probe *= 2;
            capacity *= 2;
            auto hsh = murmur_hash_64a(mset[0], sizeof(mset[0]), 0xa9377665cb32);
            for(size_t i = 0; i < capacity && temp < temp_size; i++) {
                if(tombstones[i] == 1) {
                    // printf("resize insert %d at %d\n",mset[i],hsh);
                    hsh = murmur_hash_64a(mset[i], sizeof(mset[i]), 0xa9377665cb32);
                    hashInsert(temp_mset, temp_tombs, mset[i], hsh, 0);
                    temp++;
                }
            }
            free(mset);
            free(tombstones);
            mset = temp_mset;
            tombstones = temp_tombs;
        };
};