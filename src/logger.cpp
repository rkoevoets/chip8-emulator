#include <string>
#include <iostream>


/**
 * @brief Log informative messages to the desired output.
 *
 * @param msg The message to log.
 */
void log_info(std::string msg) {
    std::cout << "LOG: " << msg << std::endl;
}

/**
 * @brief Log error messages.
 *
 * @param msg The message to log.
 */
void log_err(std::string msg) {
    std::cout << "ERROR: " << msg << std::endl;
}
