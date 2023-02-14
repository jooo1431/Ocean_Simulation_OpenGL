#include <exception>
#include <iostream>

#include "core/Core.h"

int main() {
    try {
        OGL4Core2::Core::Core c;
        c.run();
    } catch (const std::exception& ex) {
        std::cerr << "OGL4Core2 Exception: " << ex.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "OGL4Core2 Exception: Unknown error!" << std::endl;
        return -1;
    }
    return 0;
}
