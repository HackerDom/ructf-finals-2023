#include <iostream>

#include "storage/storage.h"

int main() {
    std::cout << sizeof(std::shared_ptr<int>) << std::endl;
    return 0;
}
