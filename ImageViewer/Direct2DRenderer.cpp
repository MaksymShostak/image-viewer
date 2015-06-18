#include "stdafx.h"
#include "Direct2DRenderer.h"
#include "HRESULT.h"

unsigned int ChangeOrientation(const wchar_t* FilenameIn, const wchar_t* FilenameOut, unsigned short OrientationFlag, unsigned char Clockwise, unsigned char Trim)
{
	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jsrcerr, jdsterr;

	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;

	/* We assume all-in-memory processing and can therefore use only a
	* single file pointer for sequential input and output operation.
	*/
	FILE * fp;

	jpeg_transform_info transformoption;
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;

	switch (OrientationFlag)
	{
	case 1U:
	{
		transformoption.transform = (Clockwise == 1) ? JXFORM_ROT_90 : JXFORM_ROT_270;
	}
	break;
	case 2U:
	{
		transformoption.transform = JXFORM_FLIP_H;
	}
	break;
	case 3U:
	{
		transformoption.transform = JXFORM_ROT_180;
	}
	break;
	case 4U:
	{
		transformoption.transform = JXFORM_FLIP_V;
	}
	break;
	case 5U:
	{
		transformoption.transform = JXFORM_TRANSPOSE;
	}
	break;
	case 6U:
	{
		transformoption.transform = JXFORM_ROT_90;
	}
	break;
	case 7U:
	{
		transformoption.transform = JXFORM_TRANSVERSE;
	}
	break;
	case 8U:
	{
		transformoption.transform = JXFORM_ROT_270;
	}
	break;
	default:
	{
		transformoption.transform = JXFORM_NONE;
	}
	break;
	}

	transformoption.perfect = (Trim == 1) ? FALSE : TRUE;
	transformoption.trim = (Trim == 1) ? TRUE : FALSE;
	transformoption.force_grayscale = FALSE;
	transformoption.crop = FALSE;

	// Initialize the JPEG decompression object with default error handling
	srcinfo.err = jpeg_std_error(&jsrcerr);
	jpeg_create_decompress(&srcinfo);
	// Initialize the JPEG compression object with default error handling
	dstinfo.err = jpeg_std_error(&jdsterr);
	jpeg_create_compress(&dstinfo);

	// Note: we assume only the decompression object will have virtual arrays

	dstinfo.optimize_coding = TRUE;
	dstinfo.err->trace_level = 0;
	//srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

	// Open input file
	if (_wfopen_s(&fp, FilenameIn, L"rb") != 0)
	{
		return ERROR_OPEN_FAILED;
	}

	// Specify data source for decompression
	jpeg_stdio_src(&srcinfo, fp);

	// Enable saving of extra markers that we want to copy
	jcopy_markers_setup(&srcinfo, copyoption);

	// Read file header
	(void)jpeg_read_header(&srcinfo, TRUE);

	if (!jtransform_request_workspace(&srcinfo, &transformoption))
	{
		fclose(fp);
		return ERROR_INVALID_DATA;
	}

	// Read source file as DCT coefficients
	src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	// Initialize destination compression parameters from source values
	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

	/* Adjust destination parameters if required by transform options;
	* also find out which set of coefficient arrays will hold the output.
	*/
	dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

	// Close input file
	/* Note: we assume that jpeg_read_coefficients consumed all input
	* until JPEG_REACHED_EOI, and that jpeg_finish_decompress will
	* only consume more while (! cinfo->inputctl->eoi_reached).
	* We cannot call jpeg_finish_decompress here since we still need the
	* virtual arrays allocated from the source object for processing.
	*/
	fclose(fp);

	// Open the output file
	if (_wfopen_s(&fp, FilenameOut, L"wb") != 0)
	{
		return ERROR_CANNOT_MAKE;
	}

	// Set progressive mode (saves space but is slower)
	jpeg_simple_progression(&dstinfo);

	// Specify data destination for compression
	jpeg_stdio_dest(&dstinfo, fp);

	// Start compressor (note no image data is actually written here)
	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

	// Copy to the output file any extra markers that we want to preserve
	jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

	// Execute image transformation, if any
	jtransform_execute_transformation(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

	// Finish compression and release memory
	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void)jpeg_finish_decompress(&srcinfo);
	jpeg_destroy_decompress(&srcinfo);

	// Close output file
	fclose(fp);

	return 0;
}

bool operator<(const GUID & Left, const GUID & Right)
{
	WCHAR GUIDLeft[GUIDSTRING_MAX] = {0};
	WCHAR GUIDRight[GUIDSTRING_MAX] = {0};

	(void)StringFromGUID2(Left, GUIDLeft, GUIDSTRING_MAX);
	(void)StringFromGUID2(Right, GUIDRight, GUIDSTRING_MAX);

    return (StrCmpW(GUIDLeft, GUIDRight) < 0);
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
	if (nullptr == String || 0U == StringMaxLength || 0U == NumberOfCharsToTrim)
	{
		return;
	}

	UINT StringLength = static_cast<UINT>(wcsnlen(String, StringMaxLength) + 1);

	for (UINT i = StringLength - NumberOfCharsToTrim; i < StringLength; i++)
	{
		String[i] = L'\0';
	}
}

unsigned int WINAPI StaticCacheFileNameNext(void* Param)
{
	return (reinterpret_cast<Direct2DRenderer*>(Param))->CacheFileNameNext(g_FileNamePosition);
}

