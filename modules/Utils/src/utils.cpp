#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Utils/Utils.hpp"

std::vector<char> utl::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file " + filename);
    }
    const long int fileSize = file.tellg();
    if (fileSize <= 0)
    {
        throw std::runtime_error("file " + filename + " is empty");
    }
    std::vector<char> buffer(static_cast<long unsigned int>(fileSize));
    file.seekg(0, std::ios::beg);
    if (!file.read(buffer.data(), fileSize))
    {
        throw std::runtime_error("failed to read file " + filename);
    }
    return buffer;
}

std::unordered_map<std::string, std::string> utl::getEnvMap(const char *const *env)
{
    std::unordered_map<std::string, std::string> cpyEnv;

#ifdef _WIN32
    LPCH envStrings = GetEnvironmentStringsA();
    if (!envStrings)
    {
        return cpyEnv;
    }

    for (LPCH var = envStrings; *var; var += std::strlen(var) + 1)
    {
        std::string entry(var);
        if (const auto pos = entry.find('='); pos != std::string::npos)
        {
            cpyEnv.emplace(entry.substr(0, pos), entry.substr(pos + 1));
        }
    }

    FreeEnvironmentStringsA(envStrings);
#else
    for (const char *const *current = env; (current != nullptr) && (*current != nullptr); ++current)
    {
        std::string entry(*current);
        if (const auto pos = entry.find('='); pos != std::string::npos)
        {
            cpyEnv.emplace(entry.substr(0, pos), entry.substr(pos + 1));
        }
    }
#endif

    return cpyEnv;
}
