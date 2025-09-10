#ifdef _WIN32
#include <windows.h>
#endif

#include "Utils/Logger.hpp"

void utl::Logger::init()
{
#ifdef _WIN32
    const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode))
    {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}