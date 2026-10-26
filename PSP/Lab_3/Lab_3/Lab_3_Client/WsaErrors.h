#pragma once
#pragma once

#include <string>
#include <winsock2.h>
#pragma comment(lib, "WS2_32.lib")

inline std::string GetErrorMsgText(int code)
{
    switch (code)
    {
    case WSAEINTR:            return "WSAEINTR";
    case WSAEACCES:           return "WSAEACCES";
    case WSAEFAULT:           return "WSAEFAULT";
    case WSAEINVAL:           return "WSAEINVAL";
    case WSAEMFILE:           return "WSAEMFILE";
    case WSAEWOULDBLOCK:      return "WSAEWOULDBLOCK";
    case WSAEINPROGRESS:      return "WSAEINPROGRESS";
    case WSAEALREADY:         return "WSAEALREADY";
    case WSAENOTSOCK:         return "WSAENOTSOCK";
    case WSAEDESTADDRREQ:     return "WSAEDESTADDRREQ";
    case WSAEMSGSIZE:         return "WSAEMSGSIZE";
    case WSAEPROTOTYPE:       return "WSAEPROTOTYPE";
    case WSAENOPROTOOPT:      return "WSAENOPROTOOPT";
    case WSAEPROTONOSUPPORT:  return "WSAEPROTONOSUPPORT";
    case WSAESOCKTNOSUPPORT:  return "WSAESOCKTNOSUPPORT";
    case WSAEOPNOTSUPP:       return "WSAEOPNOTSUPP";
    case WSAEPFNOSUPPORT:     return "WSAEPFNOSUPPORT";
    case WSAEAFNOSUPPORT:     return "WSAEAFNOSUPPORT";
    case WSAEADDRINUSE:       return "WSAEADDRINUSE";
    case WSAEADDRNOTAVAIL:    return "WSAEADDRNOTAVAIL";
    case WSAENETDOWN:         return "WSAENETDOWN";
    case WSAENETUNREACH:      return "WSAENETUNREACH";
    case WSAENETRESET:        return "WSAENETRESET";
    case WSAECONNABORTED:     return "WSAECONNABORTED";
    case WSAECONNRESET:       return "WSAECONNRESET";
    case WSAENOBUFS:          return "WSAENOBUFS";
    case WSAEISCONN:          return "WSAEISCONN";
    case WSAENOTCONN:         return "WSAENOTCONN";
    case WSAESHUTDOWN:        return "WSAESHUTDOWN";
    case WSAETIMEDOUT:        return "WSAETIMEDOUT";
    case WSAECONNREFUSED:     return "WSAECONNREFUSED";
    case WSAEHOSTDOWN:        return "WSAEHOSTDOWN";
    case WSAEHOSTUNREACH:     return "WSAEHOSTUNREACH";
    case WSAEPROCLIM:         return "WSAEPROCLIM";
    case WSASYSNOTREADY:      return "WSASYSNOTREADY";
    case WSAVERNOTSUPPORTED:  return "WSAVERNOTSUPPORTED";
    case WSANOTINITIALISED:   return "WSANOTINITIALISED";
    case WSAEDISCON:          return "WSAEDISCON";
    case WSATYPE_NOT_FOUND:   return "WSATYPE_NOT_FOUND";
    case WSAHOST_NOT_FOUND:   return "WSAHOST_NOT_FOUND";
    case WSATRY_AGAIN:        return "WSATRY_AGAIN";
    case WSANO_RECOVERY:      return "WSANO_RECOVERY";
    case WSANO_DATA:          return "WSANO_DATA";
    case WSA_INVALID_HANDLE:  return "WSA_INVALID_HANDLE";
    case WSA_INVALID_PARAMETER: return "WSA_INVALID_PARAMETER";
    case WSA_IO_INCOMPLETE:   return "WSA_IO_INCOMPLETE";
    case WSA_IO_PENDING:      return "WSA_IO_PENDING";
    case WSA_NOT_ENOUGH_MEMORY: return "WSA_NOT_ENOUGH_MEMORY";
    case WSA_OPERATION_ABORTED: return "WSA_OPERATION_ABORTED";
    case WSASYSCALLFAILURE:   return "WSASYSCALLFAILURE";
    default:                  return "***ERROR***";
    }
}


inline std::string SetErrorMsgText(std::string msgText, int code)
{
    return msgText + GetErrorMsgText(code) + " (" + std::to_string(code) + ")";
}