unsigned int WINAPI StaticCacheFileNamePrevious(void* Param)
{
	return (reinterpret_cast<Direct2DRenderer*>(Param))->CacheFileNamePrevious(g_FileNamePosition);
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

//Release resources
/*Direct2DRenderer::~Direct2DRenderer()
{
	CloseHandle(hThreadCacheFileNamePrevious);
	CloseHandle(hThreadCacheFileNameNext);

	for (UINT i = 0U; i < m_ImagePrevious.aFrameInfo.size(); i++)
	{
		SafeRelease(m_ImagePrevious.aFrameInfo[i].pBitmap.GetAddressOf());
	}

	for (UINT i = 0U; i < m_ImageCurrent.aFrameInfo.size(); i++)
	{
		SafeRelease(m_ImageCurrent.aFrameInfo[i].pBitmap.GetAddressOf());
	}

	for (UINT i = 0U; i < m_ImageNext.aFrameInfo.size(); i++)
	{
		SafeRelease(m_ImageNext.aFrameInfo[i].pBitmap.GetAddressOf());
	}

	SafeRelease(m_pDWriteFactory.GetAddressOf());
	SafeRelease(m_pRenderTarget.GetAddressOf());
	SafeRelease(m_pTextFormat.GetAddressOf());
	SafeRelease(m_pBlackBrush.GetAddressOf());
	SafeRelease(m_pWhiteBrush.GetAddressOf());
	SafeRelease(m_pD2DFactory.GetAddressOf());
	//SafeRelease(&m_pContextDst); // Already destroyed by CoUninitialize() call that destroys m_pWICFactory that created this
	//SafeRelease(&m_pWICFactory); // Already destroyed by CoUninitialize() call
}*/

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

    if (SUCCEEDED(hr))
	{
		if (DeviceResourcesDiscarded)
		{
			//LoadBitmapCurrent(g_Files[g_FileNamePosition].FullPath.c_str());
			m_ImageCurrent.aFrameInfo[m_FrameCurrent].pID2D1Bitmap1 = nullptr;
			DeviceResourcesDiscarded = false; // only reload once
		}

		// If the device bitmap has not yet been created
		if (!m_ImageCurrent.aFrameInfo[m_FrameCurrent].pID2D1Bitmap1.Get())
		{
			// Create a Direct2D bitmap from the WIC bitmap
			hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
				m_ImageCurrent.aFrameInfo[m_FrameCurrent].pIWICBitmapSource.Get(),
				NULL,
				&m_ImageCurrent.aFrameInfo[m_FrameCurrent].pID2D1Bitmap1
				);
			if (FAILED(hr)) { return hr; }

			//m_ImageCurrent.aFrameInfo[m_FrameCurrent].pIWICBitmapSource = nullptr;
		}

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

				if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatGif && m_ImageCurrent.aFrameInfo.size() > 1U) // CLUDGE, fix with proper function for FitToWindowSize (use CalculateDrawRectangle for motivation)
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

				if ((_Direct2DRenderTargetSize.width >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (_Direct2DRenderTargetSize.height >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is larger than image
				{OutputDebugStringW(L"m_bFitToWindow: window is larger than image\n");
					m_BitmapSizeFitToWindow = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size;
					m_zoomMax = 20.0f;
				}
				else if ((_Direct2DRenderTargetSize.width >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (_Direct2DRenderTargetSize.height < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is shorter than image
				{OutputDebugStringW(L"m_bFitToWindow: window is shorter than image\n");
					ScaleFactor = _Direct2DRenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
					m_zoomMax = 20.0f/ScaleFactor;
				}
				else if ((_Direct2DRenderTargetSize.width < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (_Direct2DRenderTargetSize.height >= m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is thinner than image
				{OutputDebugStringW(L"m_bFitToWindow: window is thinner than image\n");
					ScaleFactor = _Direct2DRenderTargetSize.width/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
					m_zoomMax = 20.0f/ScaleFactor;
				}
				else if ((_Direct2DRenderTargetSize.width < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) && (_Direct2DRenderTargetSize.height < m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height)) // if window is smaller than image
				{OutputDebugStringW(L"m_bFitToWindow: window is smaller than image\n");
					if (((m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width - _Direct2DRenderTargetSize.width)/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width) < ((m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height - _Direct2DRenderTargetSize.height)/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height))
					{OutputDebugStringW(L"m_bFitToWindow: height constrained\n");
						ScaleFactor = _Direct2DRenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
						m_zoomMax = 20.0f/ScaleFactor;
					}
					else
					{OutputDebugStringW(L"m_bFitToWindow: width constrained\n");
						ScaleFactor = _Direct2DRenderTargetSize.width/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
						m_zoomMax = 20.0f/ScaleFactor;
					}
				}
			
				m_BitmapSizeFitToWindow.width = ScaleFactor * m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width;
				m_BitmapSizeFitToWindow.height = ScaleFactor * m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;

				CalculateBitmapTranslatePoint(_Direct2DRenderTargetSize);

				Pannable = false;
			}
			else if (m_ScaleToWindow)
			{
				m_TransformMatrixScale = D2D1::Matrix3x2F::Identity();
				m_zoom = (FLOAT)_Direct2DRenderTargetSize.height/m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height;
				m_zoomMax = 20.0f;

				m_BitmapSizeFitToWindow.width = (FLOAT)_Direct2DRenderTargetSize.height * (m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.width)/(m_ImageCurrent.aFrameInfo[m_FrameCurrent].Size.height);
				m_BitmapSizeFitToWindow.height = (FLOAT)_Direct2DRenderTargetSize.height;

				CalculateBitmapTranslatePoint(_Direct2DRenderTargetSize);

				Pannable = false;
			}
			else // if not fit to window
			{
				if ((_Direct2DRenderTargetSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
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

			m_pRenderTarget->DrawBitmap
			(
				m_ImageCurrent.aFrameInfo[m_FrameCurrent].pID2D1Bitmap1.Get(),
				D2D1::RectF
				(
					0.0f,
					0.0f,
					(96.0f/m_dpiX)*m_BitmapSizeFitToWindow.width,
					(96.0f/m_dpiX)*m_BitmapSizeFitToWindow.height
				),
				1.0F,
				D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC
			);
			
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
				m_pTextFormat.Get(),
				D2D1::RectF(0.0f, 0.0f, (96.0f/m_dpiX)*static_cast<FLOAT>(_Direct2DRenderTargetSize.width), (96.0f/m_dpiX)*static_cast<FLOAT>(_Direct2DRenderTargetSize.height)),
				BackgroundColorBlack ? m_pWhiteBrush.Get() : m_pBlackBrush.Get()
				);
		}

        hr = m_pRenderTarget->EndDraw();

		if (SUCCEEDED(hr))
		{
			// Present (new for Direct2D 1.1)
			DXGI_PRESENT_PARAMETERS parameters = { 0 };
			parameters.DirtyRectsCount = 0U;
			parameters.pDirtyRects = nullptr;
			parameters.pScrollRect = nullptr;
			parameters.pScrollOffset = nullptr;

			hr = _pIDXGISwapChain1->Present1(1U, 0U, &parameters);
		}

		// If any part of the drawing failed
		if (FAILED(hr))
		{
			// If the reason for the error is not the drawing surface is occluded
			// e.g. D2DERR_RECREATE_TARGET
			if (DXGI_STATUS_OCCLUDED != hr)
			{
				DiscardDeviceResources();
			}

			hr = S_OK;
		}
    }

	if (SUCCEEDED(hr))
	{
		if (GUID_ContainerFormatGif == m_ImageCurrent.guidContainerFormat)
		{
			if (m_ImageCurrent.aFrameInfo.size() > 1U && 0U == m_uLoopNumber && 0U == m_FrameCurrent)
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
		if (GUID_ContainerFormatJpeg == m_ImageCurrent.guidContainerFormat) // can always rotate JPEG
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
	WCHAR FileNameUnicode[PATHCCH_MAX_CCH] = L"\\\\?\\";
	WCHAR FileNameTemporary[PATHCCH_MAX_CCH] = {0};
	WIN32_FILE_ATTRIBUTE_DATA FileAttributeDataOriginal = {0};
	HANDLE HandleNew = nullptr;

	HRESULT hr = StringCchCatW(FileNameUnicode, PATHCCH_MAX_CCH, g_Files[g_FileNamePosition].FullPath.c_str());

	if (SUCCEEDED(hr))
	{
		hr = GetFileAttributesExW(FileNameUnicode, GetFileExInfoStandard, &FileAttributeDataOriginal) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (SUCCEEDED(hr))
	{
		hr = StringCchCopyW(FileNameTemporary, PATHCCH_MAX_CCH, g_Files[g_FileNamePosition].FullPath.c_str());
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, PATHCCH_MAX_CCH, L"temp");
	}

	if (SUCCEEDED(hr))
	{
		if (m_ImageCurrent.guidContainerFormat == GUID_ContainerFormatJpeg)
		{
			hr = RotateJPEG(g_Files[g_FileNamePosition].FullPath.c_str(), FileNameTemporary, m_ImageCurrent.aFrameInfo[m_FrameCurrent].RotationFlag, Clockwise);
		}
		else
		{
			hr = RotateByReencode(m_pWICFactory.Get(), g_Files[g_FileNamePosition].FullPath.c_str(), FileNameTemporary, Clockwise);
			if (FAILED(hr) && hr != WINCODEC_ERR_ABORTED) // delete the temporary file we created in case the above function fails
			{
				if (SUCCEEDED(StringCchCatW(FileNameUnicode, PATHCCH_MAX_CCH, L"temp")))
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
		hr = StringCchCopyW(FileNameTemporary, PATHCCH_MAX_CCH, L"\\\\?\\");
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, PATHCCH_MAX_CCH, g_Files[g_FileNamePosition].FullPath.c_str());
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchCatW(FileNameTemporary, PATHCCH_MAX_CCH, L"temp");
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
		LoadBitmapFromFile(m_pWICFactory.Get(), g_Files[g_FileNamePosition].FullPath.c_str(), m_pContextDst.Get(), &m_ImageCurrent);

		ResetRenderingParameters();

		hr = OnRender();
	}

	return hr;
}

HRESULT Direct2DRenderer::RotateByReencode(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, LPCWSTR FileNameTemporary, bool Clockwise)
{
	Microsoft::WRL::ComPtr<IWICBitmapDecoder> pDecoder;
	Microsoft::WRL::ComPtr<IWICBitmapDecoderInfo> pWICBitmapDecoderInfo;
	Microsoft::WRL::ComPtr<IWICStream> piFileStream;
	Microsoft::WRL::ComPtr<IWICBitmapEncoder> piEncoder;
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
		hr = piEncoder->Initialize(piFileStream.Get(), WICBitmapEncoderNoCache);
	}

	//Process each frame of the image.
	for (UINT i = 0U; i < m_ImageCurrent.aFrameInfo.size() && SUCCEEDED(hr); i++)
	{
		//Frame variables.
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> piFrameDecode;
		Microsoft::WRL::ComPtr<IWICBitmap> pBitmap;
		Microsoft::WRL::ComPtr<IWICBitmapFlipRotator> pFlipRotator;
		Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> piFrameEncode;
		IPropertyBag2 *pIEncoderOptions = nullptr;
		Microsoft::WRL::ComPtr<IWICMetadataQueryWriter> piFrameQWriter;
		Microsoft::WRL::ComPtr<IWICMetadataBlockWriter> piBlockWriter;
		Microsoft::WRL::ComPtr<IWICMetadataBlockReader> piBlockReader;
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
			hr = pIWICFactory->CreateBitmapFromSource(piFrameDecode.Get(), WICBitmapCacheOnDemand, &pBitmap);
		}

		if (SUCCEEDED(hr))
		{
			hr = pIWICFactory->CreateBitmapFlipRotator(&pFlipRotator);
		}

		if (SUCCEEDED(hr))
		{
			//if (i == m_FrameCurrent)
			//{
				hr = pFlipRotator->Initialize(pBitmap.Get(), Clockwise ? WICBitmapTransformRotate90 : WICBitmapTransformRotate270);
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
			hr = piFrameDecode.As(&piBlockReader);
			
			if (hr == E_NOINTERFACE) //Some data formats do not have metadata e.g. Bitmap
			{
				hr = S_OK;
			}
			else if (SUCCEEDED(hr))
			{
				hr = piFrameEncode.As(&piBlockWriter);

				if (SUCCEEDED(hr))
				{
					piBlockWriter->InitializeFromBlockReader(piBlockReader.Get()); // ignore return value
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
				static_cast<IWICBitmapSource*> (pFlipRotator.Get()),
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

HRESULT Direct2DRenderer::RotateByMetadata(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, USHORT *pRotationFlag,	bool /*Clockwise*/)
{
	Microsoft::WRL::ComPtr<IWICBitmapDecoder> pDecoder;
	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> pSource;
	Microsoft::WRL::ComPtr<IWICFastMetadataEncoder> pFME;
	Microsoft::WRL::ComPtr<IWICMetadataQueryWriter> pFMEQW;
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
		hr = pIWICFactory->CreateFastMetadataEncoderFromFrameDecode(pSource.Get(), &pFME);
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

	SafeRelease(pDecoder.GetAddressOf());
    SafeRelease(pSource.GetAddressOf());
	SafeRelease(pFME.GetAddressOf());
	SafeRelease(pFMEQW.GetAddressOf());
	PropVariantClear(&propvariantOrientationFlag);

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pFME->Commit\n");
		LoadBitmapFromFile(m_pWICFactory.Get(), FileName, m_pContextDst.Get(), &m_ImageCurrent);

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

unsigned int Direct2DRenderer::CacheFileNameNext(size_t FileNamePositionToWorkFrom)
{
	if ((FileNamePositionToWorkFrom + 1U) < g_Files.size())
	{
		g_FileNamePositionNext = FileNamePositionToWorkFrom + 1U;
	}
	else
	{
		g_FileNamePositionNext = 0U;
	}

	HRESULT hr = LoadBitmapFromFile(m_pWICFactory.Get(), g_Files[g_FileNamePositionNext].FullPath.c_str(), m_pContextDst.Get(), &m_ImageNext);

	if (FAILED(hr))
	{
		if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
		{
			return CacheFileNameNext(((FileNamePositionToWorkFrom + 1U) < g_Files.size()) ? FileNamePositionToWorkFrom + 1U : 0U);
		}
		else
		{
			return (unsigned)-hr;
		}
	}

	return hr;
}

unsigned int Direct2DRenderer::CacheFileNamePrevious(size_t FileNamePositionToWorkFrom)
{
	if (0U != FileNamePositionToWorkFrom)
	{
		g_FileNamePositionPrevious = FileNamePositionToWorkFrom - 1U;
	}
	else
	{
		g_FileNamePositionPrevious = g_Files.size() - 1U;
	}

	HRESULT hr = LoadBitmapFromFile(m_pWICFactory.Get(), g_Files[g_FileNamePositionPrevious].FullPath.c_str(), m_pContextDst.Get(), &m_ImagePrevious);

	if (FAILED(hr))
	{
		if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
		{
			return CacheFileNamePrevious((0U != FileNamePositionToWorkFrom)? FileNamePositionToWorkFrom - 1U : g_Files.size() - 1U);
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

	m_TransformMatrixTranslation = D2D1::Matrix3x2F::Translation((96.0F/m_dpiX)*m_BitmapTranslatePoint.x, (96.0F/m_dpiX)*m_BitmapTranslatePoint.y);
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
    static const FLOAT msc_fontSize = 12.0F;

    // Create a Direct2D factory.
	
    D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(DEBUG) || defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_MULTI_THREADED,
        options,
        m_pD2DFactory.GetAddressOf()
        );
	
    if (SUCCEEDED(hr))
    {OutputDebugStringW(L"D2D1CreateFactory\n");
        // Create WIC factory.
        hr = CoCreateInstance(
            CLSID_WICImagingFactory2,
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
            reinterpret_cast<IUnknown **>(m_pDWriteFactory.GetAddressOf())
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
        hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	if (SUCCEEDED(hr))
	{
		OutputDebugStringW(L"m_pTextFormat->SetParagraphAlignment\n");

		DWORD dwSize = PATHCCH_MAX_CCH;
		WCHAR ICMProfileName[PATHCCH_MAX_CCH] = { 0 };

		HDC hDC = GetDC(m_hWnd);

		if (hDC)
		{
			OutputDebugStringW(L"GetDC\n");
			if (GetICMProfileW(hDC, &dwSize, ICMProfileName))
			{
				OutputDebugStringW(L"GetICMProfileW\n");
				hr = m_pWICFactory->CreateColorContext(&m_pContextDst);

				if (SUCCEEDED(hr))
				{
					OutputDebugStringW(L"pIWICFactory->CreateColorContext(&pContextDst)\n");
					hr = m_pContextDst->InitializeFromFilename(ICMProfileName);
				}
			}
		}

		if (1 == ReleaseDC(m_hWnd, hDC))
		{
			OutputDebugStringW(L"ReleaseDC\n");
		}
	}

    return hr;
}

HRESULT CreateDeviceD3D11(
	__in D3D_DRIVER_TYPE const type,
	__inout Microsoft::WRL::ComPtr<ID3D11Device> & pID3D11Device,
	__inout Microsoft::WRL::ComPtr<ID3D11DeviceContext> & pID3D11DeviceContext,
	__inout_opt D3D_FEATURE_LEVEL * pD3D_FEATURE_LEVEL = nullptr
	)
{
	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// You need it for compatibility with Direct2D.
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the set of DirectX hardware feature levels this app supports.
	// The ordering is important and you should preserve it.
	// Don't forget to declare your app's minimum required feature level in its
	// description. All apps are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL supportedD3D_FEATURE_LEVELs[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL returnedD3D_FEATURE_LEVEL;

	HRESULT hr = D3D11CreateDevice(
		nullptr, // use the default adapter
		type,
		nullptr, // no software rasterizer
		flags,
		supportedD3D_FEATURE_LEVELs, // supported feature levels
		ARRAYSIZE(supportedD3D_FEATURE_LEVELs),
		D3D11_SDK_VERSION,
		pID3D11Device.GetAddressOf(),
		&returnedD3D_FEATURE_LEVEL, // returns feature level of device created
		&pID3D11DeviceContext // returns the device immediate context
		);

	if (SUCCEEDED(hr))
	{
		if (pD3D_FEATURE_LEVEL)
		{
			*pD3D_FEATURE_LEVEL = returnedD3D_FEATURE_LEVEL;
		}
	}

	return hr;
}

HRESULT Direct2DRenderer::CreateDeviceSwapChainBitmap()
{
	Microsoft::WRL::ComPtr<IDXGIDevice1> pIDXGIDevice1;

	HRESULT hr = _pID3D11Device1.As(&pIDXGIDevice1);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"_pID3D11Device1.As(&pIDXGIDevice)\n");

	hr = m_pD2DFactory->CreateDevice(pIDXGIDevice1.Get(), &_pID2D1Device);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"m_pD2DFactory->CreateDevice(pIDXGIDevice.Get(), &_pID2D1Device)\n");

	hr = _pID2D1Device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_pRenderTarget);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"_pID2D1Device->CreateDeviceContext\n");

	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = 0U; // use automatic sizing
	swapChainDesc.Height = 0U;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1U; // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0U;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2U; // use double buffering to enable flip
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // CreateSwapChainForHwnd compatible option
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // CreateSwapChainForHwnd compatible option 
	swapChainDesc.Flags = 0U;

	// Identify the physical adapter (GPU or card) this device is runs on.
	Microsoft::WRL::ComPtr<IDXGIAdapter> pIDXGIAdapter;

	hr = pIDXGIDevice1->GetAdapter(&pIDXGIAdapter);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"pIDXGIDevice->GetAdapter\n");

	// Get the factory object that created the DXGI device.
	Microsoft::WRL::ComPtr<IDXGIFactory2> pIDXGIFactory2;

	hr = pIDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory2));
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"pIDXGIAdapter->GetParent\n");

	// Create DXGI swap chain targeting a window handle (the only Windows 7-compatible option)
	hr = pIDXGIFactory2->CreateSwapChainForHwnd(
		_pID3D11Device1.Get(),
		m_hWnd,
		&swapChainDesc,
		nullptr, // assume will always be windowed
		nullptr,
		&_pIDXGISwapChain1
		);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"pIDXGIFactory2->CreateSwapChainForHwnd\n");

	// DXGI will not interfere with application's handling of window mode changes or Alt+Enter
	hr = pIDXGIFactory2->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"pIDXGIFactory2->MakeWindowAssociation\n");

	// Ensure that DXGI doesn't queue more than one frame at a time.
	hr = pIDXGIDevice1->SetMaximumFrameLatency(1U);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"pIDXGIDevice->SetMaximumFrameLatency\n");

	// Get the back buffer as an IDXGISurface (Direct2D doesn't accept an ID3D11Texture2D directly as a render target)
	Microsoft::WRL::ComPtr<IDXGISurface> pIDXGISurface;

	hr = _pIDXGISwapChain1->GetBuffer(0, IID_PPV_ARGS(&pIDXGISurface));
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"_pIDXGISwapChain1->GetBuffer\n");

	hr = m_pD2DFactory->ReloadSystemMetrics();
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"m_pD2DFactory->ReloadSystemMetrics()\n");

	m_pD2DFactory->GetDesktopDpi(&m_dpiX, &m_dpiY);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"m_pD2DFactory->GetDesktopDpi\n");

	// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_IGNORE),
		m_dpiX,
		m_dpiY
		);

	hr = m_pRenderTarget->CreateBitmapFromDxgiSurface(
		pIDXGISurface.Get(),
		&bitmapProperties,
		&_pID2D1Bitmap1_BackBuffer
		);
	if (FAILED(hr)) { return hr; }
	OutputDebugStringW(L"m_pRenderTarget->CreateBitmapFromDxgiSurface\n");

	// Set surface as render target in Direct2D device context
	m_pRenderTarget->SetTarget(_pID2D1Bitmap1_BackBuffer.Get());
	OutputDebugStringW(L"m_pRenderTarget->SetTarget\n");

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

		// Create Direct3D device and context
		Microsoft::WRL::ComPtr<ID3D11Device> pID3D11Device;

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pID3D11DeviceContext;

		// Try to create a hardware-accelerated device
		hr = CreateDeviceD3D11(
			D3D_DRIVER_TYPE_HARDWARE,
			pID3D11Device,
			pID3D11DeviceContext
			);

		// If a GPU is unavailable, try to fall back to WARP
		if (DXGI_ERROR_UNSUPPORTED == hr)
		{
			hr = CreateDeviceD3D11(
				D3D_DRIVER_TYPE_WARP,
				pID3D11Device,
				pID3D11DeviceContext
				);
		}

		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"CreateDeviceD3D11\n");

		hr = pID3D11Device.As(&_pID3D11Device1);
		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"pID3D11Device.As(&_pID3D11Device1)\n");

		hr = pID3D11DeviceContext.As(&_pID3D11DeviceContext1);
		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"pID3D11DeviceContext.As(&_pID3D11DeviceContext1)\n");

		hr = CreateDeviceSwapChainBitmap();
		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"CreateDeviceSwapChainBitmap()\n");

		_MaximumBitmapSize = m_pRenderTarget->GetMaximumBitmapSize();
		OutputDebugStringW(L"m_pRenderTarget->GetMaximumBitmapSize()\n");

		_Direct2DRenderTargetSize = m_pRenderTarget->GetPixelSize();
		OutputDebugStringW(L"m_pRenderTarget->GetPixelSize()\n");

        // Create a black brush.
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"m_pRenderTarget->CreateSolidColorBrush (Black)\n");

        // Create a black brush.
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
		if (FAILED(hr)) { return hr; }
		OutputDebugStringW(L"m_pRenderTarget->CreateSolidColorBrush (White)\n");
    }

    return hr;
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void Direct2DRenderer::DiscardDeviceResources()
{
	SafeRelease(m_pBlackBrush.GetAddressOf());
	SafeRelease(m_pWhiteBrush.GetAddressOf());
	//SafeRelease(m_pContextDst.GetAddressOf());
	SafeRelease(m_pRenderTarget.GetAddressOf());
	SafeRelease(_pID3D11Device1.GetAddressOf());
	SafeRelease(_pID3D11DeviceContext1.GetAddressOf());
	SafeRelease(_pID2D1Device.GetAddressOf());
	SafeRelease(_pIDXGISwapChain1.GetAddressOf());
	SafeRelease(_pID2D1Bitmap1_BackBuffer.GetAddressOf());

	DeviceResourcesDiscarded = true;
}

