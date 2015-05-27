// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifndef NTDDI_VERSION // Allow use of features specific to Windows Vista or later.
#define NTDDI_VERSION NTDDI_WIN7 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef WINVER // Allow use of features specific to Windows Vista or later.
#define WINVER _WIN32_WINNT_WIN7 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT // Allow use of features specific to Windows Vista or later.
#define _WIN32_WINNT _WIN32_WINNT_WIN7 // Change this to the appropriate value to target other versions of Windows.
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
//#include <ppl.h> // async_future
//#include <agents.h> // async_future
#include <process.h> // _beginthreadex
#include <Shlwapi.h> // PathRemoveFileSpecW, PathStripPath
#include <algorithm> // std::sort
//#include <D3D9.h>
#include <Propvarutil.h> // InitPropVariantFromInt16
#include "jpeglib.h" // jpeg_stdio_src

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
	LPWSTR FullPath;
	unsigned long long SizeInBytes;
	SYSTEMTIME DateModified;
	HBITMAP Thumbnail;
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

inline UINT NumberOfDigits(UINT n)
{
	UINT i = 0U;

	if (n == 0)
	{
		return 1U;
	}

	while (n)
	{
		n /= 10;
		i++;
	};
	
	return i;
}

//template <typename T>
//class async_future
//{
//public:
//   template <class Functor>
//   explicit async_future(Functor&& fn)
//   {
//      // Execute the work function in a task group and send the result
//      // to the single_assignment object.
//      Concurrency::_tasks.run([fn, this]() {
//         Concurrency::send(_value, fn());
//       });
//   }
//
//   ~async_future()
//   {
//      // Wait for the task to finish.
//      Concurrency::_tasks.wait();
//   }
//
//   // Retrieves the result of the work function.
//   // This method blocks if the async_future object is still 
//   // computing the value.
//   T get()
//   { 
//      return Concurrency::receive(_value); 
//   }
//
//private:
//   // Executes the asynchronous work function.
//   Concurrency::task_group _tasks;
//
//   // Stores the result of the asynchronous work function.
//   Concurrency::single_assignment<T> _value;
//};