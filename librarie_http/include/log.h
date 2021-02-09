#pragma once

#include <iostream>
#include <string>

class log {
public:
    static void write(const std::string_view message) {
        std::cout << message << std::endl;
    }
};
