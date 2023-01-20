#pragma once

#include <stdexcept>
#include <string>

class inputNotMatching : public std::runtime_error {
public:
explicit inputNotMatching(const std::string& str) : std::runtime_error(
"input not matching: " + str) {}

};
