#include "bag/bag.hpp"
#include <iostream>

bool isEven(int num) {return num%2==0 ? true : false;}

void increment(int num) {num++;}

void print(int item) {
            printf("%g\n",std::to_string(item));
        }

int main(int argc, char** argv) {
    Bag<int> bag;
    bag.insert(5);
    bag.insert(12);
    int temp = *(bag.fetch_if(isEven));
    std::cout << temp << std::endl;
    bag.remove(12);
    bag.insert(18);
    bag.insert(11);
    bag.insert(2);
    bag.for_each(increment);
    bag.for_each(print); //should be 5 18 11 2
}