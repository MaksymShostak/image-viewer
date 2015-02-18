#include "stdafx.h"
#include "Direct2DRenderer.h"
#include "HRESULT.h"

bool operator<(const GUID & Left, const GUID & Right)
{
	WCHAR GUIDLeft[GUIDSTRING_MAX] = {0};
	WCHAR GUIDRight[GUIDSTRING_MAX] = {0};

	StringFromGUID2(Left, GUIDLeft, GUIDSTRING_MAX);
	StringFromGUID2(Right, GUIDRight, GUIDSTRING_MAX);

    return (StrCmp(GUIDLeft, GUIDRight) < 0);
}

inline bool AllSpaces(LPCWSTR String)
{
	for (UINT i = 0U; i < wcslen(String); i++)
	{
		if (String[i] != L' ')
		{
			return false;
		}
	}

	return true;
}

std::wstring ReplaceCharInString(
    const std::wstring &source,
    wchar_t charToReplace,
    const std::wstring replaceString
    )
{
    std::wstring result;
	 
    // For each character in source string: 
    const wchar_t * pch = source.c_str();
    while (*pch != L'\0')
    {
        // Found character to be replaced?
        if (*pch == charToReplace)
        {
            result += replaceString;
        }
        else
        {
            // Just copy original character
            result += (*pch);
        }
		 
        // Move to next character
        ++pch;
    }
	 
    return result;
}

HRESULT ReplaceCharInString(
    LPCWSTR source,
	LPWSTR destination,
    WCHAR charToReplace,
    LPCWSTR replaceString
    )
{
	std::wstring wsource = source;
	std::wstring wreplaceString = replaceString;
	std::wstring result = ReplaceCharInString(wsource, charToReplace, wreplaceString);

	HRESULT hr = StringCchCopyW(destination, 1000, result.c_str());

	return hr;
}

void RTrim(LPWSTR String, UINT StringMaxLength, const UINT NumberOfCharsToTrim)
{
	if (String == nullptr || StringMaxLength == 0U || NumberOfCharsToTrim == 0U)
	{
		return;
	}

	UINT StringLength = static_cast<UINT>(wcsnlen(String, StringMaxLength) + 1);

	for (UINT i = StringLength - NumberOfCharsToTrim; i < StringLength; i++)
	{
		String[i] = L'\0';
	}
}

unsigned WINAPI StaticCacheFileNameNext(void* Param)
{
	return ((Direct2DRenderer*)Param)->CacheFileNameNext(g_FileNamePosition);
}

unsigned WINAPI StaticCacheFileNamePrevious(void* Param)
{
	return ((Direct2DRenderer*)Param)->CacheFileNamePrevious(g_FileNamePosition);
}

//
// Initialize members.
//
Direct2DRenderer::Direct2DRenderer() :
    m_hWnd(nullptr),
    m_pD2DFactory(nullptr),
    m_pWICFactory(nullptr),
    m_pRenderTarget(nullptr),
	m_dpiX(96.0f),
	m_dpiY(96.0f),
	m_zoomFactor(1.2f),
	m_zoom(1.0f),
	m_zoomMin(1.0f),
	m_zoomMax(20.0f),
	m_BitmapTranslatePoint(D2D1::Point2F()),
	m_TranslatePoint(D2D1::Point2F()),
	m_TranslatePointEnd(D2D1::Point2F()),
	m_FitToWindow(true),
	m_ScaleToWindow(false),
	Pannable(false),
	m_TransformMatrixTranslation(D2D1::Matrix3x2F::Identity()),
	m_TransformMatrixPanning(D2D1::Matrix3x2F::Identity()),
	m_TransformMatrixScale(D2D1::Matrix3x2F::Identity()),
	m_pDWriteFactory(nullptr),
	m_pTextFormat(nullptr),
	m_pBlackBrush(nullptr),
	m_pWhiteBrush(nullptr),
	hThreadCacheFileNamePrevious(nullptr),
	hThreadCacheFileNameNext(nullptr),
	m_pContextDst(nullptr),
	m_FrameCurrent(0U),
	BackgroundColorBlack(true),
	DeviceResourcesDiscarded(false),
	m_uLoopNumber(0U),
	AnimationRunning(false),
	ConformGIF(false)
{}

//
// Release resources.
//
//Direct2DRenderer::~Direct2DRenderer()
//{
//	CloseHandle(hThreadCacheFileNamePrevious);
//	CloseHandle(hThreadCacheFileNameNext);
//	SafeRelease(&m_pDWriteFactory);
//	SafeRelease(&m_pRenderTarget);
//	SafeRelease(&m_pTextFormat);
//	SafeRelease(&m_pBlackBrush);
//	SafeRelease(&m_pWhiteBrush);
//
//	for (UINT i = 0U; i < m_ImagePrevious.Frames; i++)
//	{
//		SafeRelease(&m_ImagePrevious.aFrameInfo[i].pBitmap);
//	}
//
//	for (UINT i = 0U; i < m_ImageCurrent.Frames; i++)
//	{
//		SafeRelease(&m_ImageCurrent.aFrameInfo[i].pBitmap);
//	}
//
//	for (UINT i = 0U; i < m_ImageNext.Frames; i++)
//	{
//		SafeRelease(&m_ImageNext.aFrameInfo[i].pBitmap);
//	}
//
//	/*delete [] m_ImagePrevious.Title;
//	m_ImagePrevious.Title = nullptr;*/
//
//	/*delete [] m_ImageCurrent.Title;
//	m_ImageCurrent.Title = nullptr;*/
//
//	/*delete [] m_ImageNext.Title;
//	m_ImageNext.Title = nullptr;*/
//
//	SafeRelease(&m_pD2DFactory);
//	//SafeRelease(&m_pContextDst); // Already destroyed by CoUninitialize() call that destroys m_pWICFactory that created this
//	//SafeRelease(&m_pWICFactory); // Already destroyed by CoUninitialize() call
//}

