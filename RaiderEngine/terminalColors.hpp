#pragma once
#include "stdafx.h"

// add support for colored terminal output by enclosing print statements in ERRORCOLOR, WARNINGCOLOR, or SUCCESSCOLOR
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
inline CONSOLE_SCREEN_BUFFER_INFO cbInfo;
inline HANDLE hConsole;
inline int originalColor;
#define WARNINGCOLOR(msg) { SetConsoleTextAttribute(hConsole, 14); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#define ERRORCOLOR(msg) { SetConsoleTextAttribute(hConsole, 12); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#define SUCCESSCOLOR(msg) { SetConsoleTextAttribute(hConsole, 10); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#else
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define WARNINGCOLOR(msg) { printf(ANSI_COLOR_YELLOW); msg; printf(ANSI_COLOR_RESET); }
#define ERRORCOLOR(msg) { printf(ANSI_COLOR_RED); msg; printf(ANSI_COLOR_RESET); }
#define SUCCESSCOLOR(msg) { printf(ANSI_COLOR_GREEN); msg; printf(ANSI_COLOR_RESET); }
#endif