//
//  If the application receives a WM_SIZE message, this method resizes the render target appropriately.
//
HRESULT Direct2DRenderer::OnResize(UINT width, UINT height)
{
	HRESULT hr = DXGI_ERROR_INVALID_CALL;

	// If the size of the render target has actually changed
	if (_Direct2DRenderTargetSize.width != width && _Direct2DRenderTargetSize.height != height)
	{
		CalculateBitmapTranslatePoint(D2D1::SizeU(width, height));

		// Can get here while resources are being re-created, so make sure you have valid pointers
		if (m_pRenderTarget.Get())
		{
			m_pRenderTarget->SetTarget(nullptr);

			hr = _pIDXGISwapChain1->ResizeBuffers(
				0U,
				0U, 0U,
				DXGI_FORMAT_UNKNOWN,
				0U
				);

			if (SUCCEEDED(hr))
			{
				hr = CreateDeviceSwapChainBitmap();
			}
			else
			{
				DiscardDeviceResources();
			}
		}
	}

	return hr;
}

inline HRESULT Direct2DRenderer::GIF_GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo)
{
	Microsoft::WRL::ComPtr<IWICMetadataQueryReader> pWICMetadataQueryReader;

	PROPVARIANT propValue = {0};
    PropVariantInit(&propValue);

	HRESULT hr = pWICBitmapFrameDecode->GetMetadataQueryReader(&pWICMetadataQueryReader);

	if (SUCCEEDED(hr))
    {
        hr = pWICMetadataQueryReader->GetMetadataByName(L"/imgdesc/Left", &propValue);

        if (SUCCEEDED(hr))
        {
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_BOOL == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI1 == propValue.vt) ? S_OK : E_FAIL;

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

	return hr;
}

inline HRESULT Direct2DRenderer::GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo)
{
	Microsoft::WRL::ComPtr<IWICMetadataQueryReader> pQueryReader;

	HRESULT hr = pWICBitmapFrameDecode->GetMetadataQueryReader(&pQueryReader);

	if (SUCCEEDED(hr))
	{
		PROPVARIANT propvariantOrientationFlag = { 0 };
		PropVariantInit(&propvariantOrientationFlag);

		hr = pQueryReader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &propvariantOrientationFlag);
		// if cannot find EXIF orientation try xmp
		if (FAILED(hr))
		{
			pQueryReader->GetMetadataByName(L"/xmp/tiff:Orientation", &propvariantOrientationFlag);
		}

		FrameInfo->RotationFlag = static_cast<unsigned char>(PropVariantToUInt16WithDefault(propvariantOrientationFlag, 1U));

		PropVariantClear(&propvariantOrientationFlag);

		/*if (!(1 <= FrameInfo->RotationFlag && FrameInfo->RotationFlag <= 8)) // do not change value of rotation flag, deal with incorrect data in other functions e.g. rotate will set these to 1
		{
			FrameInfo->RotationFlag = 1;
		}*/
		//hr = pQueryReader->GetMetadataByName(L"/com/TextEntry", &propvariantImageDescription); // usually information added by jpeg encoders, so pointless to display user

		PROPVARIANT propvariantImageDescription = { 0 };
		PropVariantInit(&propvariantImageDescription);

		hr = pQueryReader->GetMetadataByName(L"/app1/ifd/{ushort=270}", &propvariantImageDescription);

		if (SUCCEEDED(hr))
		{
			LPWSTR pszTitle;

			hr = PropVariantToStringAlloc(propvariantImageDescription, &pszTitle);

			if (SUCCEEDED(hr))
			{
				hr = AllSpaces(pszTitle) ? E_FAIL : S_OK;

				if (SUCCEEDED(hr))
				{
					FrameInfo->Title = pszTitle;
				}

				CoTaskMemFree(pszTitle);
			}			
		}

		PropVariantClear(&propvariantImageDescription);

		if (FAILED(hr))
		{
			FrameInfo->Title = L"";

			hr = S_OK;
		}
	}
	else // if cannot get QueryReader
	{
		FrameInfo->RotationFlag = 1U;

		FrameInfo->Title = L"";

		hr = S_OK;
	}

	return hr;
}

