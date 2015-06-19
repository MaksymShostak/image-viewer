#pragma once

#include <dwrite_1.h>
#include <Wincodecsdk.h> // IWICMetadataBlockWriter
#include <Icm.h> // GetStandardColorSpaceProfileW
#include <comdef.h>
#include <d3d11_1.h>

extern void ErrorDescription(HRESULT hr);
extern void HRESULTDecode(HRESULT hr, LPWSTR Severity, LPWSTR Facility, LPWSTR ErrorDescription);
extern size_t CountOccurencesOfCharacterInString(wchar_t character, std::wstring * pString);
extern std::list<IMAGEFILE> g_Files;
extern std::list<IMAGEFILE>::iterator g_IteratorCurrent;
extern HANDLE hThreadCreateFileNameVectorFromDirectory;
extern const UINT DELAY_TIMER_ID;

enum DISPOSAL_METHODS
{
    DM_UNDEFINED  = 0,
    DM_NONE       = 1,
    DM_BACKGROUND = 2,
    DM_PREVIOUS   = 3 
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
	HRESULT LoadBitmapCurrent();
	HRESULT OnAnimationStartStop();
	HRESULT OnDelete();
	HRESULT OnFrameNext();
	HRESULT OnFramePrevious();
	HRESULT OnNext();
	HRESULT OnPrevious();
	HRESULT OnRender();
	HRESULT OnResize(UINT width, UINT height);
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

	unsigned int CacheFileNamePrevious();
	unsigned int CacheFileNameNext();
	
private:
	HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

	HRESULT CreateDeviceSwapChainBitmap();

	inline void CalculateBitmapTranslatePoint(D2D1_SIZE_U RenderTargetSize);
	HRESULT CalculateDrawRectangle(D2D1_RECT_F &drawRect);

	inline HRESULT GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo);
	inline HRESULT GIF_GetFrameMetadata(IWICBitmapFrameDecode *pWICBitmapFrameDecode, FRAME_INFO *FrameInfo);
	inline HRESULT GIF_GetGlobalMetadata(IWICBitmapDecoder *pDecoder, IMAGEFILE *ImageInfo);
	inline HRESULT GIF_GetBackgroundColor(IWICBitmapDecoder *pDecoder, IWICMetadataQueryReader *pMetadataQueryReader, D2D1_COLOR_F *BackgroundColor);

	HRESULT LoadBitmapFromFile(
        IWICImagingFactory2 *pIWICFactory,
		IMAGEFILE* file,
		IWICColorContext *pContextDst
        );

	HRESULT RotateByMetadata(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, USHORT *pRotationFlag, bool Clockwise);
	HRESULT RotateJPEG(LPCWSTR FileName, LPCWSTR FileNameTemporary, USHORT RotationFlag, bool Clockwise);
	HRESULT RotateByReencode(IWICImagingFactory2 *pIWICFactory, LPCWSTR FileName, LPCWSTR FileNameTemporary, bool Clockwise);

	HRESULT ResetRenderingParameters();

	HRESULT EnumerateDecoders(IWICImagingFactory2 *pIWICFactory, COMDLG_FILTERSPEC **ppFilterSpec, UINT *cFileTypes);

	HRESULT SetJPEGOrientation(LPCWSTR FileName);

	HWND m_hWnd;
	Microsoft::WRL::ComPtr<ID3D11Device1> _pID3D11Device1;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _pID3D11DeviceContext1;
	Microsoft::WRL::ComPtr<ID2D1Device> _pID2D1Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> _pIDXGISwapChain1;
	// Direct2D target rendering bitmap
	// (linked to DXGI back buffer which is linked to Direct3D pipeline)
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> _pID2D1Bitmap1_BackBuffer;
	Microsoft::WRL::ComPtr<ID2D1Factory1> m_pD2DFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2> m_pWICFactory;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_pRenderTarget;
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
	Microsoft::WRL::ComPtr<IDWriteFactory1> m_pDWriteFactory;
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
	UINT32 _MaximumBitmapSize;
	D2D1_SIZE_U _Direct2DRenderTargetSize;
};