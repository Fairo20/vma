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
class Vec_Bag {
    using self_type  = Bag<Item, Alloc>;
    using value_type = Item;

    public:
        Vec_Bag(){};
        ~Vec_Bag(){};

        void insert(value_type item) {
            mbag.push_back(item);
            size++;
        };

        void remove(value_type item) {
            for(value_type mitem : mbag) {
                if(item == mitem){
                    value_type temp = mbag.at(size-1);
                    mbag.at(size-1) = mitem;
                    mitem = temp;
                    mbag.pop_back(); 
                    size--;
                }
            }
        };

        template <typename Function>
        void removeif(Function fn) {
            for(int i = 0; i < size; i++) {
                if(fn(mbag.at(i))) {
                    value_type temp = mbag.at(size-1);
                    mbag.at(size-1) = mbag.at(i);
                    mbag.at(i) = temp;
                    mbag.pop_back();
                    size--;
                }
            }
        }

        template <typename Function>
        void for_each(Function fn) {
            for(size_t i = 0; i < size; i++) {
                fn(mbag.at(i));
            }
        };

        void clear() {
            mbag.clear();
        }

        void swap(self_type &s) {
            mbag.swap(s);
        }


    private:
        std::vector<value_type> mbag;
        size_t size = 0;
};