//
//  Called whenever the application needs to display the client
//  window.
//
//  Note that this function will not render anything if the window
//  is occluded (e.g. when the screen is locked).
//  Also, this function will automatically discard device-specific
//  resources if the Direct3D device disappears during function
//  invocation, and will recreate the resources the next time it's
//  invoked.
//
HRESULT Direct2DRenderer::OnRender()
{
    HRESULT hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		if (DeviceResourcesDiscarded)
		{
			LoadBitmapCurrent(g_Files[g_FileNamePosition].FullPath);
			DeviceResourcesDiscarded = false; // only reload once
		}

		D2D1_SIZE_U m_RenderTargetSize = m_pRenderTarget->GetPixelSize(); // in device pixels

		m_pRenderTarget->BeginDraw();
		
		m_pRenderTarget->Clear(D2D1::ColorF(BackgroundColorBlack ? D2D1::ColorF::Black : D2D1::ColorF::White));

		if (m_ImageCurrent.LoadResult == S_OK)
		{
			if (m_FitToWindow)
			{
				m_TransformMatrixScale = D2D1::Matrix3x2F::Identity();
				m_zoom = 1.0f;
				float ScaleFactor = 1.0f;

				m_TranslatePoint = D2D1::Point2F(0.0f, 0.0f);
				m_TranslatePointEnd = D2D1::Point2F(0.0f, 0.0f);

				if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatGif && m_ImageCurrent.Frames > 1U) // CLUDGE, fix with proper function for FitToWindowSize (use CalculateDrawRectangle for motivation)
				{
					if (ConformGIF)
					{
						m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width = static_cast<FLOAT>(m_ImageCurrent.GifInfo.m_cxGifImagePixel); // use this set to conform to standard
						m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height = static_cast<FLOAT>(m_ImageCurrent.GifInfo.m_cyGifImagePixel);
					}
					else
					{
						m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width = static_cast<FLOAT>(m_ImageCurrent.GifInfo.Size.width); // use this set to ignore the aspect ratio metadata (IE does this)
						m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height = static_cast<FLOAT>(m_ImageCurrent.GifInfo.Size.height);
					}				
				}

				if ((m_RenderTargetSize.width >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (m_RenderTargetSize.height >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is larger than image
				{OutputDebugStringW(L"m_bFitToWindow: window is larger than image\n");
					m_BitmapSizeFitToWindow = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size;
					m_zoomMax = 20.0f;
				}
				else if ((m_RenderTargetSize.width >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (m_RenderTargetSize.height < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is shorter than image
				{OutputDebugStringW(L"m_bFitToWindow: window is shorter than image\n");
					ScaleFactor = m_RenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
					m_zoomMax = 20.0f/ScaleFactor;
				}
				else if ((m_RenderTargetSize.width < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (m_RenderTargetSize.height >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is thinner than image
				{OutputDebugStringW(L"m_bFitToWindow: window is thinner than image\n");
					ScaleFactor = m_RenderTargetSize.width/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
					m_zoomMax = 20.0f/ScaleFactor;
				}
				else if ((m_RenderTargetSize.width < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (m_RenderTargetSize.height < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is smaller than image
				{OutputDebugStringW(L"m_bFitToWindow: window is smaller than image\n");
					if (((m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width - m_RenderTargetSize.width)/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) < ((m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height - m_RenderTargetSize.height)/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height))
					{OutputDebugStringW(L"m_bFitToWindow: height constrained\n");
						ScaleFactor = m_RenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
						m_zoomMax = 20.0f/ScaleFactor;
					}
					else
					{OutputDebugStringW(L"m_bFitToWindow: width constrained\n");
						ScaleFactor = m_RenderTargetSize.width/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
						m_zoomMax = 20.0f/ScaleFactor;
					}
				}
			
				m_BitmapSizeFitToWindow.width = ScaleFactor * m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
				m_BitmapSizeFitToWindow.height = ScaleFactor * m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;

				CalculateBitmapTranslatePoint(m_RenderTargetSize);

				Pannable = false;
			}
			else if (m_ScaleToWindow)
			{
				m_TransformMatrixScale = D2D1::Matrix3x2F::Identity();
				m_zoom = (FLOAT)m_RenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
				m_zoomMax = 20.0f;

				m_BitmapSizeFitToWindow.width = (FLOAT)m_RenderTargetSize.height * (m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width)/(m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height);
				m_BitmapSizeFitToWindow.height = (FLOAT)m_RenderTargetSize.height;

				CalculateBitmapTranslatePoint(m_RenderTargetSize);

				Pannable = false;
			}
			else // if not fit to window
			{
				if ((m_RenderTargetSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (m_RenderTargetSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
				{
					Pannable = false;
				}
				else
				{
					Pannable = true;
				}
			}
		
	/*wchar_t buffer[260];
	HRESULT hr = StringCchPrintfW(buffer, 260, L"m_TranslatePoint: (%f, %f)", m_TranslatePoint.x, m_TranslatePoint.y);
	if SUCCEEDED(hr)
	{
		SetWindowTextW(m_hWnd, buffer);
	}*/
			m_TransformMatrixPanning = D2D1::Matrix3x2F::Translation(m_TranslatePoint.x, m_TranslatePoint.y); // Calculate here as there are functions outside of SetTranslate that 

			m_pRenderTarget->SetTransform(/*m_TransformMatrixRotation */ m_TransformMatrixScale * m_TransformMatrixTranslation * m_TransformMatrixPanning);

			//if (g_Files[g_FileNamePosition].Thumbnail) // stops crash in case of NULL pointer to bitmap
			//{
			//	IWICBitmap *pWICBitmap = nullptr;
			//	hr = m_pWICFactory->CreateBitmapFromHBITMAP(g_Files[g_FileNamePosition].Thumbnail, 0, WICBitmapIgnoreAlpha, &pWICBitmap);

			//	//IWICFormatConverter* spConverter = nullptr;
			//	//hr = m_pWICFactory->CreateFormatConverter(&spConverter);
			//	//if (spConverter)
			//	//{
			//	//	spConverter->Initialize(
			//	//		pWICBitmap,
			//	//		GUID_WICPixelFormat32bppPBGRA,
			//	//		WICBitmapDitherTypeNone,
			//	//		NULL,
			//	//		0.0f,
			//	//		WICBitmapPaletteTypeMedianCut
			//	//		);
			//	//}

			//	hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
			//		pWICBitmap, //spConverter,
			//		NULL,
			//		&(m_ImageCurrent.aFrameInfo[m_FrameCurrent].pBitmap)
			//		);

			//	pWICBitmap->Release();
			//	//spConverter->Release();
			//}

			if (m_ImageCurrent.aFrameInfo[m_FrameCurrent].pBitmap) // stops crash in case of NULL pointer to bitmap
			{
				m_pRenderTarget->DrawBitmap(m_ImageCurrent.aFrameInfo[m_FrameCurrent].pBitmap, D2D1::RectF(0.0f, 0.0f, (96.0f/m_dpiX)*m_BitmapSizeFitToWindow.width, (96.0f/m_dpiX)*m_BitmapSizeFitToWindow.height));
			}
			
			/*ID2D1SolidColorBrush *m_pBlueBrush = nullptr;

			hr = m_pRenderTarget->CreateSolidColorBrush(
					D2D1::ColorF(D2D1::ColorF::Blue),
					&m_pBlueBrush
					);
		
			m_pRenderTarget->DrawLine(D2D1::Point2F(-1000.0f, 0), D2D1::Point2F(1000.0f, 0), m_pBlackBrush, 2.0f);
			m_pRenderTarget->DrawLine(D2D1::Point2F(0, -1000.0f), D2D1::Point2F(0, 1000.0f), m_pBlueBrush, 2.0f);*/
		}
		else
		{
			m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			
			/*WCHAR ErrorDescription[106] = L"Image Viewer can't open this picture because the file appears to the damaged, corrupted, or is too large.";

			m_pRenderTarget->DrawTextW(
				ErrorDescription,
				106U,
				m_pTextFormat,
				D2D1::RectF(0.0f, 0.0f, (FLOAT)m_RenderTargetSize.width, (FLOAT)m_RenderTargetSize.height),
				m_pBlackBrush
				);*/

			WCHAR ErrorDescription[SHRT_MAX] = {0};

			HRESULTDecode(m_ImageCurrent.LoadResult, NULL, NULL, ErrorDescription);

			m_pRenderTarget->DrawTextW(
				ErrorDescription,
				static_cast<UINT>(wcsnlen(ErrorDescription, SHRT_MAX) + 1),
				m_pTextFormat,
				D2D1::RectF(0.0f, 0.0f, (96.0f/m_dpiX)*static_cast<FLOAT>(m_RenderTargetSize.width), (96.0f/m_dpiX)*static_cast<FLOAT>(m_RenderTargetSize.height)),
				BackgroundColorBlack ? m_pWhiteBrush : m_pBlackBrush
				);
		}

        hr = m_pRenderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }

	if (SUCCEEDED(hr))
	{
		if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatGif)
		{
			if (m_ImageCurrent.Frames > 1U && m_uLoopNumber == 0U && m_FrameCurrent == 0U)
			{
				if (m_ImageCurrent.aFrameInfo[m_FrameCurrent].UserInputFlag)
				{
					AnimationRunning = false;
					MessageBeep(MB_ICONWARNING);
				}
				else
				{
					m_uLoopNumber = 1U; // increment immediately so that if user stops animation and returns to first frame within first loop, this doesn't fire up again - comparison with loop number max then has to be >= to stop
					AnimationRunning = true;
					// Set the timer according to the delay
					SetTimer(m_hWnd, DELAY_TIMER_ID, m_ImageCurrent.aFrameInfo[m_FrameCurrent].m_uFrameDelay, NULL); // The clock starts ticking immediately after the graphic is rendered
				}
			}
		}
	}

    return hr;
}

//D2D1_RECT_F drawRect;
			//hr = CalculateDrawRectangle(drawRect);
			//if (SUCCEEDED(hr))
			//{
			//	m_pRenderTarget->DrawBitmap(m_ImageCurrent.aFrameInfo[m_FrameCurrent].pBitmap, drawRect);
			//}

bool Direct2DRenderer::RotateAutoEnabled()
{
	return (m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag != 1U) ? true : false; // Assumption: Rotation flag will only exists for JPEGs
}

bool Direct2DRenderer::RotateEnabled()
{
	bool ReturnValue = false;

	if (m_ImageCurrent.guidContainerFormat != GUID_NULL &&
		m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag != 2U &&
		m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag != 4U &&
		m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag != 5U &&
		m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag != 7U) // these rotation flags cannot be rectified by any combination of clockwise/counterclockwise rotations
	{
		if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatJpeg) // can always rotate JPEG
		{
			ReturnValue = true;
		}
		else
		{
			ReturnValue = DecoderHasEncoder.find(m_ImageCurrent.guidContainerFormat)->second;
		}
	}

	return ReturnValue;
}

HRESULT Direct2DRenderer::Rotate(bool Clockwise)
{
	WCHAR FileNameUnicode[MAX_PATH_UNICODE] = L"\\\\?\\";
	WCHAR FileNameTemporary[MAX_PATH_UNICODE] = {0};
	WIN32_FILE_ATTRIBUTE_DATA FileAttributeDataOriginal = {0};
	HANDLE HandleNew = nullptr;

	HRESULT hr = StringCchCatW(FileNameUnicode, MAX_PATH_UNICODE, g_Files[g_FileNamePosition].FullPath);
	if (SUCCEEDED(hr))
	{
		hr = GetFileAttributesExW(FileNameUnicode, GetFileExInfoStandard, &FileAttributeDataOriginal) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (SUCCEEDED(hr))
	{
		hr = StringCchCopyW(FileNameTemporary, MAX_PATH_UNICODE, g_Files[g_FileNamePosition].FullPath);
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, MAX_PATH_UNICODE, L"temp");
	}

	if (SUCCEEDED(hr))
	{
		if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatJpeg)
		{
			hr = RotateJPEG(g_Files[g_FileNamePosition].FullPath, FileNameTemporary, m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag, Clockwise);
		}
		else
		{
			hr = RotateByReencode(m_pWICFactory, g_Files[g_FileNamePosition].FullPath, FileNameTemporary, Clockwise);
			if (FAILED(hr) && hr != WINCODEC_ERR_ABORTED) // delete the temporary file we created in case the above function fails
			{
				if (SUCCEEDED(StringCchCatW(FileNameUnicode, MAX_PATH_UNICODE, L"temp")))
				{
					DeleteFileW(FileNameUnicode); // ignore return value as temp file may not have been created yet when error arose
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = DeleteFileW(FileNameUnicode) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCopyW(FileNameTemporary, MAX_PATH_UNICODE, L"\\\\?\\");
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, MAX_PATH_UNICODE, g_Files[g_FileNamePosition].FullPath);
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, MAX_PATH_UNICODE, L"temp");
	}

	if (SUCCEEDED(hr))
	{
		hr = MoveFileExW(FileNameTemporary, FileNameUnicode, MOVEFILE_WRITE_THROUGH) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		HandleNew = CreateFileW(FileNameUnicode, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		hr = (HandleNew != INVALID_HANDLE_VALUE) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = SetFileTime(HandleNew, &FileAttributeDataOriginal.ftCreationTime, &FileAttributeDataOriginal.ftLastAccessTime, &FileAttributeDataOriginal.ftLastWriteTime) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = CloseHandle(HandleNew) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		LoadBitmapFromFile(m_pWICFactory, g_Files[g_FileNamePosition].FullPath, m_pContextDst, m_pRenderTarget, &m_ImageCurrent);

		ResetRenderingParameters();

		hr = OnRender();
	}

	return hr;
}

HRESULT Direct2DRenderer::RotateByReencode(IWICImagingFactory *pIWICFactory, LPCWSTR FileName, LPCWSTR FileNameTemporary, bool Clockwise)
{
	IWICBitmapDecoderPtr pDecoder;
	//IWICBitmapDecoder *pDecoder = nullptr;
	IWICBitmapDecoderInfoPtr pWICBitmapDecoderInfo;
	//IWICBitmapDecoderInfo *pWICBitmapDecoderInfo = nullptr;
	IWICStreamPtr piFileStream;
	//IWICStream *piFileStream = nullptr;
	IWICBitmapEncoderPtr piEncoder;
	//IWICBitmapEncoder *piEncoder = nullptr;
	BOOL SupportLossless = FALSE;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		FileName,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, // A decoder must be created using the WICDecodeOptions value WICDecodeMetadataCacheOnDemand to perform in-place metadata updates. Using the WICDecodeMetadataCacheOnLoad option causes the decoder to release the file stream necessary to perform the metadata updates.
        &pDecoder
        );

	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetDecoderInfo(&pWICBitmapDecoderInfo);
	}

	if (SUCCEEDED(hr))
	{
		hr =  pWICBitmapDecoderInfo->DoesSupportLossless(&SupportLossless);
	}

	if (SUCCEEDED(hr))
	{
		if (!SupportLossless)
		{
			LPWSTR Type = nullptr;
			UINT NumberOfCharacters = 0U;

			if (SUCCEEDED(pWICBitmapDecoderInfo->GetFriendlyName(0U, NULL, &NumberOfCharacters)))
			{
				Type = new WCHAR[NumberOfCharacters];
				if (FAILED(pWICBitmapDecoderInfo->GetFriendlyName(NumberOfCharacters, Type, &NumberOfCharacters)))
				{
					wmemset(Type, 0, NumberOfCharacters);
				}

				RTrim(Type, NumberOfCharacters, 8U); // 8 for ' Decoder'
			}

			TASKDIALOGCONFIG config = {0};
			TASKDIALOG_BUTTON aCustomButtons[] =
			{
			  {1000, L"&Rotate\nThe quality of the image will be reduced."},
			  {1001, L"Do &not rotate"}
			};
			int nClickedBtn = 0;

			config.cbSize				= sizeof(config);
			config.pButtons				= aCustomButtons;
			config.cButtons				= _countof(aCustomButtons);
			config.dwFlags				= TDF_POSITION_RELATIVE_TO_WINDOW | TDF_USE_COMMAND_LINKS;
			config.hwndParent			= m_hWnd;
			config.nDefaultButton		= 1001;
			config.pszContent			= L"This file format does not support lossless rotation";
			config.pszMainIcon			= TD_WARNING_ICON;
			config.pszMainInstruction	= Type;
			config.pszWindowTitle		= L"Rotate";
	
			hr = TaskDialogIndirect(&config, &nClickedBtn, NULL, NULL);
			if (SUCCEEDED(hr))
			{
				if (nClickedBtn == 1000)
				{
					hr = S_OK;
				}
				else
				{
					hr = WINCODEC_ERR_ABORTED;
				}
			}
			else
			{
				ErrorDescription(hr);
			}

			delete [] Type;
		}
	}

	// Create a file stream.
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateStream(&piFileStream);
	}

	// Initialize our new file stream.
	if (SUCCEEDED(hr))
	{
		hr = piFileStream->InitializeFromFilename(FileNameTemporary, GENERIC_WRITE);
	}

	// Create the encoder.
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateEncoder(m_ImageCurrent.guidContainerFormat, NULL, &piEncoder);
		//if (hr == WINCODEC_ERR_COMPONENTNOTFOUND)
		//{
		//	//no encoder exists
		//}
	}

	// Initialize the encoder
	if (SUCCEEDED(hr))
	{
		hr = piEncoder->Initialize(piFileStream, WICBitmapEncoderNoCache);
	}

	//Process each frame of the image.
	for (UINT i = 0U; i < m_ImageCurrent.Frames && SUCCEEDED(hr); i++)
	{
		//Frame variables.
		IWICBitmapFrameDecodePtr piFrameDecode;
		//IWICBitmapFrameDecode *piFrameDecode = nullptr;
		IWICBitmapPtr pBitmap;
		//IWICBitmap *pBitmap = nullptr;
		IWICBitmapFlipRotatorPtr pFlipRotator;
		//IWICBitmapFlipRotator *pFlipRotator = nullptr;
		IWICBitmapFrameEncodePtr piFrameEncode;
		//IWICBitmapFrameEncode *piFrameEncode = nullptr;
		IPropertyBag2 *pIEncoderOptions = nullptr;
		//IWICMetadataQueryReader *piFrameQReader = nullptr;
		IWICMetadataQueryWriterPtr piFrameQWriter;
		//IWICMetadataQueryWriter *piFrameQWriter = nullptr;
		IWICMetadataBlockWriterPtr piBlockWriter;
		//IWICMetadataBlockWriter *piBlockWriter = nullptr;
		IWICMetadataBlockReaderPtr piBlockReader;
		//IWICMetadataBlockReader *piBlockReader = nullptr;
		WICPixelFormatGUID pixelFormat = GUID_NULL;
		double dpiX = 0.0;
		double dpiY = 0.0;
		UINT width = 0U;
		UINT height = 0U;
		PROPVARIANT	propvariantOrientationFlag;
		PropVariantInit(&propvariantOrientationFlag);

		//Get and create image frame.
		if (SUCCEEDED(hr))
		{
			hr = pDecoder->GetFrame(i, &piFrameDecode);
		}

		if (SUCCEEDED(hr))
		{
			hr = pIWICFactory->CreateBitmapFromSource(piFrameDecode, WICBitmapCacheOnDemand, &pBitmap);
		}

		if (SUCCEEDED(hr))
		{
			hr = pIWICFactory->CreateBitmapFlipRotator(&pFlipRotator);
		}

		if (SUCCEEDED(hr))
		{
			//if (i == m_FrameCurrent)
			//{
				hr = pFlipRotator->Initialize(pBitmap, Clockwise ? WICBitmapTransformRotate90 : WICBitmapTransformRotate270);
			//}
			//else
			//{
			//	hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate0);
			//}

			//switch (*pRotationFlag)
			//{
			//case 2:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformFlipHorizontal);
			//	}
			//	break;
			//case 3:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate180);
			//	}
			//	break;
			//case 4:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformFlipVertical);
			//	}
			//	break;
			//case 5:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, static_cast<WICBitmapTransformOptions>(WICBitmapTransformFlipVertical | WICBitmapTransformRotate90));
			//	}
			//	break;
			//case 6:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate90);
			//	}
			//	break;
			//case 7:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, static_cast<WICBitmapTransformOptions>(WICBitmapTransformFlipVertical | WICBitmapTransformRotate270));
			//	}
			//	break;
			//case 8:
			//	{
			//		hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate270);
			//	}
			//	break;
			//}
		}

		if (SUCCEEDED(hr))
		{
			hr = piEncoder->CreateNewFrame(&piFrameEncode, &pIEncoderOptions);
		}

		//Initialize the encoder.
		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->Initialize(pIEncoderOptions);
		}

		//Get and set size.
		if (SUCCEEDED(hr))
		{
			hr = pFlipRotator->GetSize(&width, &height);
		}

		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->SetSize(width, height);
		}

		//Get and set resolution.
		if (SUCCEEDED(hr))
		{
			hr = pFlipRotator->GetResolution(&dpiX, &dpiY);
		}

		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->SetResolution(dpiX, dpiY);
		}

		//Set pixel format.
		if (SUCCEEDED(hr))
		{
			hr = pFlipRotator->GetPixelFormat(&pixelFormat);
		}

		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->SetPixelFormat(&pixelFormat);
		}

		//Copy metadata using metadata block reader/writer.
		if (SUCCEEDED(hr))
		{
			hr = piFrameDecode->QueryInterface(
				IID_IWICMetadataBlockReader,
				(LPVOID*)&piBlockReader);
			
			if (hr == E_NOINTERFACE) //Some data formats do not have metadata e.g. Bitmap
			{
				hr = S_OK;
			}
			else if (SUCCEEDED(hr))
			{
				hr = piFrameEncode->QueryInterface(
				IID_IWICMetadataBlockWriter,
				(LPVOID*)&piBlockWriter);

				if (SUCCEEDED(hr))
				{
					piBlockWriter->InitializeFromBlockReader(piBlockReader); // ignore return value
				}

				if (SUCCEEDED(hr))
				{
					if (SUCCEEDED(piFrameEncode->GetMetadataQueryWriter(&piFrameQWriter)))
					{
						//Set rotation flag to 1 if possible
						if (SUCCEEDED(InitPropVariantFromUInt16(1, &propvariantOrientationFlag)))
						{
							piFrameQWriter->SetMetadataByName(L"/app1/ifd/{ushort=274}", &propvariantOrientationFlag);
							piFrameQWriter->SetMetadataByName(L"/xmp/tiff:Orientation", &propvariantOrientationFlag);
						}
					}
				}
			}
		}		

		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->WriteSource(
				static_cast<IWICBitmapSource*> (pFlipRotator),
				NULL); // Using NULL enables JPEG loss-less encoding.
		}

		//Commit the frame.
		if (SUCCEEDED(hr))
		{
			hr = piFrameEncode->Commit();
		}
		
		//SafeRelease(&piFrameDecode);
		//SafeRelease(&pBitmap);
		//SafeRelease(&pFlipRotator);
		//SafeRelease(&piFrameEncode);
		//SafeRelease(&pIEncoderOptions);
		//SafeRelease(&piFrameQReader);
		//SafeRelease(&piFrameQWriter);
		//SafeRelease(&piBlockWriter);
		//SafeRelease(&piBlockReader);
		PropVariantClear(&propvariantOrientationFlag);
	}

	if (SUCCEEDED(hr))
	{
		piEncoder->Commit();
	}

	if (SUCCEEDED(hr))
	{
		piFileStream->Commit(STGC_ONLYIFCURRENT);
	}

	//SafeRelease(&pDecoder);
	//SafeRelease(&pWICBitmapDecoderInfo);
	//SafeRelease(&piFileStream);
	//SafeRelease(&piEncoder);

	//if (SUCCEEDED(hr))
	//{
	//	hr = DeleteFileWithIFO(NULL, FileName, false, true); // null hWnd to prevent RETURNEDFROMDELETEFILEWITHIFO message
	//}

	//HANDLE DeleteFileTransaction = INVALID_HANDLE_VALUE;

	//if (SUCCEEDED(hr))
	//{
	//	DeleteFileTransaction = CreateTransaction(NULL, 0, NULL, 0, 0, INFINITE, NULL);
	//	if (DeleteFileTransaction != INVALID_HANDLE_VALUE)
	//	{
	//		hr = S_OK;
	//	}
	//	else
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	}
	//}

	//if (SUCCEEDED(hr))
	//{
	//	if (DeleteFileTransactedW(FileName, DeleteFileTransaction)) //  "\\?\"
	//	{
	//		hr = S_OK;
	//	}
	//	else
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	}
	//}

	/*if (SUCCEEDED(hr))
	{
		hr = RenameFileWithIFO(FileNameTemporary, FileName);
	}*/

	//if (SUCCEEDED(hr))
	//{
	//	if (CommitTransaction(DeleteFileTransaction))
	//	{
	//		hr = S_OK;
	//	}
	//	else
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	}
	//}
	//else
	//{
	//	if (RollbackTransaction(DeleteFileTransaction))
	//	{
	//		hr = S_OK;
	//	}
	//	else
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	}
	//}

	//if (SUCCEEDED(hr))
	//{
	//	if (CloseHandle(DeleteFileTransaction))
	//	{
	//		hr = S_OK;
	//	}
	//	else
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//	}
	//}

	//if (FAILED(hr) && hr != WINCODEC_ERR_ABORTED)
	//{
	//	DeleteFileWithIFO(NULL, FileNameTemporary, true, true); // null hWnd to prevent RETURNEDFROMDELETEFILEWITHIFO message
	//}

	return hr;
}

