#include <string>
#include <format>
#include <iostream>
#include <fstream>


std::ofstream log_file;


void open_log_file() {
    log_file.open("log.txt");
}

void close_log_file() {
    log_file.close();
}

/**
 * @brief Log informative messages to the desired output.
 *
 * @param msg The message to log.
 */
void log_info(std::string msg) {
    std::string complete_msg = std::format("LOG: {}", msg);
    std::cout << complete_msg << std::endl;

    log_file << complete_msg << std::endl;
}

/**
 * @brief Log error messages.
 *
 * @param msg The message to log.
 */
void log_err(std::string msg) {
    std::cout << "ERROR: " << msg << std::endl;
}