//
// Creates a Direct2D bitmap from the specified file name.
//
HRESULT Direct2DRenderer::LoadBitmapFromFile(
	IWICImagingFactory2 *pIWICFactory,
	LPCWSTR FileName,
	IWICColorContext *pContextDst,
    IMAGE_INFO *ImageInfo
    )
{
	ImageInfo->LoadResult = E_FAIL;

	Microsoft::WRL::ComPtr<IWICBitmapDecoder> pDecoder;

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

	UINT NumberOfFrames = 0U;

	if (SUCCEEDED(hr))
    {OutputDebugStringW(L"pIWICFactory->CreateDecoderFromFilename\n");
        hr = pDecoder->GetFrameCount(&NumberOfFrames);
    }

	if (SUCCEEDED(hr))
    {
		ImageInfo->aFrameInfo.resize(NumberOfFrames);
    }
	
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetContainerFormat(&(ImageInfo->guidContainerFormat));
	}

	if (SUCCEEDED(hr))
	{
		if (ImageInfo->guidContainerFormat == GUID_ContainerFormatGif)
		{
			hr = GIF_GetGlobalMetadata(pDecoder.Get(), ImageInfo); // if returns no size?
		}
	}

	if (SUCCEEDED(hr))
	{OutputDebugStringW(L"pDecoder->GetFrameCount\n");
		for (UINT i = 0U; i < ImageInfo->aFrameInfo.size(); i++)
		{WCHAR buffer[260] = {0}; StringCchPrintfW(buffer, 260, L"Frame: %d\n", i); OutputDebugStringW(buffer);
			
			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> pSource;

			hr = pDecoder->GetFrame(i, &pSource);

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"pDecoder->GetFrame(i, &pSource)\n");
				UINT width, height = 0U;

				if (SUCCEEDED(pSource->GetSize(&width, &height)))
				{
					if (width > _MaximumBitmapSize || height > _MaximumBitmapSize)
					{
						hr = D2DERR_MAX_TEXTURE_SIZE_EXCEEDED;
					}
				}
			}

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"Checked size\n");
				if (GUID_ContainerFormatGif == ImageInfo->guidContainerFormat)
				{
					hr = GIF_GetFrameMetadata(pSource.Get(), &(ImageInfo->aFrameInfo[i]));
					ImageInfo->aFrameInfo[i].RotationFlag = 1U; // Not located in metadata
				}
				else
				{
					hr = GetFrameMetadata(pSource.Get(), &(ImageInfo->aFrameInfo[i]));
				}
			}

			Microsoft::WRL::ComPtr<IWICColorTransform> pColorTransform;

			UINT colorContextCount = 0U;

			// Transform colours
			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"GetMetadata(pSource, ImageInfo->aFrameInfo[i])\n");
				hr = pSource->GetColorContexts(0U, nullptr, &colorContextCount);
				if (FAILED(hr) || 0U == colorContextCount)
				{OutputDebugStringW(L"pSource->GetColorContexts(0U, NULL, &colorContextCount) FAILED or returned 0\n");
					colorContextCount = 1U;
					hr = S_OK;
				}

				std::vector<Microsoft::WRL::ComPtr<IWICColorContext>> contexts;

				contexts.resize(colorContextCount);

				//IWICColorContext **contexts = new IWICColorContext*[colorContextCount];
				for (UINT j = 0U; j < colorContextCount; j++)
				{
					if (SUCCEEDED(hr))
					{
						hr = pIWICFactory->CreateColorContext(&contexts[j]);
					}
				}

				if (SUCCEEDED(hr))
				{OutputDebugStringW(L"pIWICFactory->CreateColorContext(&contexts[i])\n");
					hr = pSource->GetColorContexts(colorContextCount, contexts.data()->GetAddressOf(), &colorContextCount);
					if (FAILED(hr) || 0U == colorContextCount)
					{OutputDebugStringW(L"pSource->GetColorContexts(colorContextCount, contexts, &colorContextCount) FAILED or returned 0\n");
						// A sRGB color space
						hr = contexts[0]->InitializeFromExifColorSpace(1U);

						if (SUCCEEDED(hr))
						{
							colorContextCount = 1U;
						}
					}
				}

				if (SUCCEEDED(hr))
				{OutputDebugStringW(L"ColorContext(s) initialised\n");
					WICPixelFormatGUID PixelFormat;
					hr = pSource->GetPixelFormat(&PixelFormat);
					if (SUCCEEDED(hr))
					{OutputDebugStringW(L"pSource->GetPixelFormat\n");
						hr = m_pWICFactory->CreateColorTransformer(&pColorTransform);
						if (SUCCEEDED(hr))
						{OutputDebugStringW(L"m_pWICFactory->CreateColorTransformer\n");
							for (UINT j = 0U; j < colorContextCount; j++)
							{
								hr = pColorTransform->Initialize(pSource.Get(), contexts[j].Get(), pContextDst, PixelFormat);
								if (SUCCEEDED(hr))
								{OutputDebugStringW(L"pColorTransform->Initialize(pSource, contexts[i], pContextDst, PixelFormat)\n");
									break;
								}
							}
						}
					}
				}

				/*for (UINT j = 0U; j < colorContextCount; j++)
				{
					SafeRelease(&contexts[j]);
				}
				delete[] contexts;*/

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

			Microsoft::WRL::ComPtr<IWICFormatConverter> pIWICFormatConverter;

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"Handled rotation\n");
				// Convert the image format to 32bppPBGRA
				// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
				hr = pIWICFactory->CreateFormatConverter(&pIWICFormatConverter);
			}

			if (SUCCEEDED(hr))
			{OutputDebugStringW(L"CreateFormatConverter\n");
				hr = pIWICFormatConverter->Initialize(
						//ImageInfo->aFrameInfo[i].RotationFlag != 1 ? (IWICBitmapSource*)pFlipRotator : ((colorContextCount == 0U) ? (IWICBitmapSource*)pSource : (IWICBitmapSource*)pColorTransform),
						(0U == colorContextCount) ? static_cast<IWICBitmapSource*>(pSource.Get()) : static_cast<IWICBitmapSource*>(pColorTransform.Get()),
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.0f,
						WICBitmapPaletteTypeMedianCut
						);
			}
			

			Microsoft::WRL::ComPtr<IWICBitmapSource> pIWICBitmapSource;

			hr = pIWICFormatConverter.As(&pIWICBitmapSource);

			if (SUCCEEDED(hr))
			{
				ImageInfo->aFrameInfo[i].pIWICBitmapSource.Swap(pIWICBitmapSource);
			}

			OutputDebugStringW(L"pConverter->Initialize\n");

			if (SUCCEEDED(hr))
			{
				if (ImageInfo->guidContainerFormat != GUID_ContainerFormatGif)
				{
					UINT uiWidth = 0U;
					UINT uiHeight = 0U;

					hr = ImageInfo->aFrameInfo[i].pIWICBitmapSource->GetSize(&uiWidth, &uiHeight);

					if (SUCCEEDED(hr))
					{
						ImageInfo->aFrameInfo[i].Size.width = static_cast<FLOAT>(uiWidth);
						ImageInfo->aFrameInfo[i].Size.height = static_cast<FLOAT>(uiHeight);
					}
				}
			}
		}
	}

	if (GUID_ContainerFormatGif == ImageInfo->guidContainerFormat)
	{
		for (UINT i = 0U; i < ImageInfo->aFrameInfo.size() && SUCCEEDED(hr); i++)
		{
			Microsoft::WRL::ComPtr<IWICBitmap> pIWICBitmap;

			hr = m_pWICFactory->CreateBitmap(
				ImageInfo->GifInfo.Size.width,
				ImageInfo->GifInfo.Size.height,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapCacheOnDemand,
				&pIWICBitmap
				);

			if (SUCCEEDED(hr))
			{
				Microsoft::WRL::ComPtr<ID2D1RenderTarget> pID2D1RenderTarget;

				D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties();
				renderTargetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
				renderTargetProperties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
				renderTargetProperties.dpiX = m_dpiX;
				renderTargetProperties.dpiY = m_dpiY;
				renderTargetProperties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
				renderTargetProperties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

				hr = m_pD2DFactory->CreateWicBitmapRenderTarget(
					pIWICBitmap.Get(),
					renderTargetProperties,
					&pID2D1RenderTarget
					);

				if (SUCCEEDED(hr))
				{
					// Start producing the bitmap
					pID2D1RenderTarget->BeginDraw();

					// If first frame
					if (0U == i)
					{
						// Draw background
						pID2D1RenderTarget->Clear(ImageInfo->GifInfo.BackgroundColor);
					}
					else
					{
						switch (ImageInfo->aFrameInfo[i - 1U].m_uFrameDisposal)
						{
						case DM_UNDEFINED:
						case DM_NONE:
						{
							Microsoft::WRL::ComPtr<ID2D1Bitmap> pID2D1Bitmap;

							// Create a Direct2D bitmap from the WIC bitmap.
							hr = pID2D1RenderTarget->CreateBitmapFromWicBitmap(
								ImageInfo->aFrameInfo[i - 1U].pIWICBitmapSource.Get(),
								NULL,
								&pID2D1Bitmap
								);

							if (SUCCEEDED(hr))
							{
								// We simply draw on the previous frames
								pID2D1RenderTarget->DrawBitmap(pID2D1Bitmap.Get(), NULL);
							}
						}
						break;
						case DM_BACKGROUND: // Clear the area covered by the current raw frame with background color
						{
							// Clip the render target to the size of the raw frame
							pID2D1RenderTarget->PushAxisAlignedClip(&ImageInfo->aFrameInfo[i - 1U].m_framePosition, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

							pID2D1RenderTarget->Clear(ImageInfo->GifInfo.BackgroundColor);

							// Remove the clipping
							pID2D1RenderTarget->PopAxisAlignedClip();
						}
						break;
						case DM_PREVIOUS:
						{
							// We restore the previous composed frame first
							if (i >= 2)
							{
								Microsoft::WRL::ComPtr<ID2D1Bitmap> pID2D1Bitmap;

								// Create a Direct2D bitmap from the WIC bitmap.
								hr = pID2D1RenderTarget->CreateBitmapFromWicBitmap(
									ImageInfo->aFrameInfo[i - 2U].pIWICBitmapSource.Get(),
									NULL,
									&pID2D1Bitmap
									);

								if (SUCCEEDED(hr))
								{
									pID2D1RenderTarget->DrawBitmap(pID2D1Bitmap.Get(), NULL);
								}
							}
						}
						break;
						}
					}

					Microsoft::WRL::ComPtr<ID2D1Bitmap> pID2D1Bitmap;

					// Create a Direct2D bitmap from the WIC bitmap.
					hr = pID2D1RenderTarget->CreateBitmapFromWicBitmap(
						ImageInfo->aFrameInfo[i].pIWICBitmapSource.Get(),
						NULL,
						&pID2D1Bitmap
						);

					// Produce the frame
					pID2D1RenderTarget->DrawBitmap(pID2D1Bitmap.Get(), ImageInfo->aFrameInfo[i].m_framePosition);

					hr = pID2D1RenderTarget->EndDraw();

					if (SUCCEEDED(hr))
					{
						Microsoft::WRL::ComPtr<IWICBitmapSource> pIWICBitmapSource;

						hr = pIWICBitmap.As(&pIWICBitmapSource);

						if (SUCCEEDED(hr))
						{
							ImageInfo->aFrameInfo[i].pIWICBitmapSource.Swap(pIWICBitmapSource);
						}
					}
				}
			}
		}
	}

	ImageInfo->LoadResult = hr;

    return hr;
}

