#pragma once

/* Windows */
#ifdef _WINDOWS
#ifdef EZTCP_EXPORTS
#define EZTCP_API __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

#else
#define EZTCP_API __declspec(dllimport)
#endif // EZTCP_EXPORTS

#define INT16 __int16
#define INT32 __int32
#define INT64 __int64
#define SIZET size_t
#endif // _WINDOWS

/* Linux */
#ifdef __linux
#include <cstddef>
#ifdef EZTCP_EXPORTS
#include <netinet/in.h>
#endif
#define EZTCP_API
#define INT16 short
#define INT32 int
#define INT64 long long
#define SIZET size_t
#endif // __linux
