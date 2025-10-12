///
/// @file Utils.hpp
/// @brief This file contains utility functions
/// @namespace utl
///

#pragma once

#include <unordered_map>
#include <vector>

namespace utl
{

    [[nodiscard]] std::vector<char> readFile(const std::string &filename);
    [[nodiscard]] std::unordered_map<std::string, std::string> getEnvMap(const char *const *env);

} // namespace utl
