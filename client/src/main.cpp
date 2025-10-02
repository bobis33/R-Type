#include "Client/ArgsHandler.hpp"
#include "Client/Client.hpp"
#include "Utils/Logger.hpp"

int main(const int argc, const char *const *argv, const char *const *env)
{
    utl::Logger::init();

    try
    {
        const cli::ArgsConfig argsConf = cli::ArgsHandler::ParseArgs(argc, argv);
        const cli::EnvConfig envConf = cli::ArgsHandler::ParseEnv(env);
        if (argsConf.exit)
        {
            return EXIT_SUCCESS;
        }
        cli::Client client(argsConf);
        client.run();
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