HRESULT Direct2DRenderer::SetHwnd(HWND hWnd)
{
	if (nullptr == hWnd)
	{
		return E_POINTER;
	}

	if (!IsWindow(hWnd))
	{
		return E_HANDLE;
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

	if ((_Direct2DRenderTargetSize.width >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window larger than image
	{
		m_ZoomCentre = D2D1::Point2F(m_BitmapSizeFitToWindow.width/2.0f, m_BitmapSizeFitToWindow.height/2.0f);
	}
	else if ((_Direct2DRenderTargetSize.width < m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window thinner than image
	{
		m_ZoomCentre = D2D1::Point2F((FLOAT)x - m_BitmapTranslatePoint.x - m_TranslatePoint.x, m_BitmapSizeFitToWindow.height/2.0f);
	}
	else if ((_Direct2DRenderTargetSize.width >= m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height < m_zoomFactor*m_zoom*m_BitmapSizeFitToWindow.height)) // if at next higher zoom window shorter than image
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

		if ((_Direct2DRenderTargetSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
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
	if ((_Direct2DRenderTargetSize.width >= m_zoom*m_BitmapSizeFitToWindow.width) && (_Direct2DRenderTargetSize.height >= m_zoom*m_BitmapSizeFitToWindow.height)) // if window larger than image
	{
		OutputDebugStringW(L"SetTranslate: window larger than image\n");
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
	m_TranslatePoint.x = m_TranslatePointEnd.x + (96.0f / m_dpiX)*(FLOAT)x;

	/*if (m_TranslatePoint.x > m_zoom*m_BitmapSizeFitToWindow.width/2.0f)
	{
		m_TranslatePoint.x = m_zoom*m_BitmapSizeFitToWindow.width/2.0f;
	}
	else if (m_TranslatePoint.x < -m_zoom*m_BitmapSizeFitToWindow.width/2.0f)
	{
		m_TranslatePoint.x = -m_zoom*m_BitmapSizeFitToWindow.width/2.0f;
	}*/

	m_TranslatePoint.y = m_TranslatePointEnd.y + (96.0f / m_dpiY)*(FLOAT)y;

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
		hThreadCacheFileNameNext = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(LPVOID), // unsigned stack_size
			StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * )
			this, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

		hr = hThreadCacheFileNameNext ? S_OK : HRESULT_FROM_WIN32(GetLastError());

		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNamePrevious = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(LPVOID), // unsigned stack_size
				StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * )
				this, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

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
		(void)LoadBitmapFromFile(m_pWICFactory.Get(), FileName, m_pContextDst.Get(), &m_ImageCurrent);

		(void)ResetRenderingParameters();
	}

	if (hThreadCreateFileNameVectorFromDirectory)
	{
		hr = (WaitForSingleObject(hThreadCreateFileNameVectorFromDirectory, INFINITE) != WAIT_FAILED) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		(void)SetTitleBarText();
	}

	if (g_Files.size() > 1)
	{
		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNameNext = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(LPVOID), // unsigned stack_size
				StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * )
				this, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

			hr = hThreadCacheFileNameNext ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}

		if (SUCCEEDED(hr))
		{
			hThreadCacheFileNamePrevious = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(LPVOID), // unsigned stack_size
				StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * )
				this, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

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

	m_TranslatePoint = D2D1::Point2F(0.0F, 0.0F);
	m_TranslatePointEnd = D2D1::Point2F(0.0F, 0.0F);

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

	(void)WaitForSingleObject(hThreadCacheFileNameNext, INFINITE);
	(void)WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE); // cludge

	if (0U != g_FileNamePosition)
	{
		g_FileNamePositionPrevious = g_FileNamePosition - 1U;
	}
	else
	{
		g_FileNamePositionPrevious = g_Files.size() - 1U;
	}

	g_FileNamePosition = g_FileNamePositionNext;

	m_ImageCurrent = m_ImageNext;

	(void)ResetRenderingParameters();

	hThreadCacheFileNameNext = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
		NULL, // void *security
		sizeof(LPVOID), // unsigned stack_size
		StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * )
		this, // void *arglist
		0U, // unsigned initflag
		NULL // unsigned *thrdaddr
		));

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
	if (1U == m_ImageCurrent.aFrameInfo.size() || AnimationRunning)
	{
		return S_OK;
	}

	if (m_FrameCurrent + 1U < m_ImageCurrent.aFrameInfo.size())
	{
		m_FrameCurrent = m_FrameCurrent + 1U;
	}
	else
	{
		m_FrameCurrent = 0U;
	}

	HRESULT hr = SetTitleBarText();
	if (FAILED(hr)) { return hr; }

	return OnRender();
}

