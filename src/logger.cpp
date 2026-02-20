#include <string>
#include <iostream>


void log_info(std::string msg) {
    std::cout << "LOG: " << msg << std::endl;
}

void log_err(std::string msg) {
    std::cout << "ERROR: " << msg << std::endl;
}
