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

#include "targetver.h"

#include <windows.h>
#include <Windowsx.h> // HANDLE_MSG

#include <vector> // std::vector (Include before Strsafe or will give compiler warnings)
#include <list> //std::list
#include <map> // std::map
#include <stdlib.h>
#include <memory.h>
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
#include <atomic> // std::atomic
#include <d2d1_1.h> // D2D1_SIZE_U
#include <wincodec.h> // IWICImagingFactory2

extern "C" {
#include "transupp.h" // Support routines for jpegtran
}

#ifndef _DEBUG
#define OutputDebugStringA(expr) ((void)0)
#define OutputDebugStringW(expr) ((void)0)
#endif

struct GIF_INFO
{
	D2D1_SIZE_U Size;
	D2D1_COLOR_F BackgroundColor;
	UINT m_cxGifImagePixel;
	UINT m_cyGifImagePixel;
	UINT m_uTotalLoopCount;

	GIF_INFO() :
		BackgroundColor(D2D1::ColorF(0U, 0.0F)),
		m_cxGifImagePixel(0U),
		m_cyGifImagePixel(0U),
		m_uTotalLoopCount(0U),
		Size(D2D1::SizeU(0U, 0U))
	{}
};

struct FRAME_INFO
{
	Microsoft::WRL::ComPtr<IWICBitmapSource> pIWICBitmapSource;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> pID2D1Bitmap1;
	D2D1_SIZE_F Size;
	std::wstring Title;
	unsigned char RotationFlag;
	UINT m_uFrameDisposal;
	UINT m_uFrameDelay;
	D2D1_RECT_F m_framePosition;
	bool UserInputFlag;

	FRAME_INFO() :
		pIWICBitmapSource(nullptr),
		pID2D1Bitmap1(nullptr),
		Size(D2D1::SizeF(0.0F, 0.0F)),
		Title(),
		RotationFlag(1U),
		m_uFrameDisposal(0U),
		m_uFrameDelay(0U),
		m_framePosition(D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F)),
		UserInputFlag(false)
	{}
};

struct IMAGEFILE
{
	uint32_t ID;
	std::wstring FullPath;
	size_t SizeInBytes;
	SYSTEMTIME DateModified;
	HRESULT LoadResult;
	GUID guidContainerFormat;
	std::vector<FRAME_INFO> aFrameInfo;
	GIF_INFO GifInfo;

	IMAGEFILE() :
		ID(0U),
		FullPath(),
		SizeInBytes(0U),
		DateModified({ 0 }),
		LoadResult(E_FAIL),
		guidContainerFormat(GUID_NULL),
		aFrameInfo(),
		GifInfo()
	{}
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
inline int NumberOfDigits(int32_t x)
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
inline int NumberOfDigits(char n)
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