int read_1_byte(FILE * File)
{
	return getc(File);
  /*int c = getc(File);
  if (c == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return c;*/
}

/* Read 2 bytes, convert to unsigned int */
/* All 2-byte quantities in JPEG markers are MSB first */
unsigned int read_2_bytes(FILE * File)
{
  int c1 = getc(File);
  /*if (c1 == EOF)
    ERREXIT("Premature EOF in JPEG file");*/
  int c2 = getc(File);
  /*if (c2 == EOF)
    ERREXIT("Premature EOF in JPEG file");*/
  return (((unsigned int) c1) << 8) + ((unsigned int) c2);
}

HRESULT Direct2DRenderer::SetJPEGOrientation(LPCWSTR FileName)
{	
	FILE * file = nullptr;		/* My JPEG file */
	unsigned char exif_data[65536L] = {0};
	unsigned int length = 0U;
	unsigned int i = 0U;
	bool is_motorola = false; /* Flag for byte order */
	unsigned int offset = 0U;
	unsigned int number_of_tags = 0U;
	unsigned int tagnum = 0U;

	// Open input file
	if (_wfopen_s(&file, FileName, L"rb+") != 0)
	{
		return HRESULT_FROM_WIN32(110); // ERROR_OPEN_FAILED
    }

	/* Read File head, check for JPEG SOI + Exif APP1 */
	for (i = 0U; i < 4U; i++)
	{
		exif_data[i] = (unsigned char) read_1_byte(file);
	}
	
	if (exif_data[0] != 0xFF ||
		exif_data[1] != 0xD8 ||
		exif_data[2] != 0xFF ||
		exif_data[3] != 0xE1)
	{
		return E_FAIL;
	}
	
	/* Get the marker parameter length count */
	length = read_2_bytes(file);

	/* Length includes itself, so must be at least 2 */
	/* Following Exif data length must be at least 6 */
	if (length < 8U)
	{
		return E_FAIL;
	}
	
	length -= 8U;
	/* Read Exif head, check for "Exif" */
	for (i = 0U; i < 6U; i++)
	{
		exif_data[i] = (unsigned char) read_1_byte(file);
	}
	
	if (exif_data[0] != 0x45 ||
		exif_data[1] != 0x78 ||
		exif_data[2] != 0x69 ||
		exif_data[3] != 0x66 ||
		exif_data[4] != 0 ||
		exif_data[5] != 0)
	{
		return E_FAIL;
	}
	
	/* Read Exif body */
	for (i = 0; i < length; i++)
	{
		exif_data[i] = (unsigned char) read_1_byte(file);
	}

	if (length < 12) return E_FAIL; /* Length of an IFD entry */

	/* Discover byte order */
	if (exif_data[0] == 0x49 && exif_data[1] == 0x49)
	{
		is_motorola = false;
	}
	else if (exif_data[0] == 0x4D && exif_data[1] == 0x4D)
	{
		is_motorola = true;
	}
	else
	{
		return E_FAIL;
	}

	/* Check Tag Mark */
	if (is_motorola)
	{
		if (exif_data[2] != 0) return E_FAIL;
		if (exif_data[3] != 0x2A) return E_FAIL;
	}
	else
	{
		if (exif_data[3] != 0) return E_FAIL;
		if (exif_data[2] != 0x2A) return E_FAIL;
	}

	/* Get first IFD offset (offset to IFD0) */
	if (is_motorola)
	{
		if (exif_data[4] != 0) return E_FAIL;
		if (exif_data[5] != 0) return E_FAIL;
		offset = exif_data[6];
		offset <<= 8;
		offset += exif_data[7];
	}
	else
	{
		if (exif_data[7] != 0) return E_FAIL;
		if (exif_data[6] != 0) return E_FAIL;
		offset = exif_data[5];
		offset <<= 8;
		offset += exif_data[4];
	}

	if (offset > length - 2)
	{
		return E_FAIL; /* check end of data segment */
	}
	
	/* Get the number of directory entries contained in this IFD */
	if (is_motorola)
	{
		number_of_tags = exif_data[offset];
		number_of_tags <<= 8;
		number_of_tags += exif_data[offset+1];
	}
	else
	{
		number_of_tags = exif_data[offset+1];
		number_of_tags <<= 8;
		number_of_tags += exif_data[offset];
	}

	if (number_of_tags == 0)
	{
		return E_FAIL;
	}
	
	offset += 2;
	
	/* Search for Orientation Tag in IFD0 */
	for (;;)
	{
		if (offset > length - 12) return E_FAIL; /* check end of data segment */

		/* Get Tag number */
		if (is_motorola)
		{
			tagnum = exif_data[offset];
			tagnum <<= 8;
			tagnum += exif_data[offset+1];
		}
		else
		{
			tagnum = exif_data[offset+1];
			tagnum <<= 8;
			tagnum += exif_data[offset];
		}
		
		if (tagnum == 0x0112) break; /* found Orientation Tag */
		
		if (--number_of_tags == 0)
		{
			return E_FAIL;
		}

		offset += 12;
	}

	/* Set the Orientation value */
	if (is_motorola)
	{
		exif_data[offset+2] = 0; /* Format = unsigned short (2 octets) */
		exif_data[offset+3] = 3;
		exif_data[offset+4] = 0; /* Number Of Components = 1 */
		exif_data[offset+5] = 0;
		exif_data[offset+6] = 0;
		exif_data[offset+7] = 1;
		exif_data[offset+8] = 0;
		exif_data[offset+9] = (unsigned char)1;
		exif_data[offset+10] = 0;
		exif_data[offset+11] = 0;
	}
	else
	{
		exif_data[offset+2] = 3; /* Format = unsigned short (2 octets) */
		exif_data[offset+3] = 0;
		exif_data[offset+4] = 1; /* Number Of Components = 1 */
		exif_data[offset+5] = 0;
		exif_data[offset+6] = 0;
		exif_data[offset+7] = 0;
		exif_data[offset+8] = (unsigned char)1;
		exif_data[offset+9] = 0;
		exif_data[offset+10] = 0;
		exif_data[offset+11] = 0;
	}
	fseek(file, (4 + 2 + 6 + 2) + offset, SEEK_SET);
	fwrite(exif_data + 2 + offset, 1, 10, file);
	fclose(file);

	return S_OK;
}

HRESULT Direct2DRenderer::RotateJPEG(LPCWSTR FileName, LPCWSTR FileNameTemporary, USHORT RotationFlag, bool Clockwise)
{
	HRESULT hr = HRESULT_FROM_WIN32(ChangeOrientation(FileName, FileNameTemporary, RotationFlag, Clockwise ? 1 : 0, 0));
	
	if (FAILED(hr) && hr == HRESULT_FROM_WIN32(ERROR_INVALID_DATA)) // if failed because perfect rotation not possible
	{
		TASKDIALOGCONFIG config = {0};
		TASKDIALOG_BUTTON aCustomButtons[] =
		{
			{1000, L"&Rotate\nSome edge pixels will be lost."},
			{1001, L"Do &not rotate"}
		};
		int nClickedBtn = 0;

		config.cbSize				= sizeof(config);
		config.pButtons				= aCustomButtons;
		config.cButtons				= _countof(aCustomButtons);
		config.dwFlags				= TDF_POSITION_RELATIVE_TO_WINDOW | TDF_USE_COMMAND_LINKS;
		config.hwndParent			= m_hWnd;
		config.nDefaultButton		= 1001;
		config.pszContent			= L"This file cannot be rotated losslessly because of its dimensions";
		config.pszMainIcon			= TD_WARNING_ICON;
		config.pszMainInstruction	= L"JPEG";
		config.pszWindowTitle		= L"Rotate";
	
		hr = TaskDialogIndirect(&config, &nClickedBtn, NULL, NULL);
		if (SUCCEEDED(hr))
		{
			if (nClickedBtn == 1000)
			{
				hr = HRESULT_FROM_WIN32(ChangeOrientation(FileName, FileNameTemporary, RotationFlag, Clockwise ? 1 : 0, 1));
			}
			else
			{
				return WINCODEC_ERR_ABORTED;
			}
		}
		else
		{
			ErrorDescription(hr);
		}
	}

	if (SUCCEEDED(hr)) // attempt to change Orientation metadata to 1
	{
		if (RotationFlag != 1U)
		{
			hr = SetJPEGOrientation(FileNameTemporary);
			/*IWICBitmapDecoder *pDecoder = nullptr;
			IWICBitmapFrameDecode *pSource = nullptr;
			IWICFastMetadataEncoder *pFME = nullptr;
			IWICMetadataQueryWriter *pFMEQW = nullptr;
			PROPVARIANT propvariantOrientationFlag = {0};
			PropVariantInit(&propvariantOrientationFlag);

			hr = pIWICFactory->CreateDecoderFromFilename(
				FileNameTemporary,
				NULL,
				GENERIC_READ | GENERIC_WRITE,
				WICDecodeMetadataCacheOnDemand, // A decoder must be created using the WICDecodeOptions value WICDecodeMetadataCacheOnDemand to perform in-place metadata updates. Using the WICDecodeMetadataCacheOnLoad option causes the decoder to release the file stream necessary to perform the metadata updates.
				&pDecoder
				);

			if (SUCCEEDED(hr))
			{
				hr = pDecoder->GetFrame(0U, &pSource);
			}
			else
			{
				MessageBoxW(m_hWnd, L"pIWICFactory->CreateDecoderFromFilename", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			if (SUCCEEDED(hr))
			{
				hr = pIWICFactory->CreateFastMetadataEncoderFromFrameDecode(pSource, &pFME);
			}
			else
			{
				MessageBoxW(m_hWnd, L"pDecoder->GetFrame", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			if (SUCCEEDED(hr))
			{
 				hr = pFME->GetMetadataQueryWriter(&pFMEQW);
			}
			else
			{
				MessageBoxW(m_hWnd, L"pIWICFactory->CreateFastMetadataEncoderFromFrameDecode", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			if (SUCCEEDED(hr))
			{
				hr = InitPropVariantFromUInt16(1U, &propvariantOrientationFlag);
			}
			else
			{
				MessageBoxW(m_hWnd, L"pFME->GetMetadataQueryWriter", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFMEQW->SetMetadataByName(L"/app1/ifd/{ushort=274}", &propvariantOrientationFlag);
				if (FAILED(hr))
				{
					MessageBoxW(m_hWnd, L"pFMEQW->SetMetadataByName", L"Info", MB_ICONINFORMATION | MB_OK);
					hr = pFMEQW->SetMetadataByName(L"/xmp/tiff:Orientation", &propvariantOrientationFlag);
				}
			}
			else
			{
				MessageBoxW(m_hWnd, L"InitPropVariantFromUInt16", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			if (SUCCEEDED(hr))
			{
				hr = pFME->Commit(); //  this changes the last modified date, so do date changes after this
			}
			else
			{
				MessageBoxW(m_hWnd, L"pFMEQW->SetMetadataByName", L"Info", MB_ICONINFORMATION | MB_OK);
			}

			SafeRelease(&pDecoder);
			SafeRelease(&pSource);
			SafeRelease(&pFME);
			SafeRelease(&pFMEQW);
			PropVariantClear(&propvariantOrientationFlag);

			if (FAILED(hr))
			{
				ErrorDescription(hr);
				//MessageBoxW(m_hWnd, L"Could not set orientation metadata", L"Info", MB_ICONINFORMATION | MB_OK);
				hr = S_OK;
			}
			*/
		}
	}

	return hr;
}

HRESULT Direct2DRenderer::RotateByMetadata(IWICImagingFactory *pIWICFactory, LPCWSTR FileName, USHORT *pRotationFlag,	bool /*Clockwise*/)
{
	IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pSource = nullptr;
	IWICFastMetadataEncoder *pFME = nullptr;
	IWICMetadataQueryWriter *pFMEQW = nullptr;
	PROPVARIANT propvariantOrientationFlag = {0};
	PropVariantInit(&propvariantOrientationFlag);

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		FileName,
        NULL,
        GENERIC_READ | GENERIC_WRITE,
        WICDecodeMetadataCacheOnDemand, // A decoder must be created using the WICDecodeOptions value WICDecodeMetadataCacheOnDemand to perform in-place metadata updates. Using the WICDecodeMetadataCacheOnLoad option causes the decoder to release the file stream necessary to perform the metadata updates.
        &pDecoder
        );

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"pIWICFactory->CreateDecoderFromFilename\n");
        // Create the initial frame.
        hr = pDecoder->GetFrame(0U, &pSource);
    }

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"pDecoder->GetFrame\n");
		hr = pIWICFactory->CreateFastMetadataEncoderFromFrameDecode(pSource, &pFME);
	}

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pIWICFactory->CreateFastMetadataEncoderFromFrameDecode\n");
 		hr = pFME->GetMetadataQueryWriter(&pFMEQW);
	}

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pFME->GetMetadataQueryWriter\n");
		hr = InitPropVariantFromUInt16(*pRotationFlag, &propvariantOrientationFlag);
	}

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"InitPropVariantFromUInt16\n");
		hr = pFMEQW->SetMetadataByName(L"/app1/ifd/{ushort=274}", &propvariantOrientationFlag);
	}
	
	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pFMEQW->SetMetadataByName\n");
		hr = pFME->Commit();
	}

	SafeRelease(&pDecoder);
    SafeRelease(&pSource);
	SafeRelease(&pFME);
	SafeRelease(&pFMEQW);
	PropVariantClear(&propvariantOrientationFlag);

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pFME->Commit\n");
		LoadBitmapFromFile(m_pWICFactory, FileName, m_pContextDst, m_pRenderTarget, &m_ImageCurrent);

		ResetRenderingParameters();

		hr = OnRender();
	}

	return hr;
}

HRESULT Direct2DRenderer::ActualSize()
{
	m_FitToWindow = false;

	m_TransformMatrixScale = D2D1::Matrix3x2F::Identity();
	m_zoom = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width/m_BitmapSizeFitToWindow.width;
	m_zoomMax = 20.0f*m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width/m_BitmapSizeFitToWindow.width;
	m_BitmapSizeFitToWindow = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size;

	D2D1_SIZE_U WindowSize = m_pRenderTarget->GetPixelSize();

	CalculateBitmapTranslatePoint(WindowSize);

	m_TranslatePoint = D2D1::Point2F(0.0f, 0.0f);
	m_TranslatePointEnd = D2D1::Point2F(0.0f, 0.0f);

	return OnRender();
}

