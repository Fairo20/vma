#include "bag/bag.hpp"
#include <iostream>

bool isEven(int num) {return num%2==0 ? true : false;}

void increment(int &num) {num++;}

void print(int item) {
            // printf("%f\n",std::to_string(item));
            std::cout << item << std::endl;
        }

int main(int argc, char** argv) {
    Bag<int> bag;
    // std::cout << "made bag" << std::endl;
    // bag.insert(5);
    // std::cout << "5" << std::endl;
    // bag.insert(12);
    // std::cout << "12" << std::endl;
    // int *temp = (bag.fetch_if(isEven));
    // // std::cout << "fetch" << std::endl;
    // std::cout << "addr: " << temp << " value: " << *temp << std::endl;
    // bag.remove(12);
    // bag.insert(18);
    // bag.insert(11);
    // bag.insert(2);
    // std::cout << "print old amount then increment" << std::endl;
    // bag.for_each(print); 
    // bag.for_each(increment);
    // std::cout << "print new amount" << std::endl;
    // bag.for_each(print); //should be 6 19 12 3
    for(int i = 0; i < 100000; i++) {
        std::cout << i << std::endl;
        bag.insert(i);
    }
    bag.for_each(print); 
}