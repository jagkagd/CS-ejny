#include "socket.hh"

#include "address.hh"
#include "util.hh"

#include <array>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>
#include <iostream>

int main() {
    std::cout << "socket_dt_test" << std::endl;
    try {
        {
#include "socket_example_1.cc"
        } {
#include "socket_example_2.cc"
        } {
#include "socket_example_3.cc"
        }
    } catch (...) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
