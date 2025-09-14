#include <iostream>

#include "Server/ArgsHandler.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"

int main(const int argc, const char *const argv[], const char *const env[])
{
    utl::Logger::init();
    try
    {
        const srv::ArgsConfig argsConf = srv::ArgsHandler::ParseArgs(argc, argv);
        const srv::EnvConfig envConf = srv::ArgsHandler::ParseEnv(env);
        if (argsConf.exit)
        {
            return EXIT_SUCCESS;
        }
        srv::Server server(argsConf);
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
