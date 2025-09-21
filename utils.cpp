// utils.cpp
#include "utils.h"
#include <fstream>
#include <sstream>

// IMPLEMENTATION HERE
std::string readFileContent(const std::filesystem::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