HRESULT Direct2DRenderer::GIF_OnFrameNext()
{
	(void)KillTimer(m_hWnd, DELAY_TIMER_ID);

	if (m_ImageCurrent.aFrameInfo.size() == 1U || !AnimationRunning || m_ImageCurrent.aFrameInfo[m_FrameCurrent].UserInputFlag)
	{
		if (m_ImageCurrent.aFrameInfo[m_FrameCurrent].UserInputFlag)
		{
			(void)MessageBeep(MB_ICONASTERISK);
		}

		AnimationRunning = false;

		return S_OK;
	}

	if (m_FrameCurrent + 1U < m_ImageCurrent.aFrameInfo.size())
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
	(void)SetTimer(m_hWnd, DELAY_TIMER_ID, m_ImageCurrent.aFrameInfo[m_FrameCurrent].m_uFrameDelay, NULL);

	return OnRender();
}

HRESULT Direct2DRenderer::OnFramePrevious()
{
	if (m_ImageCurrent.aFrameInfo.size() == 1U || AnimationRunning)
	{
		return S_OK;
	}

	if (m_FrameCurrent != 0U)
	{
		m_FrameCurrent = m_FrameCurrent - 1U;
	}
	else
	{
		m_FrameCurrent = static_cast<UINT>(m_ImageCurrent.aFrameInfo.size()) - 1U;
	}

	HRESULT hr = SetTitleBarText();
	if (FAILED(hr)) { return hr; }

	return OnRender();
}

