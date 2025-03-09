#pragma once

#include <stdexcept>
#include <string>

struct UseCaseError : std::runtime_error {
   public:
    UseCaseError(std::string code, std::string message)
        : std::runtime_error(message), code(code) {}

    std::string code;
};
