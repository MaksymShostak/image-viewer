#pragma once

#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h> // IWICImagingFactory2
#include <Wincodecsdk.h> // IWICMetadataBlockWriter
#include <Icm.h> // GetStandardColorSpaceProfileW
#include <comdef.h>

extern void ErrorDescription(HRESULT hr);
extern void HRESULTDecode(HRESULT hr, LPWSTR Severity, LPWSTR Facility, LPWSTR ErrorDescription);
extern size_t CountOccurencesOfCharacterInString(wchar_t character, std::wstring * pString);
extern std::vector<FILESTRUCT> g_Files;
extern UINT g_FileNamePosition;
extern UINT FileNamePositionPrevious;
extern UINT FileNamePositionNext;
extern HANDLE hThreadCreateFileNameVectorFromDirectory;
extern const UINT DELAY_TIMER_ID;

enum DISPOSAL_METHODS
{
    DM_UNDEFINED  = 0,
    DM_NONE       = 1,
    DM_BACKGROUND = 2,
    DM_PREVIOUS   = 3 
};

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
	Microsoft::WRL::ComPtr<ID2D1Bitmap> pBitmap;
	D2D1_SIZE_F Size;
	std::wstring Title;
	unsigned char RotationFlag;
	UINT m_uFrameDisposal;
	UINT m_uFrameDelay;
	D2D1_RECT_F m_framePosition;
	bool UserInputFlag;

	FRAME_INFO() :
		m_framePosition(D2D1::RectF(0.0F, 0.0F, 0.0F, 0.0F)),
		m_uFrameDelay(0U),
		m_uFrameDisposal(0U),
		pBitmap(nullptr),
		RotationFlag(1U),
		Size(D2D1::SizeF(0.0F, 0.0F)),
		Title(),
		UserInputFlag(false)
		{}
};

struct IMAGE_INFO
{
	HRESULT LoadResult;
	GUID guidContainerFormat;
	UINT Frames;
	std::vector<FRAME_INFO> aFrameInfo;
	GIF_INFO GifInfo;

	IMAGE_INFO() :
		aFrameInfo(),
		Frames(0U),
		GifInfo(),
		guidContainerFormat(GUID_NULL),
		LoadResult(E_FAIL)
		{}
};

class Direct2DRenderer
{
public:
    Direct2DRenderer();
    //~Direct2DRenderer();
	
	HRESULT ActualSize();
	HRESULT CreateDeviceIndependentResources();
	HRESULT EnumerateDecoders(COMDLG_FILTERSPEC **ppFilterSpec, UINT *cFileTypes);
	HRESULT FitToWindow();
	HRESULT GIF_OnFrameNext();
	HRESULT LoadBitmapCurrent(LPCWSTR FileName);
	HRESULT OnAnimationStartStop();
	HRESULT OnDelete();
	HRESULT OnFrameNext();
	HRESULT OnFramePrevious();
	HRESULT OnNext();
	HRESULT OnPrevious();
	HRESULT OnRender();
	void OnResize(UINT width, UINT height);
	HRESULT ReloadAfterSort();
	HRESULT Rotate(bool Clockwise);
	HRESULT ScaleToWindow();
	void SetDragEnd();
	HRESULT SetCurrentErrorCode(HRESULT hr);
	HRESULT SetHwnd(HWND hWnd);
	HRESULT SetTitleBarText();
	HRESULT SetTranslate(int x, int y);
	HRESULT ToggleBackgroundColor();
	HRESULT ZoomIn(UINT x, UINT y);
	HRESULT ZoomOut(UINT x, UINT y);

	bool Pannable;
	bool ConformGIF;
	bool RotateAutoEnabled();
	bool RotateEnabled();

	friend static unsigned WINAPI StaticCacheFileNamePrevious(LPVOID Param);
	friend static unsigned WINAPI StaticCacheFileNameNext(LPVOID Param);

	unsigned CacheFileNamePrevious(UINT);
	unsigned CacheFileNameNext(UINT);
	
private:
	HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

	inline void CalculateBitmapTranslatePoint(D2D1_SIZE_U RenderTargetSize);
	HRESULT CalculateDrawRectangle(D2D1_RECT_F &drawRect);

	inline HRESULT GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo);
	inline HRESULT GIF_GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo);
	inline HRESULT GIF_GetGlobalMetadata(IWICBitmapDecoder *pDecoder, IMAGE_INFO *ImageInfo);
	inline HRESULT GIF_GetBackgroundColor(IWICBitmapDecoder *pDecoder, IWICMetadataQueryReader *pMetadataQueryReader, D2D1_COLOR_F *BackgroundColor);

	HRESULT LoadBitmapFromFile(
        IWICImagingFactory2 *pIWICFactory,
		LPCWSTR FileName,
		IWICColorContext *pContextDst,
		ID2D1RenderTarget *pRenderTarget,
		IMAGE_INFO *ImageInfo
        );

	HRESULT RotateByMetadata(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, USHORT *pRotationFlag, bool Clockwise);
	HRESULT RotateJPEG(LPCWSTR FileName, LPCWSTR FileNameTemporary, USHORT RotationFlag, bool Clockwise);
	HRESULT RotateByReencode(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, LPCWSTR FileNameTemporary, bool Clockwise);

	HRESULT ResetRenderingParameters();

	HRESULT EnumerateDecoders(IWICImagingFactory2 *pIWICFactory, COMDLG_FILTERSPEC **ppFilterSpec, UINT *cFileTypes);

	HRESULT SetJPEGOrientation(LPCWSTR FileName);

	HWND m_hWnd;
	Microsoft::WRL::ComPtr<ID2D1Factory> m_pD2DFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2> m_pWICFactory;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
	IMAGE_INFO m_ImagePrevious;
	IMAGE_INFO m_ImageCurrent;
	IMAGE_INFO m_ImageNext;
	UINT m_FrameCurrent;
	FLOAT m_dpiX, m_dpiY;
	D2D1_SIZE_F m_BitmapSizeFitToWindow;
	D2D1_POINT_2F m_BitmapTranslatePoint;
	D2D1_POINT_2F m_TranslatePoint;
	D2D1_POINT_2F m_TranslatePointEnd;
	float m_zoomFactor;
	float m_zoom;
	float m_zoomMin;
	float m_zoomMax;
	D2D1_MATRIX_3X2_F m_TransformMatrixTranslation;
	D2D1_MATRIX_3X2_F m_TransformMatrixPanning;
	D2D1_MATRIX_3X2_F m_TransformMatrixScale;
	bool m_FitToWindow;
	bool m_ScaleToWindow;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_pDWriteFactory;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pBlackBrush;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pWhiteBrush;
	Microsoft::WRL::ComPtr<IWICColorContext> m_pContextDst;
	HANDLE hThreadCacheFileNamePrevious;
	HANDLE hThreadCacheFileNameNext;
	bool BackgroundColorBlack;
	bool DeviceResourcesDiscarded;
	std::map<GUID, bool> DecoderHasEncoder;
	UINT m_uLoopNumber;
	bool AnimationRunning;
};