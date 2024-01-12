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

template <typename Item, typename Alloc = std::allocator<Item>>
class Bag {
    using self_type  = Bag<Item, Alloc>;
    using value_type = Item;
    
    //bag is effectively a vector that does not guarantee ordering, so indexes won't be used for any user-facing features

    public:
        Bag() {
            map = (value_type*)mmap(NULL, mlen, mprot, mflags, -1, 0);
            if (map == MAP_FAILED) {
                handle_error("mmap");
            }
            //std::cout << "page size" << mlen << std::endl;
        }

        ~Bag() {
            int check = munmap(map, mlen);
            if(check==-1) {
                handle_error("munmap");
            }
        }

        //insert an item into the bag
        void insert(const value_type &item) {
            if((msize * sizeof(value_type)) + sizeof(value_type) >= mlen) {
                //std::cout << "resizing" << std::endl;
                resize();
            }
            //don't think i have to memcpy every time but gonna leave this here just in case
            /*memcpy(item,addr+offset,sizeof(value_type));*/
            map[msize] = item;
            msize++;    
            // moffset += sizeof(value_type);
        };

        //remove a specific item from the bag
        int remove(const value_type &item) {
            for(size_t i = 0; i < msize; i++) {
                if(map[i] == (item)) {
                    value_type temp = map[msize-1];
                    map[i] = map[msize-1];
                    map[msize-1] = '\0';
                    msize--;
                    i--;
                    return 1;
                }
            }
            return 0;
        };
        
        //remove all items that fit a value function
        template <typename Function>
        int removeif(Function fn) {
            int numDeleted = 0;
            for(size_t i = 0; i < msize; i++) {
                if(fn(map[i])) {
                    value_type temp = map[msize-1];
                    map[i] = map[msize-1];
                    ~map[msize-1];
                    msize--;
                    i--;
                    numDeleted++;
                }
            }
            return numDeleted;
        };

        //retrieve all items that fit a value function
        template <typename Function>
        value_type* fetch_if(Function fn) {
            for(size_t i = 0; i < msize; i++) {
                if(fn(map[i])) {
                    return &(map[i]);
                }
            }
            return NULL;
        };
        
        //applies a function to all items
        template <typename Function>
        void for_each(Function fn) {
            for(size_t i = 0; i < msize; i++) {
                fn(map[i]);
            }
        };

        //empty bag
        void clear() {
            for(size_t i = 0; i < msize; i++) {
                ~map[i];
            }
            madvise(map+(size_t)sysconf(_SC_PAGESIZE), mlen, MADV_DONTNEED);
            mlen = (size_t)sysconf(_SC_PAGESIZE);
            msize = 0;
        };

        //replace bag items with another bag's items (low priority rn will develop later)
        void swap(self_type &s) {
            std::swap(map, s.map);
            std::swap(mlen, s.mlen);
            std::swap(msize, s.msize);
        };

        //resize bag to allow for more items (i think for now just gonna double it every time)
        void resize() {
            map = (value_type*)mremap(map, mlen, mlen*2, mremap_flags);
            mlen *= 2;
            //std::cout << "new mlen: " << mlen << std::endl;
            if (map == MAP_FAILED) {
                handle_error("mremap");
            }
        };

        void shrinktofit() {
            if((msize * sizeof(value_type)) + (size_t)sysconf(_SC_PAGESIZE) < mlen) {
                //pages used: ceil((msize * sizeof(value_type))/(size_t)sysconf(_SC_PAGESIZE))
                //pages allocated: mlen/(size_t)sysconf(_SC_PAGESIZE)
                madvise(map+(ceil((msize * sizeof(value_type))/(size_t)sysconf(_SC_PAGESIZE))*(size_t)sysconf(_SC_PAGESIZE)), mlen);
                mlen = ceil((msize * sizeof(value_type))/(size_t)sysconf(_SC_PAGESIZE))*(size_t)sysconf(_SC_PAGESIZE);
            }
        }

        //return amount of items in bag
        size_t size() {return msize;};

        //return if bag is empty or not
        bool isempty(){return (msize == 0 ? 1 : 0);};
    
    protected:
        value_type *map;
        size_t mlen = (size_t)sysconf(_SC_PAGESIZE);
        int mprot = PROT_READ | PROT_WRITE;
        int mflags = MAP_PRIVATE | MAP_ANONYMOUS;
        int mremap_flags = MREMAP_MAYMOVE;
        int mfd = -1;
        size_t msize = 0;
};