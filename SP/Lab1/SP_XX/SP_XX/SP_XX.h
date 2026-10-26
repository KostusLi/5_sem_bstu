#pragma once

extern "C" {
    __declspec(dllexport) int sum(int x, int y);
    __declspec(dllexport) int sub(int x, int y);
    __declspec(dllexport) int mul(int x, int y);
    __declspec(dllexport) int my_div(int x, int y);
}