HRESULT Direct2DRenderer::OnNext()
{
	HRESULT hr = S_OK;

	(void)WaitForSingleObject(hThreadCacheFileNameNext, INFINITE);
	//TerminateThread(hThreadCacheFileNamePrevious, 0);
	(void)WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE); // cludge

	g_FileNamePositionPrevious = g_FileNamePosition;
	g_FileNamePosition = g_FileNamePositionNext;

	for (auto it = m_ImageCurrent.aFrameInfo.begin(); it != m_ImageCurrent.aFrameInfo.end(); ++it)
	{
		SafeRelease(it->pID2D1Bitmap1.GetAddressOf());
	}

	for (auto it = m_ImageNext.aFrameInfo.begin(); it != m_ImageNext.aFrameInfo.end(); ++it)
	{
		SafeRelease(it->pID2D1Bitmap1.GetAddressOf());
	}

	m_ImagePrevious = m_ImageCurrent;
	m_ImageCurrent = m_ImageNext;

	(void)ResetRenderingParameters();

	hThreadCacheFileNameNext = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
		NULL, // void *security
		sizeof(LPVOID), // unsigned stack_size
		StaticCacheFileNameNext, // unsigned ( __stdcall *start_address )( void * )
		this, // void *arglist
		0U, // unsigned initflag
		NULL // unsigned *thrdaddr
		));

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
		hr = SetTitleBarText();
	}

	return hr;
}

HRESULT Direct2DRenderer::OnPrevious()
{
	HRESULT hr = S_OK;

	(void)WaitForSingleObject(hThreadCacheFileNamePrevious, INFINITE);
	//TerminateThread(hThreadCacheFileNameNext, 0);
	(void)WaitForSingleObject(hThreadCacheFileNameNext, INFINITE); // cludge

	g_FileNamePositionNext = g_FileNamePosition;
	g_FileNamePosition = g_FileNamePositionPrevious;

	for (auto it = m_ImageCurrent.aFrameInfo.begin(); it != m_ImageCurrent.aFrameInfo.end(); ++it)
	{
		SafeRelease(it->pID2D1Bitmap1.GetAddressOf());
	}

	for (auto it = m_ImagePrevious.aFrameInfo.begin(); it != m_ImagePrevious.aFrameInfo.end(); ++it)
	{
		SafeRelease(it->pID2D1Bitmap1.GetAddressOf());
	}

	m_ImageNext = m_ImageCurrent;
	m_ImageCurrent = m_ImagePrevious;

	(void)ResetRenderingParameters();

	hThreadCacheFileNamePrevious = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
		NULL, // void *security
		sizeof(LPVOID), // unsigned stack_size
		StaticCacheFileNamePrevious, // unsigned ( __stdcall *start_address )( void * )
		this, // void *arglist
		0U, // unsigned initflag
		NULL // unsigned *thrdaddr
		));

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
		hr = SetTitleBarText();
	}

	return hr;
}

