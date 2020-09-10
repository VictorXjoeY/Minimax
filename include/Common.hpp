#pragma once

#ifdef _WIN32

#define COLOR_BLACK ""
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_YELLOW ""
#define COLOR_BLUE ""
#define COLOR_MAGENTA ""
#define COLOR_CYAN ""
#define COLOR_WHITE ""
#define COLOR_BRIGHT_BLACK ""
#define COLOR_BRIGHT_RED ""
#define COLOR_BRIGHT_GREEN ""
#define COLOR_BRIGHT_YELLOW ""
#define COLOR_BRIGHT_BLUE ""
#define COLOR_BRIGHT_MAGENTA ""
#define COLOR_BRIGHT_CYAN ""
#define COLOR_BRIGHT_WHITE ""
#define COLOR_RESET ""

#else

#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BRIGHT_BLACK "\033[90m"
#define COLOR_BRIGHT_RED "\033[91m"
#define COLOR_BRIGHT_GREEN "\033[92m"
#define COLOR_BRIGHT_YELLOW "\033[93m"
#define COLOR_BRIGHT_BLUE "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN "\033[96m"
#define COLOR_BRIGHT_WHITE "\033[97m"
#define COLOR_RESET "\033[0m"

#endif