unsigned Direct2DRenderer::CacheFileNameNext(UINT FileNamePositionToWorkFrom)
{
	if ((FileNamePositionToWorkFrom + 1U) < (UINT)g_Files.size())
	{
		FileNamePositionNext = FileNamePositionToWorkFrom + 1U;
	}
	else
	{
		FileNamePositionNext = 0U;
	}

	HRESULT hr = LoadBitmapFromFile(m_pWICFactory, g_Files[FileNamePositionNext].FullPath, m_pContextDst, m_pRenderTarget, &m_ImageNext);
	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			return CacheFileNameNext(((FileNamePositionToWorkFrom + 1U) < (UINT)g_Files.size()) ? FileNamePositionToWorkFrom + 1U : 0U);
		}
		else
		{
			return (unsigned)-hr;
		}
	}

	return hr;
}

unsigned Direct2DRenderer::CacheFileNamePrevious(UINT FileNamePositionToWorkFrom)
{
	if (FileNamePositionToWorkFrom != 0U)
	{
		FileNamePositionPrevious = FileNamePositionToWorkFrom - 1U;
	}
	else
	{
		FileNamePositionPrevious = (UINT)g_Files.size() - 1U;
	}

	HRESULT hr = LoadBitmapFromFile(m_pWICFactory, g_Files[FileNamePositionPrevious].FullPath, m_pContextDst, m_pRenderTarget, &m_ImagePrevious);
	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			return CacheFileNamePrevious((FileNamePositionToWorkFrom != 0U)? FileNamePositionToWorkFrom - 1U : (UINT)g_Files.size() - 1U);
		}
		else
		{
			return (unsigned)-hr;
		}
	}

	return hr;
}

// Recalculate translation that would center image every time the size of the image or render target changes
inline void Direct2DRenderer::CalculateBitmapTranslatePoint(D2D1_SIZE_U RenderTargetSize)
{
	m_BitmapTranslatePoint = D2D1::Point2F(floor((RenderTargetSize.width/2.0f) - (m_BitmapSizeFitToWindow.width/2.0f)), floor((RenderTargetSize.height/2.0f) - ((m_BitmapSizeFitToWindow.height/2.0f)))); // use floor as must be whole pixels or get blurring!

	m_TransformMatrixTranslation = D2D1::Matrix3x2F::Translation((96.0f/m_dpiX)*m_BitmapTranslatePoint.x, (96.0f/m_dpiX)*m_BitmapTranslatePoint.y);
}

/******************************************************************
*                                                                 *
*  Direct2DRenderer::CalculateDrawRectangle()                     *
*                                                                 *
*  Calculates a specific rectangular area of the hwnd             *
*  render target to draw a bitmap containing the current          *
*  composed frame.                                                *
*                                                                 *
******************************************************************/

HRESULT Direct2DRenderer::CalculateDrawRectangle(D2D1_RECT_F &drawRect)
{
    HRESULT hr = S_OK;
	RECT rcClient = {0};

    // Top and left of the client rectangle are both 0
    if (!GetClientRect(m_hWnd, &rcClient))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the area to display the image
        // Center the image if the client rectangle is larger
		drawRect.left = (static_cast<FLOAT>(rcClient.right) - m_ImageCurrent.GifInfo.m_cxGifImagePixel) / 2.f;
        drawRect.top = (static_cast<FLOAT>(rcClient.bottom) - m_ImageCurrent.GifInfo.m_cyGifImagePixel) / 2.f;
        drawRect.right = drawRect.left + m_ImageCurrent.GifInfo.m_cxGifImagePixel;
        drawRect.bottom = drawRect.top + m_ImageCurrent.GifInfo.m_cyGifImagePixel;

        // If the client area is resized to be smaller than the image size, scale
        // the image, and preserve the aspect ratio
        FLOAT aspectRatio = static_cast<FLOAT>(m_ImageCurrent.GifInfo.m_cxGifImagePixel) /
            static_cast<FLOAT>(m_ImageCurrent.GifInfo.m_cyGifImagePixel);

        if (drawRect.left < 0)
        {
            FLOAT newWidth = static_cast<FLOAT>(rcClient.right);
            FLOAT newHeight = newWidth / aspectRatio;
            drawRect.left = 0;
            drawRect.top = (static_cast<FLOAT>(rcClient.bottom) - newHeight) / 2.f;
            drawRect.right = newWidth;
            drawRect.bottom = drawRect.top + newHeight;
        }

        if (drawRect.top < 0)
        {
            FLOAT newHeight = static_cast<FLOAT>(rcClient.bottom);
            FLOAT newWidth = newHeight * aspectRatio;
            drawRect.left = (static_cast<FLOAT>(rcClient.right) - newWidth) / 2.f;
            drawRect.top = 0;
            drawRect.right = drawRect.left + newWidth;
            drawRect.bottom = newHeight;
        }
    }

    return hr;
}

//
// Create resources which are not bound
// to any device. Their lifetime effectively extends for the
// duration of the app. These resources include the Direct2D,
// DirectWrite, and WIC factories; and a DirectWrite Text Format object
// (used for identifying particular font characteristics) and
// a Direct2D geometry.
//
HRESULT Direct2DRenderer::CreateDeviceIndependentResources()
{OutputDebugStringW(L"Direct2DRenderer::CreateDeviceIndependentResources\n");
	static const WCHAR msc_fontName[] = L"Segoe UI";
    static const FLOAT msc_fontSize = 12.0f;

    // Create a Direct2D factory.
	#if defined(DEBUG) || defined(_DEBUG)
        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_MULTI_THREADED,
            options,
            &m_pD2DFactory
            );
	#else
		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_MULTI_THREADED,
			&m_pD2DFactory
			);
	#endif

    if (SUCCEEDED(hr))
    {OutputDebugStringW(L"D2D1CreateFactory\n");
        // Create WIC factory.
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pWICFactory)
            );
    }

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"CoCreateInstance\n");
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_ISOLATED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {OutputDebugStringW(L"DWriteCreateFactory\n");
        // Create a DirectWrite text format object.
        hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            msc_fontSize,
            L"", //locale
            &m_pTextFormat
            );
    }

    if (SUCCEEDED(hr))
    {OutputDebugStringW(L"m_pDWriteFactory->CreateTextFormat\n");
        // Center the text horizontally and vertically.
        hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"m_pTextFormat->SetTextAlignment\n");
        m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

    return hr;
}

//
//  This method creates resources which are bound to a particular
//  Direct3D device. It's all centralized here, in case the resources
//  need to be recreated in case of Direct3D device loss (eg. display
//  change, remoting, removal of video card, etc).
//
HRESULT Direct2DRenderer::CreateDeviceResources()
{OutputDebugStringW(L"Direct2DRenderer::CreateDeviceResources\n");
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {OutputDebugStringW(L"!m_pRenderTarget\n");
		RECT rc = {0};

        if (GetClientRect(m_hWnd, &rc))
		{OutputDebugStringW(L"GetClientRect(m_hWnd, &rc)\n");
			hr = S_OK;
		}
		else
		{OutputDebugStringW(L"GetClientRect(m_hWnd, &rc) FAILED\n");
			hr = HRESULT_FROM_WIN32(GetLastError());// E_FAIL; Does this even throw a LastError?
		}

		if (SUCCEEDED(hr))
		{
			// Create a Direct2D render target.
			hr = m_pD2DFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(m_hWnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
				&m_pRenderTarget
				);
		}

		if (SUCCEEDED(hr))
        {OutputDebugStringW(L"m_pD2DFactory->CreateHwndRenderTarget\n");
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
        }

		if (SUCCEEDED(hr))
        {OutputDebugStringW(L"m_pRenderTarget->CreateSolidColorBrush (Black)\n");
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
        }

		if (SUCCEEDED(hr))
		{OutputDebugStringW(L"m_pRenderTarget->CreateSolidColorBrush (White)\n");
			hr = m_pD2DFactory->ReloadSystemMetrics();
		}

		if (SUCCEEDED(hr))
		{OutputDebugStringW(L"m_pD2DFactory->ReloadSystemMetrics\n");
			m_pD2DFactory->GetDesktopDpi(&m_dpiX, &m_dpiY);
		}

		if (SUCCEEDED(hr))
		{OutputDebugStringW(L"m_pD2DFactory->GetDesktopDpi\n");
			DWORD dwSize = MAX_PATH_UNICODE;
			WCHAR ICMProfileName[MAX_PATH_UNICODE] = {0};

			HDC hDC = GetDC(m_hWnd);
			if (hDC)
			{OutputDebugStringW(L"GetDC\n");
				if (GetICMProfileW(hDC, &dwSize, ICMProfileName))
				{OutputDebugStringW(L"GetICMProfileW\n");
					hr = m_pWICFactory->CreateColorContext(&m_pContextDst);
					if (SUCCEEDED(hr))
					{OutputDebugStringW(L"pIWICFactory->CreateColorContext(&pContextDst)\n");
						hr = m_pContextDst->InitializeFromFilename(ICMProfileName);
					}
				}
			}
			if (ReleaseDC(m_hWnd, hDC) == 1)
			{
				OutputDebugStringW(L"ReleaseDC\n");
			}
		}
    }

    return hr;
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void Direct2DRenderer::DiscardDeviceResources()
{
	for (UINT i = 0U; i < m_ImagePrevious.Frames; i++)
	{
		SafeRelease(&m_ImagePrevious.aFrameInfo[i].pBitmap);
	}

	for (UINT i = 0U; i < m_ImageCurrent.Frames; i++)
	{
		SafeRelease(&m_ImageCurrent.aFrameInfo[i].pBitmap);
	}

	for (UINT i = 0U; i < m_ImageNext.Frames; i++)
	{
		SafeRelease(&m_ImageNext.aFrameInfo[i].pBitmap);
	}

	/*delete [] m_ImagePrevious.Title;
	m_ImagePrevious.Title = nullptr;*/

	/*delete [] m_ImageCurrent.Title;
	m_ImageCurrent.Title = nullptr;*/

	/*delete [] m_ImageNext.Title;
	m_ImageNext.Title = nullptr;*/

	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pWhiteBrush);
	SafeRelease(&m_pContextDst);
	SafeRelease(&m_pRenderTarget);

	DeviceResourcesDiscarded = true;
}

//
//  If the application receives a WM_SIZE message, this method resizes the render target appropriately.
//
void Direct2DRenderer::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
		CalculateBitmapTranslatePoint(D2D1::SizeU(width, height));

        // Note: This method can fail, but it's okay to ignore the error here -- it will be repeated on the next call to EndDraw.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

inline HRESULT Direct2DRenderer::GIF_GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo)
{
	IWICMetadataQueryReaderPtr pWICMetadataQueryReader;
	//IWICMetadataQueryReader *pWICMetadataQueryReader = nullptr;
	PROPVARIANT propValue = {0};
    PropVariantInit(&propValue);

	HRESULT hr = pWICBitmapFrameDecode->GetMetadataQueryReader(&pWICMetadataQueryReader);
	if (SUCCEEDED(hr))
    {
        hr = pWICMetadataQueryReader->GetMetadataByName(L"/imgdesc/Left", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                FrameInfo->m_framePosition.left = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pWICMetadataQueryReader->GetMetadataByName(L"/imgdesc/Top", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                FrameInfo->m_framePosition.top = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pWICMetadataQueryReader->GetMetadataByName(L"/imgdesc/Width", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                FrameInfo->m_framePosition.right = static_cast<FLOAT>(propValue.uiVal) + FrameInfo->m_framePosition.left;
				FrameInfo->Size.width = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pWICMetadataQueryReader->GetMetadataByName(L"/imgdesc/Height", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                FrameInfo->m_framePosition.bottom = static_cast<FLOAT>(propValue.uiVal) + FrameInfo->m_framePosition.top;
				FrameInfo->Size.height = static_cast<FLOAT>(propValue.uiVal);
            }
            PropVariantClear(&propValue);
        }
    }

	if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(pWICMetadataQueryReader->GetMetadataByName(L"/grctlext/UserInputFlag", &propValue)))
        {
            hr = (propValue.vt == VT_BOOL ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
				FrameInfo->UserInputFlag = propValue.boolVal ? true : false;
            }
            PropVariantClear(&propValue);
        }

		if (FAILED(hr))
		{
			FrameInfo->UserInputFlag = false;
			hr = S_OK;
		}
	}

    if (SUCCEEDED(hr))
    {
        // Get delay from the optional Graphic Control Extension
        if (SUCCEEDED(pWICMetadataQueryReader->GetMetadataByName(L"/grctlext/Delay", &propValue)))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL); 
            if (SUCCEEDED(hr))
            {
                // Convert the delay retrieved in 10 ms units to a delay in 1 ms units
                hr = UIntMult(propValue.uiVal, 10U, &FrameInfo->m_uFrameDelay);
            }
            PropVariantClear(&propValue);
        }
        else
        {
            // Failed to get delay from graphic control extension. Possibly a
            // single frame image (non-animated gif)
            FrameInfo->m_uFrameDelay = 0U;
        }

        if (SUCCEEDED(hr) && !ConformGIF)
        {
            // Insert an artificial delay to ensure rendering for gif with very small
            // or 0 delay.  This delay number is picked to match with most browsers' 
            // gif display speed.
            //
            // This will defeat the purpose of using zero delay intermediate frames in 
            // order to preserve compatibility. If this is removed, the zero delay 
            // intermediate frames will not be visible.
            if (FrameInfo->m_uFrameDelay < 33U) // was 90U, chosen to be 33 to go with maximum refresh rate of 30Hz
            {
                FrameInfo->m_uFrameDelay = 90U;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
		hr = pWICMetadataQueryReader->GetMetadataByName(L"/grctlext/Disposal", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI1) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                FrameInfo->m_uFrameDisposal = propValue.bVal;
				if (!(0U <= FrameInfo->m_uFrameDisposal && FrameInfo->m_uFrameDisposal <= 3U))
				{
					hr = E_FAIL;
				}
            }
        }
		
		if (FAILED(hr))
        {
            // Failed to get the disposal method, use default. Possibly a non-animated gif.
            FrameInfo->m_uFrameDisposal = DM_UNDEFINED;
			hr = S_OK;
        }
    }

    PropVariantClear(&propValue);

	//SafeRelease(&pWICMetadataQueryReader);

	return hr;
}

