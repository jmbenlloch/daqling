#include <iostream>

#include "utilities/logging.hpp"

//using namespace spdlog;

int main() {
    
    std::cout << "Hello c++17!" << std::endl;

    INFO("YOLO " << 12345);

    // create color multi threaded logger
    //auto console = spdlog::stdout_color_mt("console");
    //console->info("Welcome to spdlog!");
    //console->error("Some error message with arg: {}", 1);

    //auto err_logger = spdlog::stderr_color_mt("stderr");
    //err_logger->error("Some error message");

    // Formatting examples
    //console->warn("Easy padding in numbers like {:08d}", 12);
    //console->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    //console->info("Support for floats {:03.2f}", 1.23456);
}

