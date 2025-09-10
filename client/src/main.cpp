#include <iostream>

#include "R-Type/ArgsHandler.hpp"
#include "R-Type/Client.hpp"
#include "R-Type/Generated/Version.hpp"
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
        rtp::Client client(argsConf);
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
