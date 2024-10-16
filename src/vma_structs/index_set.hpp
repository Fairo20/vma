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
            mset = (value_type*)mmap(NULL, capacity, mprot, mflags, -1, 0);
            tombstones = (int*)mmap(NULL, capacity, mprot, mflags, -1, 0);
            for(int i = 0; i < capacity; i++) {
                tombstones[i] = 0;
            }
        };
        ~Index_Set(){
            munmap(mset, capacity);
            munmap(tombstones, capacity);
        };

        //is this a true set that does not allow duplicates? if so need to search the entire probe distance before inserting
        void insert(value_type val) {
            auto hsh = Hash{}(val)%capacity;
            // printf("entered put with value %d and hash %d\n", val, hsh);
            hashInsert(mset, tombstones, val, hsh, 0);
        };

        bool find(value_type val) {
            auto hsh = Hash{}(val)%capacity;
            int i = 1;
            int j = 0;
            int probe_level = init_probe;
            while(i < num_levels) {
                while(j < max_probe) {
                    if(tombstones[hsh] == 0) {
                        return false;
                    }
                    else {
                        if(mset[hsh] == val && tombstones[hsh] == 1) {
                            return true;
                        }
                    }
                    if(j >= probe_level) {
                        i++;
                        j = 0;
                        probe_level *= 2;
                    }
                }
            }
            return false;
        };

        //do i have to deconstrunct objects deleted or can i just leave them to be overwritten
        void remove(value_type val) {
            // printf("begin remove for val %d\n", val);
            auto hsh = Hash{}(val)%capacity;
            for(int i = 0; i < max_probe; i++) {        //maximum probe loc
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
            for(size_t i = 0; i < size; i++) {
                ~mset[i];
                tombstones[i] = 0;
            }
            size = 0;
        }

    private:
        value_type *mset;
        int *tombstones;
        size_t capacity = (size_t)sysconf(_SC_PAGESIZE);
        size_t size = 0;
        int num_levels = 1;
        int level_scale = 2;
        int init_probe = 8;
        int max_probe = 8;
        int mprot = PROT_READ | PROT_WRITE;
        int mflags = MAP_PRIVATE | MAP_ANONYMOUS;
        int mremap_flags = MREMAP_MAYMOVE;
        int mfd = -1;

        int hashInsert(value_type *set, int *tombstone, value_type val, auto hsh, int curr_probe) {            
            if(tombstone[hsh] == 0) {
                // printf("found empty slot at %d for %d\n",hsh,val);
                set[hsh] = val;
                tombstone[hsh] = 1;
                size++;
                if(size >= capacity * 3 / 4) {      //remove this
                    resize();
                }
                // printf("inserted %d at %d\n", set[hsh], hsh);
            }
            else if(tombstone[hsh] == 2) {
                int i = 1;
                while(i + curr_probe < max_probe) {     //maximum probe loc
                    if(tombstone[hsh + i] == 0) {
                        break;
                    }
                    if(set[hsh + i] == val) {
                        return -1;
                    }
                    i++;
                }
                if(size >= capacity * 3 / 4) {
                    resize();
                }
                set[hsh] = val;
                tombstone[hsh] = 1;
            }
            else if(tombstone[hsh] == 1) {
                if(set[hsh] == val) {
                    // printf("attempt to insert duplicate %d, returning\n", val);
                    return -1;
                }
                if(curr_probe == max_probe) {           //maximum probe loc
                    resize();
                    hashInsert(set, tombstone, val, hsh+1, curr_probe+1);       //am i recomputing hash here?
                }
                else {
                    hashInsert(set, tombstone, val, hsh+1, curr_probe+1);
                }
            }
            return 0;
        };

        void resize() {
            //resize mapping
            mset = (value_type*)mremap(mset, capacity, capacity*(level_scale+1), mremap_flags);
            tombstones = (int*)mremap(tombstones, capacity, capacity*(level_scale+1), mremap_flags);
            //check against boundary calculation function

            //check max probe distance against calc

            num_levels++;
            max_probe *= level_scale;
        };

        int calcProbe(int index) {
            int cap = capacity;
            int bound;
            int lvls = num_levels;
            while(cap >= (size_t)sysconf(_SC_PAGESIZE)) {
                bound = cap / (level_scale+1);
                if(index > bound && index < cap) {
                    return lvls*8;      //init prob distance times level finds level probe
                }
                cap = bound;
            }
            return 8;
        }
};