HRESULT Direct2DRenderer::SetTitleBarText()
{
	HRESULT hr = E_FAIL;

	// If you successfully loaded the image
	if (SUCCEEDED(m_ImageCurrent.LoadResult))
	{
		short FileTitleLength = 0;

		// if there is a title
		if (!m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title.empty())
		{
			// get its length
			FileTitleLength = static_cast<short>(m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title.length());
		}
		else
		{
			// get the title from the path name
			FileTitleLength = GetFileTitleW(g_Files[g_FileNamePosition].FullPath.c_str(), NULL, 0);
			if (FileTitleLength <= 0)
			{
				return E_FAIL;
			}
		}

		std::wstring TitleBarText;

		// If there is more than one frame
		if (m_ImageCurrent.aFrameInfo.size() > 1U &&
			// and the image is not a GIF
			GUID_ContainerFormatGif != m_ImageCurrent.guidContainerFormat)
		{
			std::wstring FramePart;

			size_t FramePartLength = 8U/*space+bracket+"Frame"+space*/ + NumberOfDigits(static_cast<int32_t>(m_FrameCurrent)) + 1U + 1U/*slash*/ + NumberOfDigits(static_cast<int32_t>(m_ImageCurrent.aFrameInfo.size())) + 2U/*bracket+null char*/;

			FramePart.resize(FramePartLength);

			hr = StringCchPrintfW(&FramePart[0], FramePartLength, L" (Frame %d/%d)", m_FrameCurrent + 1U, m_ImageCurrent.aFrameInfo.size());

			TitleBarText.resize(FileTitleLength + FramePartLength);

			if (!m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title.empty())
			{
				TitleBarText = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title;
			}
			else
			{
				(void)GetFileTitleW(g_Files[g_FileNamePosition].FullPath.c_str(), &TitleBarText[0], static_cast<WORD>(FileTitleLength));
			}

			TitleBarText = TitleBarText + FramePart;
		}
		else
		{
			TitleBarText.resize(FileTitleLength);

			if (!m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title.empty())
			{
				TitleBarText = m_ImageCurrent.aFrameInfo[m_FrameCurrent].Title;
			}
			else
			{
				(void)GetFileTitleW(g_Files[g_FileNamePosition].FullPath.c_str(), &TitleBarText[0], static_cast<WORD>(FileTitleLength));
			}

			hr = S_OK;
		}

		if (SUCCEEDED(hr))
		{
			hr = SetWindowTextW(m_hWnd, TitleBarText.c_str()) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	}

	return hr;
}

HRESULT Direct2DRenderer::EnumerateDecoders(COMDLG_FILTERSPEC **paFilterSpec, UINT *cFileTypes)
{
	if (m_pWICFactory.Get())
	{
		return EnumerateDecoders(m_pWICFactory.Get(), paFilterSpec, cFileTypes);
	}
	else
	{
		return E_POINTER;
	}
}

HRESULT Direct2DRenderer::EnumerateDecoders(IWICImagingFactory2 *pIWICFactory, COMDLG_FILTERSPEC **paFilterSpec, UINT *cFileTypes)
{
	Microsoft::WRL::ComPtr<IEnumUnknown> piEnumUnknown;

	HRESULT hr = pIWICFactory->CreateComponentEnumerator(
		WICDecoder,
		// WICComponentEnumerateRefresh will makes sure registry settings are read for new decoders
		WICComponentEnumerateDefault | WICComponentEnumerateRefresh,
		&piEnumUnknown);

	if (SUCCEEDED(hr))
	{
		hr = piEnumUnknown->Reset();

		if (SUCCEEDED(hr))
		{
			Microsoft::WRL::ComPtr<IUnknown> piUnknown;
			ULONG num = 0UL;
			UINT NumberOfDecoders = 0U;

			while ((SUCCEEDED(piEnumUnknown->Next(1UL, &piUnknown, &num))) && (1UL == num))
			{
				NumberOfDecoders++;
			}

			*cFileTypes = NumberOfDecoders + 2U; // 1 for all found extensions and 1 for All files (*.*)

			if (nullptr == paFilterSpec) // function called this way to return number of decoders
			{
				return S_OK;
			}

			hr = piEnumUnknown->Reset();

			if (SUCCEEDED(hr))
			{
				LPWSTR pszAllExtensions = nullptr; // not deleted
				pszAllExtensions = new WCHAR[1000]; // find the length properly, don't be lazy
				wmemset(pszAllExtensions, 0, 1000);

				UINT CurrentDecoder = 1U;

				while ((SUCCEEDED(piEnumUnknown->Next(1L, &piUnknown, &num))) && (num == 1L))
				{
					Microsoft::WRL::ComPtr<IWICBitmapDecoderInfo> piBitmapDecoderInfo;

					hr = piUnknown.As(&piBitmapDecoderInfo);

					if (SUCCEEDED(hr))
					{
						GUID guidContainerFormat = GUID_NULL;
						
						hr = piBitmapDecoderInfo->GetContainerFormat(&guidContainerFormat);

						if (SUCCEEDED(hr))
						{
							Microsoft::WRL::ComPtr<IWICBitmapEncoder> pEncoder;

							bool HasEncoder = false;

							if (SUCCEEDED(pIWICFactory->CreateEncoder(guidContainerFormat, NULL, &pEncoder)))
							{
								HasEncoder = true;
							}
							else
							{
								HasEncoder = false;
							}

							DecoderHasEncoder.insert(std::pair<GUID, bool>(guidContainerFormat, HasEncoder));
						}

						UINT uActual = 0U;

						if (SUCCEEDED(hr))
						{
							hr = piBitmapDecoderInfo->GetFriendlyName(0U, NULL, &uActual);
						}

						LPWSTR pszFriendlyName = nullptr;

						std::wstring pszExtensions;

						LPWSTR pszSpecTemp = nullptr;

						LPWSTR pszName = nullptr; // not deleted

						LPWSTR pszSpec = nullptr; // not deleted

						if (SUCCEEDED(hr))
						{
							if (uActual > 0U)
							{
								pszFriendlyName = new WCHAR[uActual];

								if (pszFriendlyName)
								{
									(void)wmemset(pszFriendlyName, 0, uActual);
									hr = piBitmapDecoderInfo->GetFriendlyName(uActual, pszFriendlyName, &uActual);
								}

								pszName = new WCHAR[uActual - 8U]; // subtract 8 characters of " Decoder" ending

								if (pszName)
								{
									(void)wmemset(pszName, 0, (uActual - 8U));
									(void)StringCchCopyW(pszName, uActual - 8, pszFriendlyName); // this fails with less buffer error, check
								}
							}
						}

						if (SUCCEEDED(hr))
						{
							// Extension
							hr = piBitmapDecoderInfo->GetFileExtensions(0U, nullptr, &uActual);

							if (uActual > 0U)
							{
								pszExtensions.resize(uActual);

								hr = piBitmapDecoderInfo->GetFileExtensions(uActual, &pszExtensions[0], &uActual);

								pszSpecTemp = new WCHAR[uActual + CountOccurencesOfCharacterInString('.', &pszExtensions)];

								if (pszSpecTemp)
								{
									(void)ReplaceCharInString(pszExtensions.c_str(), pszSpecTemp, '.', L"*.");
								}

								pszSpec = new WCHAR[uActual + CountOccurencesOfCharacterInString('.', &pszExtensions)];

								if (pszSpec)
								{
									(void)ReplaceCharInString(pszSpecTemp, pszSpec, ',', L";");
								}
							}
						}

						(*paFilterSpec)[CurrentDecoder].pszName = pszName;
						(*paFilterSpec)[CurrentDecoder].pszSpec = pszSpec;

						(void)StringCchCatW(pszAllExtensions, 1000, pszSpec);

						if (CurrentDecoder != NumberOfDecoders)
						{
							StringCchCatW(pszAllExtensions, 1000, L";");
						}

						delete[] pszSpecTemp;
						delete[] pszFriendlyName;
					}
					CurrentDecoder++;
				}

				(*paFilterSpec)[0].pszName = L"All image files";
				(*paFilterSpec)[0].pszSpec = pszAllExtensions;

				(*paFilterSpec)[CurrentDecoder].pszName = L"All files";
				(*paFilterSpec)[CurrentDecoder].pszSpec = L"*.*";
			}
		}
	}

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
	Microsoft::WRL::ComPtr<IWICMetadataQueryReader> pMetadataQueryReader;

	PROPVARIANT propValue = {0};
	PropVariantInit(&propValue);

	// Create a MetadataQueryReader from the decoder
	HRESULT hr = pDecoder->GetMetadataQueryReader(&pMetadataQueryReader);

	// Get background color
	if (SUCCEEDED(hr))
    {
		hr = ConformGIF ? GIF_GetBackgroundColor(pDecoder, pMetadataQueryReader.Get(), &(ImageInfo->GifInfo.BackgroundColor)) : E_FAIL;

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI2 == propValue.vt ? S_OK : E_FAIL);

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
            hr = (VT_UI1 == propValue.vt ? S_OK : E_FAIL);

            if (SUCCEEDED(hr))
            {
				UINT uPixelAspRatio = propValue.bVal;

                if (0U != uPixelAspRatio) // 1 sometimes incorrectly used by gif makers to mean 0 here
                {
                    // Need to calculate the ratio. The value in uPixelAspRatio 
                    // allows specifying widest pixel 4:1 to the tallest pixel of 
                    // 1:4 in increments of 1/64th
                    FLOAT pixelAspRatio = (static_cast<FLOAT>(uPixelAspRatio) + 15.0F) / 64.0F;

                    // Calculate the image width and height in pixel based on the
                    // pixel aspect ratio. Only shrink the image.
                    if (pixelAspRatio > 1.0F)
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
			11UL == propValue.caub.cElems &&  // Length of the application block
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
                    1 == propValue.caub.pElems[1])
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
	//		for (UINT i = 0U; i < ImageInfo->aFrameInfo.size(); i++)
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
	//		for (UINT i = 0U; i < ImageInfo->aFrameInfo.size(); i++)
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
	PROPVARIANT propVariant = { 0 };
	PropVariantInit(&propVariant);

	// If we have a global palette, get the palette and background color
	HRESULT hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/GlobalColorTableFlag", &propVariant);
	if (FAILED(hr)) { return hr; }

	// If the value is not a BOOL or there is no global color table
	if (propVariant.vt != VT_BOOL || !propVariant.boolVal)
	{
		return E_FAIL;
	}

	PropVariantClear(&propVariant);

	WICColor rgColors[256] = { 0 };

	// Set sensible fall back to the number of WICColors
	UINT GlobalColorTableSize = ARRAYSIZE(rgColors);

	hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/GlobalColorTableSize", &propVariant);

	if (SUCCEEDED(hr))
	{
		if (VT_UI1 == propVariant.vt)
		{
			GlobalColorTableSize = static_cast<UINT>(pow(2.0f, propVariant.bVal + 1));
		}
	}

	PropVariantClear(&propVariant);

	BYTE backgroundIndex = 0;

	// Background color index
	hr = pMetadataQueryReader->GetMetadataByName(L"/logscrdesc/BackgroundColorIndex", &propVariant);

	if (SUCCEEDED(hr))
	{
		if (VT_UI1 == propVariant.vt)
		{
			backgroundIndex = propVariant.bVal;
		}
		else
		{
			return E_FAIL;
		}
	}

	PropVariantClear(&propVariant);

	Microsoft::WRL::ComPtr<IWICPalette> pWicPalette;

	// Get the color from the palette
	hr = m_pWICFactory->CreatePalette(&pWicPalette);
	if (FAILED(hr)) { return hr; }

	// Get the global palette
	hr = pDecoder->CopyPalette(pWicPalette.Get());
	if (FAILED(hr)) { return hr; }

	UINT cColorsCopied = 0U;

	hr = pWicPalette->GetColors(GlobalColorTableSize, rgColors, &cColorsCopied);
	if (FAILED(hr)) { return hr; }

	// If the background color is outside the range 
	if (backgroundIndex >= GlobalColorTableSize)
	{
		return E_FAIL;
	}

	DWORD dwBGColor = 0;

	// Get the color in ARGB format
	dwBGColor = rgColors[backgroundIndex];

	// The background color is in ARGB format, and we want to 
	// extract the alpha value and convert it in FLOAT
	FLOAT alpha = (dwBGColor >> 24) / 255.0F;

	*BackgroundColor = D2D1::ColorF(dwBGColor, alpha);

	return hr;
}

/*HRESULT Invert()
{
	Microsoft::WRL::ComPtr<IWICImagingFactory2> pFactory;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFactory)
		);

	if (SUCCEEDED(hr))
	{
		INT iWidth = 640;
		INT iHeight = 480;

		WICPixelFormatGUID formatGUID = GUID_WICPixelFormat32bppPBGRA;

		Microsoft::WRL::ComPtr<IWICBitmap> pBitmap;

		hr = pFactory->CreateBitmap(iWidth, iHeight, formatGUID, WICBitmapCacheOnDemand, &pBitmap);

		if (SUCCEEDED(hr))
		{
			WICRect rcLock = { 0, 0, iWidth, iHeight };

			Microsoft::WRL::ComPtr<IWICBitmapLock> pLock;

			hr = pBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

			if (SUCCEEDED(hr))
			{
				UINT cbBufferSize = 0U;
				UINT cbStride = 0U;
				BYTE *pv = nullptr;

				// Retrieve the stride.
				hr = pLock->GetStride(&cbStride);

				if (SUCCEEDED(hr))
				{
					hr = pLock->GetDataPointer(&cbBufferSize, &pv);
				}

				// Zero out memory pointed to by the lock.
				ZeroMemory(pv, cbBufferSize);
			}
		}
	}

    return hr;
}*/

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