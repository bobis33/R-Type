#include <iostream>

#include "Utils/Logger.hpp"
#include "RType/Generated/Version.hpp"

int main(const int argc, const char* const * argv, const char* const * env)
{
    utl::Logger::init();

    utl::Logger::log("args:", utl::LogLevel::INFO);
    for (int i = 0; i < argc; ++i) {
        std::cout << "\t[" << i << "] " << argv[i] << '\n';
    }
    utl::Logger::log("env:", utl::LogLevel::INFO);
    for (const char* const* e = env; *e != nullptr; ++e) {
        std::cout << "\t" << *e << '\n';
    }
    utl::Logger::log("PROJECT INFO:\n", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    return 0;
}