inline HRESULT Direct2DRenderer::GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo)
{
	IWICMetadataQueryReaderPtr pQueryReader;
	//IWICMetadataQueryReader *pQueryReader = nullptr;

	PROPVARIANT propvariantOrientationFlag = {0};
	PropVariantInit(&propvariantOrientationFlag);
	PROPVARIANT propvariantImageDescription = {0};
	PropVariantInit(&propvariantImageDescription);

	HRESULT hr = pWICBitmapFrameDecode->GetMetadataQueryReader(&pQueryReader);
	if (SUCCEEDED(hr))
	{
		hr = pQueryReader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &propvariantOrientationFlag);
		// if cannot find EXIF orientation try xmp
		if (FAILED(hr))
		{
			pQueryReader->GetMetadataByName(L"/xmp/tiff:Orientation", &propvariantOrientationFlag);
		}

		FrameInfo->RotationFlag = PropVariantToUInt16WithDefault(propvariantOrientationFlag, 1);
		/*if (!(1 <= FrameInfo->RotationFlag && FrameInfo->RotationFlag <= 8)) // do not change value of rotation flag, deal with incorrect data in other functions e.g. rotate will set these to 1
		{
			FrameInfo->RotationFlag = 1;
		}*/
		//hr = pQueryReader->GetMetadataByName(L"/com/TextEntry", &propvariantImageDescription); // usually information added by jpeg encoders, so pointless to display user
		hr = pQueryReader->GetMetadataByName(L"/app1/ifd/{ushort=270}", &propvariantImageDescription);
		if (SUCCEEDED(hr))
		{
			FrameInfo->Title = new WCHAR[USHRT_MAX];
			wmemset(FrameInfo->Title, 0, USHRT_MAX);

			hr = PropVariantToString(propvariantImageDescription, FrameInfo->Title, USHRT_MAX);
			if (SUCCEEDED(hr))
			{
				hr = AllSpaces(FrameInfo->Title) ? E_FAIL : S_OK;
			}

			if (FAILED(hr))
			{
				delete [] FrameInfo->Title;
			}
		}

		if (FAILED(hr))
		{
			FrameInfo->Title = nullptr;
			hr = S_OK;
		}
	}
	else // if cannot get QueryReader
	{
		FrameInfo->RotationFlag = 1U;
		FrameInfo->Title = nullptr;

		hr = S_OK;
	}

	//SafeRelease(&pQueryReader);
	PropVariantClear(&propvariantOrientationFlag);
	PropVariantClear(&propvariantImageDescription);

	return hr;
}

//
// Creates a Direct2D bitmap from the specified file name.
//
HRESULT Direct2DRenderer::LoadBitmapFromFile(
	IWICImagingFactory *pIWICFactory,
	LPCWSTR FileName,
	IWICColorContext *pContextDst,
    ID2D1RenderTarget *pRenderTarget,
    IMAGE_INFO *ImageInfo
    )
{
	ImageInfo->LoadResult = E_FAIL;

	IWICBitmapDecoderPtr pDecoder;

    //IWICBitmapDecoder *pDecoder = nullptr;
	UINT MaximumBitmapSize = pRenderTarget->GetMaximumBitmapSize();

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
        FileName,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
		&pDecoder
        );
	if (FAILED(hr)) // try to load without caching metadata
	{
		hr = pIWICFactory->CreateDecoderFromFilename(
			FileName,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&pDecoder
			);
	}

	/*for (UINT i = 0U; i < ImageInfo->Frames; i++)
	{
		SafeRelease(&ImageInfo->aFrameInfo[i].pBitmap); // something weird going on here...
	}*/

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"pIWICFactory->CreateDecoderFromFilename\n");
        hr = pDecoder->GetFrameCount(&(ImageInfo->Frames));
    }

	if (SUCCEEDED(hr))
    {
		ImageInfo->aFrameInfo = new FRAME_INFO[ImageInfo->Frames];
    }
	
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetContainerFormat(&(ImageInfo->guidContainerFormat));
	}

	if (SUCCEEDED(hr))
	{
		if (ImageInfo->guidContainerFormat == GUID_ContainerFormatGif)
		{
			hr = GIF_GetGlobalMetadata(pDecoder, ImageInfo); // if returns no size?
		}
	}

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pDecoder->GetFrameCount\n");
		for (UINT i = 0U; i < ImageInfo->Frames; i++)
		{WCHAR buffer[260] = {0}; StringCchPrintfW(buffer, 260, L"Frame: %d\n", i); OutputDebugStringW(buffer);
			
			IWICBitmapFrameDecodePtr pSource;
			//IWICBitmapFrameDecode *pSource = nullptr;
			IWICColorTransformPtr pColorTransform;
			//IWICColorTransform *pColorTransform = nullptr;
			IWICFormatConverterPtr pConverter;
			//IWICFormatConverter *pConverter = nullptr;
			//IWICBitmap *pBitmap = nullptr;
			//IWICBitmapFlipRotator *pFlipRotator = nullptr;
			UINT colorContextCount = 0U;

			hr = pDecoder->GetFrame(i, &pSource);

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"pDecoder->GetFrame(i, &pSource)\n");
				UINT width = 0U;
				UINT height = 0U;

				if (SUCCEEDED(pSource->GetSize(&width, &height)))
				{
					if (width > MaximumBitmapSize || height > MaximumBitmapSize)
					{
						hr = D2DERR_MAX_TEXTURE_SIZE_EXCEEDED;
					}
				}
			}

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"Checked size\n");
				if (ImageInfo->guidContainerFormat == GUID_ContainerFormatGif)
				{
					hr = GIF_GetFrameMetadata(pSource, &(ImageInfo->aFrameInfo[i]));
					ImageInfo->aFrameInfo[i].RotationFlag = 1U; // Not located in metadata
				}
				else
				{
					hr = GetFrameMetadata(pSource, &(ImageInfo->aFrameInfo[i]));
				}
			}

			// Transform colours
			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"GetMetadata(pSource, ImageInfo->aFrameInfo[i])\n");
				hr = pSource->GetColorContexts(0U, NULL, &colorContextCount);
				if (FAILED(hr) || colorContextCount == 0U)
				{OutputDebugStringW(L"pSource->GetColorContexts(0U, NULL, &colorContextCount) FAILED or returned 0\n");
					colorContextCount = 1U;
					hr = S_OK;
				}

				IWICColorContext **contexts = new IWICColorContext*[colorContextCount];
				for (UINT i = 0U; i < colorContextCount; i++)
				{
					if (SUCCEEDED(hr))
					{
						hr = pIWICFactory->CreateColorContext(&contexts[i]);
					}
				}

				if (SUCCEEDED(hr))
				{OutputDebugStringW(L"pIWICFactory->CreateColorContext(&contexts[i])\n");
					hr = pSource->GetColorContexts(colorContextCount, contexts, &colorContextCount);
					if (FAILED(hr) || colorContextCount == 0U)
					{OutputDebugStringW(L"pSource->GetColorContexts(colorContextCount, contexts, &colorContextCount) FAILED or returned 0\n");
						colorContextCount = 1U;

						WCHAR sRGBProfileName[MAX_PATH_UNICODE] = {0};
						DWORD dwSize = MAX_PATH_UNICODE;

						if (GetStandardColorSpaceProfileW(NULL, LCS_sRGB, sRGBProfileName, &dwSize))
						{OutputDebugStringW(L"GetStandardColorSpaceProfileW(NULL, LCS_sRGB, sRGBProfileName, &dwSize)\n");
							hr = contexts[0]->InitializeFromFilename(sRGBProfileName);
						}
					}
				}

				if (SUCCEEDED(hr))
				{OutputDebugStringW(L"ColorContext(s) initialised\n");
					WICPixelFormatGUID PixelFormat;
					pSource->GetPixelFormat(&PixelFormat);
					if (SUCCEEDED(hr))
					{OutputDebugStringW(L"pSource->GetPixelFormat\n");
						hr = m_pWICFactory->CreateColorTransformer(&pColorTransform);
						if (SUCCEEDED(hr))
						{OutputDebugStringW(L"m_pWICFactory->CreateColorTransformer\n");
							for (UINT i = 0U; i < colorContextCount; i++)
							{
								hr = pColorTransform->Initialize(pSource, contexts[i], pContextDst, PixelFormat);
								if (SUCCEEDED(hr))
								{OutputDebugStringW(L"pColorTransform->Initialize(pSource, contexts[i], pContextDst, PixelFormat)\n");
									break;
								}
							}
						}
					}
				}

				for (UINT i = 0U; i < colorContextCount; i++)
				{
					SafeRelease(&contexts[i]);
				}
				delete[] contexts;

				if (FAILED(hr))
				{OutputDebugStringW(L"pColorTransform->Initialize(pSource, contexts[i], pContextDst, PixelFormat) FAILED\n");
					colorContextCount = 0U;
					hr = S_OK;
				}
			}

			//if (SUCCEEDED(hr))
			//{OutputDebugStringW(L"Handled color transform\n");
			//	if (ImageInfo->aFrameInfo[i].RotationFlag != 1U)
			//	{
			//		hr = pIWICFactory->CreateBitmapFromSource((colorContextCount == 0) ? (IWICBitmapSource*)pSource : (IWICBitmapSource*)pColorTransform, WICBitmapCacheOnLoad, &pBitmap);
			//		if (SUCCEEDED(hr))
			//		{OutputDebugStringW(L"CreateBitmapFromSource\n");
			//			hr = pIWICFactory->CreateBitmapFlipRotator(&pFlipRotator);
			//			if (SUCCEEDED(hr))
			//			{OutputDebugStringW(L"pIWICFactory->CreateBitmapFlipRotator\n");
			//				switch (ImageInfo->aFrameInfo[i].RotationFlag)
			//				{
			//				case 2U:
			//					{OutputDebugStringW(L"WICBitmapTransformFlipHorizontal\n");
			//						hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformFlipHorizontal);
			//					}
			//					break;
			//				case 3U:
			//					{OutputDebugStringW(L"WICBitmapTransformRotate180\n");
			//						hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate180);
			//					}
			//					break;
			//				case 4U:
			//					{OutputDebugStringW(L"WICBitmapTransformFlipVertical\n");
			//						hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformFlipVertical);
			//					}
			//					break;
			//				case 5U:
			//					{OutputDebugStringW(L"WICBitmapTransformFlipVertical | WICBitmapTransformRotate90\n");
			//						hr = pFlipRotator->Initialize(pBitmap, static_cast<WICBitmapTransformOptions>(WICBitmapTransformFlipVertical | WICBitmapTransformRotate90));
			//					}
			//					break;
			//				case 6U:
			//					{OutputDebugStringW(L"WICBitmapTransformRotate90\n");
			//						hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate90);
			//					}
			//					break;
			//				case 7U:
			//					{OutputDebugStringW(L"WICBitmapTransformFlipVertical | WICBitmapTransformRotate270\n");
			//						hr = pFlipRotator->Initialize(pBitmap, static_cast<WICBitmapTransformOptions>(WICBitmapTransformFlipVertical | WICBitmapTransformRotate270));
			//					}
			//					break;
			//				case 8U:
			//					{OutputDebugStringW(L"WICBitmapTransformRotate270\n");
			//						hr = pFlipRotator->Initialize(pBitmap, WICBitmapTransformRotate270);
			//					}
			//					break;
			//				}
			//			}
			//		}
			//	}
			//}

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"Handled rotation\n");
				// Convert the image format to 32bppPBGRA
				// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
				hr = pIWICFactory->CreateFormatConverter(&pConverter);
			}

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"CreateFormatConverter\n");
				hr = pConverter->Initialize(
						//ImageInfo->aFrameInfo[i].RotationFlag != 1 ? (IWICBitmapSource*)pFlipRotator : ((colorContextCount == 0U) ? (IWICBitmapSource*)pSource : (IWICBitmapSource*)pColorTransform),
						(colorContextCount == 0U) ? (IWICBitmapSource*)pSource : (IWICBitmapSource*)pColorTransform,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.0f,
						WICBitmapPaletteTypeMedianCut
						);
			}
			
			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"pConverter->Initialize\n");
				// Create a Direct2D bitmap from the WIC bitmap.
				hr = pRenderTarget->CreateBitmapFromWicBitmap(
					pConverter,
					NULL,
					&(ImageInfo->aFrameInfo[i].pBitmap)
					);
			}

			if (SUCCEEDED(hr))
			{
				if (ImageInfo->guidContainerFormat != GUID_ContainerFormatGif)
				{
					ImageInfo->aFrameInfo[i].Size = ImageInfo->aFrameInfo[i].pBitmap->GetSize();
				}
			}

			//SafeRelease(&pSource);
			//SafeRelease(&pColorTransform);
			//SafeRelease(&pConverter);
			//SafeRelease(&pBitmap);
			//SafeRelease(&pFlipRotator);
		}
	}

    //SafeRelease(&pDecoder);

	if (ImageInfo->guidContainerFormat == GUID_ContainerFormatGif)
	{
		for (UINT i = 0U; i < ImageInfo->Frames; i++)
		{
			ID2D1BitmapRenderTargetPtr m_pFrameComposeRT;
			//ID2D1BitmapRenderTarget *m_pFrameComposeRT = nullptr;

			hr = pRenderTarget->CreateCompatibleRenderTarget(D2D1::SizeF(static_cast<FLOAT>(ImageInfo->GifInfo.Size.width), static_cast<FLOAT>(ImageInfo->GifInfo.Size.height)), &m_pFrameComposeRT); // Composed frames have the same sizes as the global gif image size
			if (SUCCEEDED(hr))
			{
				// Start producing the bitmap
				m_pFrameComposeRT->BeginDraw();

				// If first frame
				if (i == 0U)
				{
					// Draw background
					m_pFrameComposeRT->Clear(ImageInfo->GifInfo.BackgroundColor);
				}
				else
				{
					switch (ImageInfo->aFrameInfo[i - 1U].m_uFrameDisposal)
					{
					case DM_UNDEFINED:
					case DM_NONE:
						{
							// We simply draw on the previous frames
							m_pFrameComposeRT->DrawBitmap(ImageInfo->aFrameInfo[i - 1U].pBitmap, NULL);
						}
						break;
					case DM_BACKGROUND: // Clear the area covered by the current raw frame with background color
						{
								// Clip the render target to the size of the raw frame
								m_pFrameComposeRT->PushAxisAlignedClip(&ImageInfo->aFrameInfo[i - 1U].m_framePosition, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
						
								m_pFrameComposeRT->Clear(ImageInfo->GifInfo.BackgroundColor);

								// Remove the clipping
								m_pFrameComposeRT->PopAxisAlignedClip();
						}
						break;
					case DM_PREVIOUS:
						{
							// We restore the previous composed frame first
							if (i >= 2)
							{
								m_pFrameComposeRT->DrawBitmap(ImageInfo->aFrameInfo[i - 2U].pBitmap, NULL);
							}
						}
						break;
					}
				}

				// Produce the frame
				m_pFrameComposeRT->DrawBitmap(ImageInfo->aFrameInfo[i].pBitmap, ImageInfo->aFrameInfo[i].m_framePosition);

				hr = m_pFrameComposeRT->EndDraw();
			}

			if (SUCCEEDED(hr))
			{
				D2D1_SIZE_U BitmapSize = ImageInfo->aFrameInfo[i].pBitmap->GetPixelSize();
				D2D1_SIZE_U RTSize = m_pFrameComposeRT->GetPixelSize();
				if (BitmapSize.width != RTSize.width || BitmapSize.height != RTSize.height)
				{
					SafeRelease(&ImageInfo->aFrameInfo[i].pBitmap);
					D2D1_BITMAP_PROPERTIES props;
					m_pFrameComposeRT->GetDpi(&props.dpiX, &props.dpiY);
					props.pixelFormat = m_pFrameComposeRT->GetPixelFormat();

					hr = pRenderTarget->CreateBitmap(RTSize, props, &ImageInfo->aFrameInfo[i].pBitmap);
				}

				if (SUCCEEDED(hr))
				{
					hr = ImageInfo->aFrameInfo[i].pBitmap->CopyFromRenderTarget(NULL, m_pFrameComposeRT, NULL);
				}
			}
			//SafeRelease(&m_pFrameComposeRT);
		}
	}

	ImageInfo->LoadResult = hr;

    return hr;
}

