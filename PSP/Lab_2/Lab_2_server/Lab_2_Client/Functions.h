#pragma once
#include <iostream>
#include <string>
#include "Winsock2.h"
using namespace std;

namespace bbb
{
	string GetErrorMsgText(int code);
	string SetErrorMsgText(string prefix, int code);
}