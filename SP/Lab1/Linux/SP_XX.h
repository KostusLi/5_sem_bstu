#pragma once

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

extern "C" {
    EXPORT int sum(int x, int y);
    EXPORT int sub(int x, int y);
    EXPORT int mul(int x, int y);
    EXPORT int my_div(int x, int y); 
}