HRESULT Direct2DRenderer::SetHwnd(HWND hWnd)
{
	if (hWnd == nullptr)
	{
		return E_POINTER;
	}

	if (!IsWindow(hWnd))
	{
		return ERROR_INVALID_HANDLE;
	}

	m_hWnd = hWnd;

	return S_OK;
}

HRESULT Direct2DRenderer::ToggleBackgroundColor()
{
	BackgroundColorBlack = !BackgroundColorBlack;

	return OnRender();
}

HRESULT Direct2DRenderer::ZoomIn(UINT x, UINT y)
{
	if (m_zoom == m_zoomMax) // if already at maximum zoom
	{
		return S_OK; // do nothing 
	}

	//x *= (96.0f/m_dpiX);
	//y *= (96.0f/m_dpiY);

	m_FitToWindow = false;
	m_ScaleToWindow = false;

	D2D1_POINT_2F m_ZoomCentre = D2D1::Point2F();

	D2D1_SIZE_U WindowSize = m_pRenderTarget->GetPixelSize();

	if ((WindowSize.width >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window larger than image
	{
		m_ZoomCentre = D2D1::Point2F(m_BitmapSizeFitToWindow.width/2.0f, m_BitmapSizeFitToWindow.height/2.0f);
	}
	else if ((WindowSize.width < m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window thinner than image
	{
		m_ZoomCentre = D2D1::Point2F((FLOAT)x - m_BitmapTranslatePoint.x - m_TranslatePoint.x, m_BitmapSizeFitToWindow.height/2.0f);
	}
	else if ((WindowSize.width >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height < m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window shorter than image
	{
		m_ZoomCentre = D2D1::Point2F(m_BitmapSizeFitToWindow.width/2.0f, (FLOAT)y - m_BitmapTranslatePoint.y - m_TranslatePoint.y);
	}
	else
	{
		m_ZoomCentre = D2D1::Point2F((FLOAT)x - m_BitmapTranslatePoint.x - m_TranslatePoint.x, (FLOAT)y - m_BitmapTranslatePoint.y - m_TranslatePoint.y);
	}

	m_ZoomCentre.x *= (96.0f/m_dpiX);
	m_ZoomCentre.y *= (96.0f/m_dpiY);

	if (m_zoom*m_zoomFactor < m_zoomMax)
	{
		m_zoom = m_zoomFactor*m_zoom;
		m_TransformMatrixScale = m_TransformMatrixScale * D2D1::Matrix3x2F::Scale(m_zoomFactor, m_zoomFactor, m_ZoomCentre);
	}
	else
	{
		m_zoom = m_zoomMax;
		m_TransformMatrixScale = m_TransformMatrixScale * D2D1::Matrix3x2F::Scale(D2D1::SizeF(m_zoomMax/m_zoom, m_zoomMax/m_zoom), m_ZoomCentre);
	}

	return OnRender();
}

HRESULT Direct2DRenderer::ZoomOut(UINT x, UINT y)
{
	if (m_zoom == m_zoomMin) // if already at minimum zoom
	{
		return S_OK; // do nothing 
	}

	D2D1_POINT_2F m_ZoomCentre;

	if (m_zoom/m_zoomFactor > m_zoomMin)
	{
		m_zoom = m_zoom/m_zoomFactor;

		m_FitToWindow = false;
		m_ScaleToWindow = false;

		D2D1_SIZE_U WindowSize = m_pRenderTarget->GetPixelSize();

		if ((WindowSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
		{OutputDebugStringW(L"window larger than image\n");
			m_ZoomCentre = D2D1::Point2F(m_BitmapSizeFitToWindow.width/2.0f, m_BitmapSizeFitToWindow.height/2.0f);
			m_ZoomCentre.x *= (96.0f/m_dpiX);
			m_ZoomCentre.y *= (96.0f/m_dpiY);
			m_TransformMatrixScale = D2D1::Matrix3x2F::Scale(m_zoom, m_zoom, m_ZoomCentre);
			m_TranslatePoint = D2D1::Point2F(0.0f, 0.0f);
			m_TranslatePointEnd = D2D1::Point2F(0.0f, 0.0f);
		}
		//else if ((WindowSize.width < (m_zoom)*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= (m_zoom)*m_BitmapSizeFitToWindow.height)) // if window thinner than image
		//{OutputDebugStringW(L"window thinner than image\n");
		//	m_ZoomCentre = D2D1::Point2F((FLOAT)x - m_BitmapTranslatePoint.x, (1.2f*m_BitmapSizeFitToWindow.height)/2.0f);
		////m_ZoomCentre = D2D1::Point2F((m_BitmapSizeFitToWindow.width/1.2f)/2.0f, (m_BitmapSizeFitToWindow.height/1.2f)/2.0f);
		//	m_ZoomCentre.x *= (96.0f/m_dpiX);
		//	m_ZoomCentre.y *= (96.0f/m_dpiY);
		//	m_TransformMatrixScale = D2D1::Matrix3x2F::Scale(m_zoom, m_zoom, m_ZoomCentre);
		//	//m_TranslatePoint.y = 0.0f;
		//	//m_TranslatePointEnd.y = 0.0f;
		//}
		//else if ((WindowSize.width >= (m_zoom)*m_BitmapSizeFitToWindow.width) && (WindowSize.height < (m_zoom)*m_BitmapSizeFitToWindow.height)) // if window shorter than image
		//{OutputDebugStringW(L"window shorter than image\n");
		//	m_ZoomCentre = D2D1::Point2F((1.2f*m_BitmapSizeFitToWindow.width)/2.0f, (FLOAT)y - m_BitmapTranslatePoint.y);
		////m_ZoomCentre = D2D1::Point2F((m_BitmapSizeFitToWindow.width/1.2f)/2.0f, (m_BitmapSizeFitToWindow.height/1.2f)/2.0f);
		//	m_ZoomCentre.x *= (96.0f/m_dpiX);
		//	m_ZoomCentre.y *= (96.0f/m_dpiY);
		//	m_TransformMatrixScale = D2D1::Matrix3x2F::Scale(m_zoom, m_zoom, m_ZoomCentre);
		//	//m_TranslatePoint.x = 0.0f;
		//	//m_TranslatePointEnd.x = 0.0f;
		//}
		else
		{OutputDebugStringW(L"window smaller than image\n");
			m_ZoomCentre = D2D1::Point2F((FLOAT)x - m_BitmapTranslatePoint.x - m_TranslatePoint.x, (FLOAT)y - m_BitmapTranslatePoint.y - m_TranslatePoint.y);
			m_ZoomCentre.x *= (96.0f/m_dpiX);
			m_ZoomCentre.y *= (96.0f/m_dpiY);
			m_TransformMatrixScale = m_TransformMatrixScale * D2D1::Matrix3x2F::Scale(1.0f/m_zoomFactor, 1.0f/m_zoomFactor, m_ZoomCentre);
		}
	}
	else
	{
		m_FitToWindow = true;
	}

	return OnRender();
}

HRESULT Direct2DRenderer::FitToWindow()
{
	if (m_FitToWindow)
	{
		return S_OK;
	}
	else
	{
		m_ScaleToWindow = false;
		m_FitToWindow = true;
		return OnRender();
	}
}

HRESULT Direct2DRenderer::ScaleToWindow()
{
	if (m_ScaleToWindow)
	{
		return S_OK;
	}
	else
	{
		m_FitToWindow = false;
		m_ScaleToWindow = true;
		return OnRender();
	}
}

HRESULT Direct2DRenderer::SetTranslate(int x, int y)
{
	D2D1_SIZE_U WindowSize = m_pRenderTarget->GetPixelSize();

	if ((WindowSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
	{OutputDebugStringW(L"SetTranslate: window larger than image\n");
		return S_OK;
	}

	//if ((WindowSize.width < m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window thinner than image
	//{OutputDebugStringW(L"SetTranslate: window thinner than image\n");
	//	m_TranslatePoint.x = m_TranslatePointEnd.x + (96.0f/m_dpiX)*(FLOAT)x;

	//	/*if (m_TranslatePoint.x > ((FLOAT)m_BitmapSizeFitToWindow.width)/m_zoom)
	//	{
	//		m_TranslatePoint.x = ((FLOAT)m_BitmapSizeFitToWindow.width)/m_zoom;
	//	}
	//	else if (m_TranslatePoint.x < -((FLOAT)m_BitmapSizeFitToWindow.width)/m_zoom)
	//	{
	//		m_TranslatePoint.x = -((FLOAT)m_BitmapSizeFitToWindow.width)/m_zoom;
	//	}*/

	//	m_TranslatePoint.y = m_TranslatePointEnd.y;
	//}
	//else if ((WindowSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height < m_zoom*m_BitmapSizeFitToWindow.height)) // if window shorter than image
	//{OutputDebugStringW(L"SetTranslate: window shorter than image\n");
	//	m_TranslatePoint.x = m_TranslatePointEnd.x;
	//	m_TranslatePoint.y = m_TranslatePointEnd.y + (96.0f/m_dpiY)*(FLOAT)y;

	//	/*if (m_TranslatePoint.y > (m_BitmapSizeFitToWindow.height/2.0f)/m_zoom)
	//	{
	//		m_TranslatePoint.y = (m_BitmapSizeFitToWindow.height/2.0f)/m_zoom;
	//	}
	//	else if (m_TranslatePoint.y < -(m_BitmapSizeFitToWindow.height/2.0f)/m_zoom)
	//	{
	//		m_TranslatePoint.y = -(m_BitmapSizeFitToWindow.height/2.0f)/m_zoom;
	//	}*/
	//}
	//else if ((WindowSize.width < m_zoom*m_BitmapSizeFitToWindow.width) && (WindowSize.height < m_zoom*m_BitmapSizeFitToWindow.height)) // if window smaller than image
	{OutputDebugStringW(L"SetTranslate: window smaller than image\n");
		m_TranslatePoint.x = m_TranslatePointEnd.x + (96.0f/m_dpiX)*(FLOAT)x;

		/*if (m_TranslatePoint.x > m_zoom*m_BitmapSizeFitToWindow.width/2.0f)
		{
			m_TranslatePoint.x = m_zoom*m_BitmapSizeFitToWindow.width/2.0f;
		}
		else if (m_TranslatePoint.x < -m_zoom*m_BitmapSizeFitToWindow.width/2.0f)
		{
			m_TranslatePoint.x = -m_zoom*m_BitmapSizeFitToWindow.width/2.0f;
		}*/

		m_TranslatePoint.y = m_TranslatePointEnd.y + (96.0f/m_dpiY)*(FLOAT)y;

		/*if (m_TranslatePoint.y > m_zoom*m_BitmapSizeFitToWindow.height/2.0f)
		{
			m_TranslatePoint.y = m_zoom*m_BitmapSizeFitToWindow.height/2.0f;
		}
		else if (m_TranslatePoint.y < -m_zoom*m_BitmapSizeFitToWindow.height/2.0f)
		{
			m_TranslatePoint.y = -m_zoom*m_BitmapSizeFitToWindow.height/2.0f;
		}*/
	}
	
	return OnRender();
}

HRESULT Direct2DRenderer::SetCurrentErrorCode(HRESULT ErrorCode)
{
	m_ImageCurrent.LoadResult = ErrorCode;

	return OnRender();
}

void Direct2DRenderer::SetDragEnd()
{
	m_TranslatePointEnd = m_TranslatePoint;
}

HRESULT Direct2DRenderer::ReloadAfterSort()
{
	HRESULT hr = S_OK;

	if (g_Files.size() > 1)
	{
		hThreadCacheFileNameNext = (HANDLE)_beginthreadex( // NATIVE CODE
			NULL, // void *security,
			sizeof(LPVOID), // unsigned stack_size,
			StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * ),
			this, // void *arglist,
			0U, // unsigned initflag,
			NULL // unsigned *thrdaddr
			);

		hr = hThreadCacheFileNameNext ? S_OK : HRESULT_FROM_WIN32(GetLastError());

		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNamePrevious = (HANDLE)_beginthreadex( // NATIVE CODE
				NULL, // void *security,
				sizeof(LPVOID), // unsigned stack_size,
				StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * ),
				this, // void *arglist,
				0U, // unsigned initflag,
				NULL // unsigned *thrdaddr
				);

			hr = hThreadCacheFileNamePrevious ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	}

	return hr;
}

HRESULT Direct2DRenderer::LoadBitmapCurrent(LPCWSTR FileName)
{
	HRESULT hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{
		LoadBitmapFromFile(m_pWICFactory, FileName, m_pContextDst, m_pRenderTarget, &m_ImageCurrent);

		ResetRenderingParameters();
	}

	if (hThreadCreateFileNameVectorFromDirectory)
	{
		hr = (WaitForSingleObject(hThreadCreateFileNameVectorFromDirectory, INFINITE) != WAIT_FAILED) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		SetTitleBarText(); // ignore return value
	}

	if (g_Files.size() > 1)
	{
		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNameNext = (HANDLE)_beginthreadex( // NATIVE CODE
				NULL, // void *security,
				sizeof(LPVOID), // unsigned stack_size,
				StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * ),
				this, // void *arglist,
				0U, // unsigned initflag,
				NULL // unsigned *thrdaddr
				);

			hr = hThreadCacheFileNameNext ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}

		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNamePrevious = (HANDLE)_beginthreadex( // NATIVE CODE
				NULL, // void *security,
				sizeof(LPVOID), // unsigned stack_size,
				StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * ),
				this, // void *arglist,
				0U, // unsigned initflag,
				NULL // unsigned *thrdaddr
				);

			hr = hThreadCacheFileNamePrevious ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	}

	return hr;
}

HRESULT Direct2DRenderer::ResetRenderingParameters()
{
	if (!m_ScaleToWindow)
	{
		m_FitToWindow = true;
	}

	m_TranslatePoint = D2D1::Point2F(0.0f, 0.0f);
	m_TranslatePointEnd = D2D1::Point2F(0.0f, 0.0f);

	m_FrameCurrent = 0U;
	m_uLoopNumber = 0U;

	return KillTimer(m_hWnd, DELAY_TIMER_ID) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT Direct2DRenderer::OnAnimationStartStop()
{
	HRESULT hr = S_OK;

	if (!AnimationRunning)
	{
		hr = SetTimer(m_hWnd, DELAY_TIMER_ID, m_ImageCurrent.aFrameInfo[m_FrameCurrent].m_uFrameDelay, NULL) ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		AnimationRunning = !AnimationRunning;
	}

	return hr;
}

HRESULT Direct2DRenderer::OnDelete()
{
	HRESULT hr = S_OK;

	WaitForSingleObject(hThreadCacheFileNameNext, INFINITE);
	WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE); // cludge

	if (g_FileNamePosition != 0U)
	{
		FileNamePositionPrevious = g_FileNamePosition - 1U;
	}
	else
	{
		FileNamePositionPrevious = ((UINT)g_Files.size()) - 1U;
	}

	g_FileNamePosition = FileNamePositionNext;

	//delete [] m_ImagePrevious.Title;
	//m_ImagePrevious.Title = nullptr;

	for (UINT i = 0U; i < m_ImageCurrent.Frames; i++)
	{
		SafeRelease(&m_ImageCurrent.aFrameInfo[m_FrameCurrent].pBitmap);
	}

	m_ImageCurrent = m_ImageNext;

	ResetRenderingParameters();

	hThreadCacheFileNameNext = (HANDLE)_beginthreadex( // NATIVE CODE
		NULL, // void *security,
		sizeof(LPVOID), // unsigned stack_size,
		StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * ),
		this, // void *arglist,
		0U, // unsigned initflag,
		NULL // unsigned *thrdaddr
		);

	if (hThreadCacheFileNameNext)
	{
		hr = S_OK;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = OnRender();
	}

	if (SUCCEEDED(hr))
	{
		SetTitleBarText(); // ignore return value
	}

	return hr;
}

HRESULT Direct2DRenderer::OnFrameNext()
{
	if (m_ImageCurrent.Frames == 1U || AnimationRunning)
	{
		return S_OK;
	}

	if (m_FrameCurrent + 1U < m_ImageCurrent.Frames)
	{
		m_FrameCurrent = m_FrameCurrent + 1U;
	}
	else
	{
		m_FrameCurrent = 0U;
	}

	SetTitleBarText();

	return OnRender();
}

HRESULT Direct2DRenderer::GIF_OnFrameNext()
{
	KillTimer(m_hWnd, DELAY_TIMER_ID);

	if (m_ImageCurrent.Frames == 1U || !AnimationRunning || m_ImageCurrent.aFrameInfo[m_FrameCurrent].UserInputFlag)
	{
		if (m_ImageCurrent.aFrameInfo[m_FrameCurrent].UserInputFlag)
		{
			MessageBeep(MB_ICONASTERISK);
		}

		AnimationRunning = false;
		return S_OK;
	}

	if (m_FrameCurrent + 1U < m_ImageCurrent.Frames)
	{
		m_FrameCurrent = m_FrameCurrent + 1U;
	}
	else
	{
		m_uLoopNumber++;
		if (m_ImageCurrent.GifInfo.m_uTotalLoopCount != 0U && m_uLoopNumber >= m_ImageCurrent.GifInfo.m_uTotalLoopCount)
		{
			return S_OK;
		}
		else
		{
			m_FrameCurrent = 0U;
		}
	}

	AnimationRunning = true;
	// Set the timer according to the delay
	SetTimer(m_hWnd, DELAY_TIMER_ID, m_ImageCurrent.aFrameInfo[m_FrameCurrent].m_uFrameDelay, NULL);

	return OnRender();
}

HRESULT Direct2DRenderer::OnFramePrevious()
{
	if (m_ImageCurrent.Frames == 1U || AnimationRunning)
	{
		return S_OK;
	}

	if (m_FrameCurrent != 0)
	{
		m_FrameCurrent = m_FrameCurrent - 1U;
	}
	else
	{
		m_FrameCurrent = m_ImageCurrent.Frames - 1U;
	}

	SetTitleBarText();

	return OnRender();
}

HRESULT Direct2DRenderer::OnNext()
{
	HRESULT hr = S_OK;

	WaitForSingleObject(hThreadCacheFileNameNext, INFINITE);
	//TerminateThread(hThreadCacheFileNamePrevious, 0);
	WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE); // cludge

	FileNamePositionPrevious = g_FileNamePosition;
	g_FileNamePosition = FileNamePositionNext;

	for (UINT i = 0U; i < m_ImagePrevious.Frames; i++)
	{
		SafeRelease(&m_ImagePrevious.aFrameInfo[i].pBitmap);
	}

	delete [] m_ImagePrevious.aFrameInfo;

	//delete [] m_ImagePrevious.Title;
	//m_ImagePrevious.Title = nullptr;

	m_ImagePrevious = m_ImageCurrent;
	m_ImageCurrent = m_ImageNext;

	ResetRenderingParameters();

	hThreadCacheFileNameNext = (HANDLE)_beginthreadex( // NATIVE CODE
		NULL, // void *security,
		sizeof(LPVOID), // unsigned stack_size,
		StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * ),
		this, // void *arglist,
		0U, // unsigned initflag,
		NULL // unsigned *thrdaddr
		);

	if (hThreadCacheFileNameNext)
	{
		hr = S_OK;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = OnRender();
	}

	if (SUCCEEDED(hr))
	{
		SetTitleBarText(); // ignore return value
	}

	return hr;
}

HRESULT Direct2DRenderer::OnPrevious()
{
	HRESULT hr = S_OK;

	WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE);
	//TerminateThread(hThreadCacheFileNameNext, 0);
	WaitForSingleObject(hThreadCacheFileNameNext, INFINITE); // cludge

	FileNamePositionNext = g_FileNamePosition;
	g_FileNamePosition = FileNamePositionPrevious;

	for (UINT i = 0U; i < m_ImageNext.Frames; i++)
	{
		SafeRelease(&m_ImageNext.aFrameInfo[i].pBitmap);
	}

	delete [] m_ImageNext.aFrameInfo;

	//delete [] m_ImageNext.Title;
	//m_ImageNext.Title = nullptr;

	m_ImageNext = m_ImageCurrent;
	m_ImageCurrent = m_ImagePrevious;

	ResetRenderingParameters();

	hThreadCacheFileNamePrevious = (HANDLE)_beginthreadex( // NATIVE CODE
		NULL, // void *security,
		sizeof(LPVOID), // unsigned stack_size,
		StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * ),
		this, // void *arglist,
		0U, // unsigned initflag,
		NULL // unsigned *thrdaddr
		);

	if (hThreadCacheFileNamePrevious)
	{
		hr = S_OK;
	}
	else
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = OnRender();
	}

	if (SUCCEEDED(hr))
	{
		SetTitleBarText(); // ignore return value
	}

	return hr;
}

