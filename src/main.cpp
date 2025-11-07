#include <iostream>
#include "config.hpp"

int main()
{
    int* ptr = new int(42);
    delete ptr;
}