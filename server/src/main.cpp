#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "R-Type/ArgsHandler.hpp"
#include "R-Type/Server.hpp"
#include "Utils/Logger.hpp"

int main(const int argc, const char *const argv[], const char *const env[])
{
    utl::Logger::init();
    try
    {
        const rtp::ArgsConfig argsConf = rtp::ArgsHandler::ParseArgs(argc, argv);
        const rtp::EnvConfig envConf = rtp::ArgsHandler::ParseEnv(env);
        if (argsConf.exit)
        {
            return EXIT_SUCCESS;
        }
        rtp::Server server(argsConf);
    }
    catch (const std::exception &e)
    {
        utl::Logger::log(std::string("Exception: ") + e.what(), utl::LogLevel::WARNING);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        utl::Logger::log("Unknown exception", utl::LogLevel::WARNING);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
