// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifndef NTDDI_VERSION // Allow use of features specific to Windows Vista or later.
#define NTDDI_VERSION NTDDI_WIN8 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef WINVER // Allow use of features specific to Windows Vista or later.
#define WINVER _WIN32_WINNT_WIN8 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT // Allow use of features specific to Windows Vista or later.
#define _WIN32_WINNT _WIN32_WINNT_WIN8 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE
#define UNICODE
#endif

// Section below required for TaskDialogIndirect
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#define MAX_PATH_UNICODE 32767

#include "targetver.h"

#include <windows.h>
#include <Windowsx.h> // HANDLE_MSG

#include <vector> // std::vector (Include before Strsafe or will give compiler warnings)
#include <map> // std::map
#include <sapi.h> // Speech API
#include <sphelper.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#define STRSAFE_NO_CB_FUNCTIONS // allow only character count functions as using Unicode exclusively
#include <Strsafe.h> // StringCchPrintfW
#include <math.h> // ceil
#include <Shellapi.h> // CommandLineToArgvW
#include <Shobjidl.h> // IFileOperation
#include <Wininet.h> // required for #include <Shlobj.h>
#include <Shlobj.h> // IActiveDesktop
#include <process.h> // _beginthreadex
#include <Shlwapi.h> // PathRemoveFileSpecW, PathStripPath
#include <algorithm> // std::sort
#include <Propvarutil.h> // InitPropVariantFromInt16
#include "jpeglib.h" // jpeg_stdio_src
#include <wrl\client.h> // Microsoft::WRL::ComPtr
#include <memory> // std::unique_ptr
#include <Pathcch.h> // PathCchRemoveFileSpec
#include <climits> // MIN_INT

extern "C" {
#include "transupp.h" // Support routines for jpegtran
}

#ifndef _DEBUG
#define OutputDebugStringA(expr) ((void)0)
#define OutputDebugStringW(expr) ((void)0)
#endif

struct FILESTRUCT
{
	UINT ID;
	std::wstring FullPath;
	size_t SizeInBytes;
	SYSTEMTIME DateModified;
	//HBITMAP Thumbnail;
};

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease)
	{
		(*ppInterfaceToRelease)->Release();
		*ppInterfaceToRelease = nullptr;
	}
}

// generic solution
template <class T>
int NumberOfDigits(T number)
{
	int digits = 0;
	if (number < 0) digits = 1; // remove this line if '-' counts as a digit
	while (number) {
		number /= 10;
		digits++;
	}
	return digits;
}

// partial specialization optimization for 32-bit numbers
template<>
int NumberOfDigits(int32_t x)
{
	if (x == INT_MIN) return 10 + 1;
	if (x < 0) return NumberOfDigits(-x) + 1;

	if (x >= 10000) {
		if (x >= 10000000) {
			if (x >= 100000000) {
				if (x >= 1000000000)
					return 10;
				return 9;
			}
			return 8;
		}
		if (x >= 100000) {
			if (x >= 1000000)
				return 7;
			return 6;
		}
		return 5;
	}
	if (x >= 100) {
		if (x >= 1000)
			return 4;
		return 3;
	}
	if (x >= 10)
		return 2;
	return 1;
}

// partial-specialization optimization for 8-bit numbers
template <>
int NumberOfDigits(char n)
{
	// if you have the time, replace this with a static initialization to avoid
	// the initial overhead & unnecessary branch
	static char x[256] = { 0 };
	if (x[0] == 0) {
		for (char c = 1; c != 0; c++)
			x[c] = static_cast<char>(NumberOfDigits(static_cast<int32_t>(c)));
		x[0] = 1;
	}
	return x[n];
}