HRESULT Direct2DRenderer::SetTitleBarText()
{
	HRESULT hr = E_FAIL;

	LPWSTR TitleBarText = nullptr;
	short FileTitleLength = 0;

	if (SUCCEEDED(m_ImageCurrent.LoadResult) && m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title)
	{
		FileTitleLength = static_cast<short>(wcsnlen(m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title, SHRT_MAX) + 1);
	}
	else
	{
		FileTitleLength = GetFileTitleW(g_Files[g_FileNamePosition].FullPath, NULL, 0);
		if (FileTitleLength < 0)
		{
			return E_FAIL;
		}
	}

	if (SUCCEEDED(m_ImageCurrent.LoadResult) && m_ImageCurrent.Frames > 1U && m_ImageCurrent.guidContainerFormat != GUID_ContainerFormatGif)
	{
		LPWSTR FramePart = nullptr;
		UINT FramePartLength = 8/*space+bracket+"Frame"+space*/ + NumberOfDigits(m_FrameCurrent+1) + 1/*slash*/ + NumberOfDigits(m_ImageCurrent.Frames) + 2/*bracket+null char*/;

		FramePart = new WCHAR[FramePartLength];
		wmemset(FramePart, 0, FramePartLength);

		StringCchPrintfW(FramePart, FramePartLength, L" (Frame %d/%d)", m_FrameCurrent+1, m_ImageCurrent.Frames);

		TitleBarText = new WCHAR[FileTitleLength + FramePartLength];
		wmemset(TitleBarText, 0, (FileTitleLength + FramePartLength));

		if (m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title)
		{
			StringCchCopyW(TitleBarText, FileTitleLength, m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title);
		}
		else
		{
			GetFileTitleW(g_Files[g_FileNamePosition].FullPath, TitleBarText, (WORD)FileTitleLength);
		}

		StringCchCatW(TitleBarText, FileTitleLength + FramePartLength, FramePart);

		hr = S_OK;
		
		delete [] FramePart;
	}
	else
	{
		TitleBarText = new WCHAR[FileTitleLength];

		if (SUCCEEDED(m_ImageCurrent.LoadResult) && m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title)
		{
			StringCchCopyW(TitleBarText, FileTitleLength, m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title);
		}
		else
		{
			GetFileTitleW(g_Files[g_FileNamePosition].FullPath, TitleBarText, (WORD)FileTitleLength);
		}

		hr = S_OK;
	}

	if (SUCCEEDED(hr))
	{
		hr = SetWindowTextW(m_hWnd, TitleBarText) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	delete [] TitleBarText;

	return hr;
}

HRESULT Direct2DRenderer::EnumerateDecoders(COMDLG_FILTERSPEC **paFilterSpec, UINT *cFileTypes)
{
	return EnumerateDecoders(m_pWICFactory, paFilterSpec, cFileTypes);
}

HRESULT Direct2DRenderer::EnumerateDecoders(IWICImagingFactory *pIWICFactory, COMDLG_FILTERSPEC **paFilterSpec, UINT *cFileTypes)
{
	IEnumUnknown *piEnumUnknown = nullptr;
	IUnknown *piUnknown = nullptr;
	IWICBitmapDecoderInfo *piBitmapDecoderInfo = nullptr;
	UINT NumberOfDecoders = 0U;
	ULONG num = 0L;

	HRESULT hr = pIWICFactory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateDefault | WICComponentEnumerateRefresh, &piEnumUnknown); // WICComponentEnumerateRefresh will makes sure registry settings read for new decoders
	if (SUCCEEDED(hr))
	{
		hr = piEnumUnknown->Reset();
	}
	
	if (SUCCEEDED(hr))
	{
		while ((SUCCEEDED(piEnumUnknown->Next(1L, &piUnknown, &num))) && (num == 1L))
		{
			NumberOfDecoders++;
		}
		
		*cFileTypes = NumberOfDecoders + 2U; // 1 for all found extensions and 1 for All files (*.*)

		if (paFilterSpec == nullptr) // function called this way to return number of decoders
		{
			return S_OK;
		}

		hr = piEnumUnknown->Reset();
	}
	
	if (SUCCEEDED(hr))
	{
		LPWSTR pszAllExtensions = nullptr; // not deleted
		pszAllExtensions = new WCHAR[1000]; // find the length properly, don't be lazy
		wmemset(pszAllExtensions, 0, 1000);

		UINT CurrentDecoder = 1U;

		while ((SUCCEEDED(piEnumUnknown->Next(1L, &piUnknown, &num))) && (num == 1L))
		{
			hr = piUnknown->QueryInterface(IID_PPV_ARGS(&piBitmapDecoderInfo));
			if (SUCCEEDED(hr))
			{
				GUID guidContainerFormat = GUID_NULL;
				bool HasEncoder = false;

				hr = piBitmapDecoderInfo->GetContainerFormat(&guidContainerFormat);
				if (SUCCEEDED(hr))
				{
					IWICBitmapEncoder *pEncoder = nullptr;

					if (SUCCEEDED(pIWICFactory->CreateEncoder(guidContainerFormat, NULL, &pEncoder)))
					{
						HasEncoder = true;
					}
					else
					{
						HasEncoder = false;
					}

					SafeRelease(&pEncoder);

					DecoderHasEncoder.insert(std::pair<GUID, bool>(guidContainerFormat, HasEncoder));
				}

				LPWSTR pszFriendlyName = nullptr;
				LPWSTR pszExtensions = nullptr;
				LPWSTR pszSpecTemp = nullptr;

				LPWSTR pszName = nullptr; // not deleted
				LPWSTR pszSpec = nullptr; // not deleted
				
				UINT uActual = 0U;

				if (SUCCEEDED(hr))
				{
					hr = piBitmapDecoderInfo->GetFriendlyName(0U, NULL, &uActual);
				}

				if (SUCCEEDED(hr))
				{
					if (uActual > 0U)
					{
						pszFriendlyName = new (std::nothrow) WCHAR[uActual];
						if (pszFriendlyName)
						{
							wmemset(pszFriendlyName, 0, uActual);
							hr = piBitmapDecoderInfo->GetFriendlyName(uActual, pszFriendlyName, &uActual);
						}

						pszName = new (std::nothrow) WCHAR[uActual - 8U]; // subtract 8 characters of " Decoder" ending
						if (pszName)
						{
							wmemset(pszName, 0, (uActual - 8U));
							StringCchCopyW(pszName, uActual - 8, pszFriendlyName); // this fails with less buffer error, check
						}
					}
				}

				if (SUCCEEDED(hr))
				{
					// Extension
					hr = piBitmapDecoderInfo->GetFileExtensions(0U, NULL, &uActual);
					if (uActual > 0U)
					{
						pszExtensions = new (std::nothrow) WCHAR[uActual];
						if (pszExtensions)
						{
							wmemset(pszExtensions, 0, uActual);
							hr = piBitmapDecoderInfo->GetFileExtensions(uActual, pszExtensions, &uActual);
						}

						pszSpecTemp = new (std::nothrow) WCHAR[uActual + CountOccurencesOfCharacterInString('.', pszExtensions)];
						if (pszSpecTemp)
						{
							ReplaceCharInString(pszExtensions, pszSpecTemp, '.', L"*.");
						}

						pszSpec= new (std::nothrow) WCHAR[uActual + CountOccurencesOfCharacterInString('.', pszExtensions)];
						if (pszSpec)
						{
							ReplaceCharInString(pszSpecTemp, pszSpec, ',', L";");
						}
					}
				}

				(*paFilterSpec)[CurrentDecoder].pszName = pszName;
				(*paFilterSpec)[CurrentDecoder].pszSpec = pszSpec;

				StringCchCatW(pszAllExtensions, 1000, pszSpec);
				if (CurrentDecoder != NumberOfDecoders)
				{
					StringCchCatW(pszAllExtensions, 1000, L";");
				}

				delete [] pszSpecTemp;
				delete [] pszFriendlyName;
				delete [] pszExtensions;
			}	
			CurrentDecoder++;
		}
		(*paFilterSpec)[0].pszName = L"All image files";
		(*paFilterSpec)[0].pszSpec = pszAllExtensions;

		(*paFilterSpec)[CurrentDecoder].pszName = L"All files";
		(*paFilterSpec)[CurrentDecoder].pszSpec = L"*.*";
	}
	SafeRelease(&piBitmapDecoderInfo);
	SafeRelease(&piUnknown);
	SafeRelease(&piEnumUnknown);

	return S_OK;
}

/******************************************************************
*                                                                 *
*  Direct2DRenderer::GIF_GetGlobalMetadata()                      *
*                                                                 *
*  Retrieves global metadata which pertains to the entire image.  *
*                                                                 *
******************************************************************/

inline HRESULT Direct2DRenderer::GIF_GetGlobalMetadata(IWICBitmapDecoder *pDecoder, IMAGE_INFO *ImageInfo)
{
	PROPVARIANT propValue = {0};
	PropVariantInit(&propValue);
	IWICMetadataQueryReader *pMetadataQueryReader = nullptr;
	
	// Create a MetadataQueryReader from the decoder
	HRESULT hr = pDecoder->GetMetadataQueryReader(&pMetadataQueryReader);

	// Get background color
	if (SUCCEEDED(hr))
    {
		hr = ConformGIF ? GIF_GetBackgroundColor(pDecoder, pMetadataQueryReader, &(ImageInfo->GifInfo.BackgroundColor)) : E_FAIL;
		if (FAILED(hr))
		{
			// Default to transparent if failed to get the color
			ImageInfo->GifInfo.BackgroundColor = D2D1::ColorF(0U, 0.0f);
			hr = S_OK;
		}
	}

    // Get global frame size
    if (SUCCEEDED(hr))
    {
        // Get width
        hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/Width", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
				ImageInfo->GifInfo.Size.width = propValue.uiVal;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Get height
        hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/Height", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
                ImageInfo->GifInfo.Size.height = propValue.uiVal;
            }
            PropVariantClear(&propValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Get pixel aspect ratio
        hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/PixelAspectRatio", &propValue);
        if (SUCCEEDED(hr))
        {
            hr = (propValue.vt == VT_UI1 ? S_OK : E_FAIL);
            if (SUCCEEDED(hr))
            {
				UINT uPixelAspRatio = propValue.bVal;

                if (uPixelAspRatio != 0U) // 1 sometimes incorrectly used by gif makers to mean 0 here
                {
                    // Need to calculate the ratio. The value in uPixelAspRatio 
                    // allows specifying widest pixel 4:1 to the tallest pixel of 
                    // 1:4 in increments of 1/64th
                    FLOAT pixelAspRatio = (static_cast<FLOAT>(uPixelAspRatio) + 15.0f) / 64.0f;

                    // Calculate the image width and height in pixel based on the
                    // pixel aspect ratio. Only shrink the image.
                    if (pixelAspRatio > 1.f)
                    {
						ImageInfo->GifInfo.m_cxGifImagePixel = ImageInfo->GifInfo.Size.width;
                        ImageInfo->GifInfo.m_cyGifImagePixel = static_cast<UINT>(ImageInfo->GifInfo.Size.height / pixelAspRatio);
                    }
                    else
                    {
                        ImageInfo->GifInfo.m_cxGifImagePixel = static_cast<UINT>(ImageInfo->GifInfo.Size.width * pixelAspRatio);
                        ImageInfo->GifInfo.m_cyGifImagePixel = ImageInfo->GifInfo.Size.height;
                    }
                }
                else
                {
                    // The value is 0 or 1, so its ratio is 1
                    ImageInfo->GifInfo.m_cxGifImagePixel = ImageInfo->GifInfo.Size.width;
                    ImageInfo->GifInfo.m_cyGifImagePixel = ImageInfo->GifInfo.Size.height;
                }
            }
            PropVariantClear(&propValue);
        }
    }

    // Get looping information
    if (SUCCEEDED(hr))
    {
        // First check to see if the application block in the Application Extension
        // contains "NETSCAPE2.0" and "ANIMEXTS1.0", which indicates the gif animation
        // has looping information associated with it.
        // 
        // If we fail to get the looping information, loop the animation infinitely.
        if (SUCCEEDED(pMetadataQueryReader->GetMetadataByName(
                    L"/appext/application", 
                    &propValue)) &&
            propValue.vt == (VT_UI1 | VT_VECTOR) &&
            propValue.caub.cElems == 11 &&  // Length of the application block
            (!memcmp(propValue.caub.pElems, "NETSCAPE2.0", propValue.caub.cElems) ||
             !memcmp(propValue.caub.pElems, "ANIMEXTS1.0", propValue.caub.cElems)))
        {
            PropVariantClear(&propValue);

            hr = pMetadataQueryReader->GetMetadataByName(L"/appext/data", &propValue);
            if (SUCCEEDED(hr))
            {
                //  The data is in the following format:
                //  byte 0: extsize (must be > 1)
                //  byte 1: loopType (1 == animated gif)
                //  byte 2: loop count (least significant byte)
                //  byte 3: loop count (most significant byte)
                //  byte 4: set to zero
                if (propValue.vt == (VT_UI1 | VT_VECTOR) &&
                    propValue.caub.cElems >= 4 &&
                    propValue.caub.pElems[0] > 0 &&
                    propValue.caub.pElems[1] == 1)
                {
					ImageInfo->GifInfo.m_uTotalLoopCount = MAKEWORD(propValue.caub.pElems[2], propValue.caub.pElems[3]);
                    
                    // If the total loop count is not zero, we then have a loop count
                    // If it is 0, then we repeat infinitely
                    /*if (ImageInfo->GifInfo.m_uTotalLoopCount != 0) 
                    {
                        ImageInfo->GifInfo.m_fHasLoop = true;
                    }*/
                }
            }
			PropVariantClear(&propValue);
        }
    }

	// Get Comment extension
	//if (SUCCEEDED(hr))
	//{
	//	hr = pMetadataQueryReader->GetMetadataByName(L"/commentext/TextEntry", &propValue);
	//	if (SUCCEEDED(hr))
	//	{
	//		for (UINT i = 0U; i < ImageInfo->Frames; i++)
	//		{
	//			if (ImageInfo->aFrameInfo[i].Title != nullptr)
	//			{
	//				delete [] ImageInfo->aFrameInfo[i].Title;
	//			}

	//			ImageInfo->aFrameInfo[i].Title = new WCHAR[USHRT_MAX];
	//			wmemset(ImageInfo->aFrameInfo[i].Title, 0, USHRT_MAX);

	//			hr = PropVariantToString(propValue, ImageInfo->aFrameInfo[i].Title, USHRT_MAX);
	//			if (FAILED(hr))
	//			{
	//				delete [] ImageInfo->aFrameInfo[i].Title;
	//				ImageInfo->aFrameInfo[i].Title = nullptr;
	//				hr = S_OK;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		for (UINT i = 0U; i < ImageInfo->Frames; i++)
	//		{
	//			if (ImageInfo->aFrameInfo[i].Title != nullptr)
	//			{
	//				delete [] ImageInfo->aFrameInfo[i].Title;
	//			}

	//			ImageInfo->aFrameInfo[i].Title = nullptr;
	//		}
	//		hr = S_OK;
	//	}
	//}

    PropVariantClear(&propValue);
    SafeRelease(&pMetadataQueryReader);

    return hr;
}

/******************************************************************
*                                                                 *
*  Direct2DRenderer::GIF_GetBackgroundColor()                     *
*                                                                 *
*  Reads and stores the background color for gif.                 *
*                                                                 *
******************************************************************/

inline HRESULT Direct2DRenderer::GIF_GetBackgroundColor(IWICBitmapDecoder *pDecoder, IWICMetadataQueryReader *pMetadataQueryReader, D2D1_COLOR_F *BackgroundColor)
{
	IWICPalette *pWicPalette = nullptr;
    DWORD dwBGColor = 0;
    BYTE backgroundIndex = 0;
	UINT GlobalColorTableSize = 0U;
	WICColor rgColors[256] = {0};
    UINT cColorsCopied = 0U;
	PROPVARIANT propVariant = {0};
    PropVariantInit(&propVariant);

    // If we have a global palette, get the palette and background color
    HRESULT hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/GlobalColorTableFlag", &propVariant);
    if (SUCCEEDED(hr))
    {
        hr = (propVariant.vt != VT_BOOL || !propVariant.boolVal) ? E_FAIL : S_OK;
        PropVariantClear(&propVariant);
    }

	if (SUCCEEDED(hr))
    {
		hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/GlobalColorTableSize", &propVariant);
		if (SUCCEEDED(hr))
        {
			hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
            if (SUCCEEDED(hr))
            {
				GlobalColorTableSize = static_cast<UINT>(pow(2.0f, propVariant.bVal + 1));
			}
			PropVariantClear(&propVariant);
		}

		if (FAILED(hr)) // fallback to 256
		{
			GlobalColorTableSize = ARRAYSIZE(rgColors);
			hr = S_OK;
		}
	}

    if (SUCCEEDED(hr))
    {
        // Background color index
        hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/BackgroundColorIndex", &propVariant);
        if (SUCCEEDED(hr))
        {
            hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
            if (SUCCEEDED(hr))
            {
                backgroundIndex = propVariant.bVal;
            }
            PropVariantClear(&propVariant);
        }
    }

    // Get the color from the palette
    if (SUCCEEDED(hr))
    {
        hr = m_pWICFactory->CreatePalette(&pWicPalette);
    }

    if (SUCCEEDED(hr))
    {
        // Get the global palette
        hr = pDecoder->CopyPalette(pWicPalette);
    }

    if (SUCCEEDED(hr))
    {
        hr = pWicPalette->GetColors(GlobalColorTableSize, rgColors, &cColorsCopied);
    }

    if (SUCCEEDED(hr))
    {
        // Check whether background color is outside range 
        hr = (backgroundIndex >= GlobalColorTableSize) ? E_FAIL : S_OK;
    }

    if (SUCCEEDED(hr))
    {
        // Get the color in ARGB format
        dwBGColor = rgColors[backgroundIndex];

        // The background color is in ARGB format, and we want to 
        // extract the alpha value and convert it in FLOAT
        FLOAT alpha = (dwBGColor >> 24) / 255.0F;
        *BackgroundColor = D2D1::ColorF(dwBGColor, alpha);
    }

    SafeRelease(&pWicPalette);
    return hr;
}

HRESULT Invert()
{
	IWICImagingFactory *pFactory = NULL;
    IWICBitmap *pBitmap = NULL;

    UINT uiWidth = 640;
    UINT uiHeight = 480;
    WICPixelFormatGUID formatGUID = GUID_WICPixelFormat32bppPBGRA;

    WICRect rcLock = { 0, 0, uiWidth, uiHeight };
    IWICBitmapLock *pLock = NULL;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        (LPVOID*)&pFactory
        );

    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateBitmap(uiWidth, uiHeight, formatGUID, WICBitmapCacheOnDemand, &pBitmap);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

        if (SUCCEEDED(hr))
        {
            UINT cbBufferSize = 0;
            UINT cbStride = 0;
            BYTE *pv = NULL;

            // Retrieve the stride.
            hr = pLock->GetStride(&cbStride);

            if (SUCCEEDED(hr))
            {
                hr = pLock->GetDataPointer(&cbBufferSize, &pv);
            }
            
            // Zero out memory pointed to by the lock.
            ZeroMemory(pv, cbBufferSize);

            // Release the bitmap lock.
            pLock->Release();
        }
    }

    if (pBitmap)
    {
        pBitmap->Release();
    }

    if (pFactory)
    {
        pFactory->Release();
    }

    return hr;
}

//D2D1_MATRIX_3X2_F transform;
	//m_pRenderTarget->GetTransform(&transform);
	//D2D1::Matrix3x2F transform2;
	//transform2.ReinterpretBaseType(&transform);
	//m_ZoomCentre = transform2.TransformPoint(D2D1::Point2F((FLOAT)x, (FLOAT)y));

/*wchar_t buffer[260];
HRESULT hr = StringCchPrintfW(buffer, 260, L"m_zoom: %f m_zoomMax: %f Clicked: (%f, %f)", m_zoom, m_zoomMax, m_ZoomCentre.x, m_ZoomCentre.y);
if SUCCEEDED(hr)
{
	SetWindowTextW(m_hWnd, buffer);
}*/

	/*wchar_t buffer[260];
HRESULT hr = StringCchPrintfW(buffer, 1000, L"RenderTargetSize: (%f, %f)", (FLOAT)UINT(RenderTargetSize.width), (FLOAT)UINT(RenderTargetSize.height));
if SUCCEEDED(hr)
{
	SetWindowTextW(m_hWnd, buffer);
}*/
//wchar_t buffer[260];
//HRESULT hr = StringCchPrintfW(buffer, 1000, L"RenderTargetSize: (%.2f, %.2f)\nm_BitmapCentre: (%.2f, %.2f)\nm_BitmapTranslatePoint: (%.2f, %.2f)", RenderTargetSize.width, RenderTargetSize.height, m_BitmapSizeFitToWindow.width/2.0f, m_BitmapSizeFitToWindow.height/2.0f, m_BitmapTranslatePoint.x, m_BitmapTranslatePoint.y);
//if SUCCEEDED(hr)
//{
//	SetWindowTextW(m_hWnd, buffer);
//}

//m_BitmapCentreAfterRotate = D2D1::Point2F(m_BitmapSize.width/2, m_BitmapSize.height/2);

//switch (*pBitmapRotationFlag)
//{
//case 1:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F::Identity();
//	}
//	break;
//case 2:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//		m_BitmapCentreAfterRotate = m_BitmapCentreAfterRotate * D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//	}
//	break;
//case 3:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F::Rotation(180, m_BitmapCentreAfterRotate);
//	}
//	break;
//case 4:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//		m_BitmapCentreAfterRotate = m_BitmapCentreAfterRotate * D2D1::Matrix3x2F(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
//	}
//	break;
//case 5:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//		m_BitmapCentreAfterRotate = m_BitmapCentreAfterRotate * D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//		m_TransformMatrixRotation = m_TransformMatrixRotation * D2D1::Matrix3x2F::Rotation(-90, m_BitmapCentreAfterRotate);
//	}
//	break;
//case 6:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F::Rotation(90, m_BitmapCentreAfterRotate);
//	}
//	break;
//case 7:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//		m_BitmapCentreAfterRotate = m_BitmapCentreAfterRotate * D2D1::Matrix3x2F(-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
//		m_TransformMatrixRotation = m_TransformMatrixRotation * D2D1::Matrix3x2F::Rotation(-270, m_BitmapCentreAfterRotate);
//	}
//	break;
//case 8:
//	{
//		m_TransformMatrixRotation = D2D1::Matrix3x2F::Rotation(270, m_BitmapCentreAfterRotate);
//	}
//	break;
//}

	/*	1        2       3      4       5           6           7           8

		888888  888888      88  88      8888888888  88                  88  8888888888
		88          88      88  88      88  88      88  88          88  88      88  88
		8888      8888    8888  8888    88          8888888888  8888888888          88
		88          88      88  88
		88          88  888888  888888
	
	USHORT RotationFlagTemp = 0U;

	switch (m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag)
	{
	case 1U:
		{
			RotationFlagTemp = Clockwise? 6U : 8U;
		}
		break;
	case 2U:
		{
			RotationFlagTemp = Clockwise? 7U : 5U;
		}
		break;
	case 3U:
		{
			RotationFlagTemp = Clockwise? 8U : 6U;
		}
		break;
	case 4U:
		{
			RotationFlagTemp = Clockwise? 5U : 7U;
		}
		break;
	case 5U:
		{
			RotationFlagTemp = Clockwise? 2U : 4U;
		}
		break;
	case 6U:
		{
			RotationFlagTemp = Clockwise? 3U : 1U;
		}
		break;
	case 7U:
		{
			RotationFlagTemp = Clockwise? 4U : 2U;
		}
		break;
	case 8U:
		{
			RotationFlagTemp = Clockwise? 1U : 3U;
		}
		break;
	}

	HRESULT hr = Rotate(m_pWICFactory, g_Files[g_FileNamePosition].FullPath, &RotationFlagTemp, Clockwise);
	if (FAILED(hr))
	{
		hr = RotateByReencode(m_pWICFactory, g_Files[g_FileNamePosition].FullPath, &RotationFlagTemp, Clockwise);
		if (SUCCEEDED(hr)) // if re-encoded, then the new Rotation Flag is 1
		{
			RotationFlagTemp = 1U;
		}
	}

	if (SUCCEEDED(hr)) // if succeeded in rotating, commit change to the rotation flag
	{
		m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag = RotationFlagTemp;
	}*/