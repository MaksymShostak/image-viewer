#pragma once

#include "resource.h"
#include "HRESULT.h"
#include "Direct2DRenderer.h"

struct COMMONITEMDIALOGOPEN
{
	HWND hWnd;
	std::wstring * pszTitle;
	COMDLG_FILTERSPEC *rgFilterSpec;
	UINT cFileTypes;
	std::wstring * FileName;

	COMMONITEMDIALOGOPEN() :
		hWnd(nullptr),
		pszTitle(nullptr),
		rgFilterSpec(nullptr),
		cFileTypes(0U),
		FileName(nullptr)		
		{}
};

struct DELETEFILEWITHIFO
{
	HWND hWnd;
	std::wstring * FileName;
	bool Permanent;
	bool Silent;

	DELETEFILEWITHIFO() :
		hWnd(nullptr),
		FileName(nullptr),
		Permanent(false),
		Silent(false)
		{}
};

struct RENAMEFILEWITHIFO
{
	std::wstring * FileName;
	std::wstring * FileNameNew;

	RENAMEFILEWITHIFO() :
		FileName(nullptr),
		FileNameNew(nullptr)
		{}
};

enum SORTBY
{
	SORTBYDATEMODIFIED = 0,
	SORTBYNAME = 1,
	SORTBYSIZE = 2
};

// Global Variables:
HINSTANCE hInst = nullptr; // current instance
const WCHAR szTitle[] = L"Image Viewer"; // The title bar text
const WCHAR szWindowClass[] = L"445F6F25-065C-411D-B16E-A3E887660B76"; // the main window class name
std::wstring g_FileName; // L"C:\\Users\\Max\\Desktop\\Optimal_Colony_Layout.png";
std::wstring FileDirectory;
std::vector<std::wstring> g_Directories;
std::list<IMAGEFILE> g_Files;
std::list<IMAGEFILE>::iterator g_IteratorCurrent;
HCURSOR hCursorArrow = nullptr;
HCURSOR hCursorHand = nullptr;
HCURSOR hCursorHandClosed = nullptr;
bool bFullscreen = false;
POINT DragStart = { 0 };
HANDLE hThreadCreateFileNameVectorFromDirectory = nullptr;
COMDLG_FILTERSPEC *FilterSpec = nullptr;
UINT cFileTypes = 0U;
LPWSTR *ArrayOfFileExtensions = nullptr;
UINT NumberOfFileExtensions = 0U;
HMENU hRightClickMenu = nullptr;
HMENU hRightClickMenuTitleBar = nullptr;
//UINT g_NumberOfProcessors = 0U;
volatile std::atomic_bool g_BlockMovement = false;
COMMONITEMDIALOGOPEN g_commonitemdialogopen;
DELETEFILEWITHIFO g_deletefilewithifo;
extern UINT const DELAY_TIMER_ID = 1U;    // Global ID for the timer, only one timer is used
SORTBY g_SortByCurrent = SORTBYNAME;
bool g_SortByAscending = true;
Direct2DRenderer* renderer;

// Forward declarations of functions included in this code module:
void _OnCommand_ID_FILE_ACTUALSIZE(HWND hWnd);
void _OnCommand_ID_FILE_ANIMATIONSTARTSTOP(HWND hWnd);
void _OnCommand_ID_FILE_AUTOROTATE(HWND hWnd);
void _OnCommand_ID_FILE_CLOSEALLWINDOWS(HWND hWnd);
void _OnCommand_ID_FILE_COPY(HWND hWnd);
void _OnCommand_ID_FILE_CUT(HWND hWnd);
void _OnCommand_ID_FILE_DELETE(HWND hWnd);
void _OnCommand_ID_FILE_DELETEPERMANENTLY(HWND hWnd);
void _OnCommand_ID_FILE_EXIT(HWND hWnd);
void _OnCommand_ID_FILE_FIRSTFILE(HWND hWnd);
void _OnCommand_ID_FILE_FITTOWINDOW(HWND hWnd);
void _OnCommand_ID_FILE_FRAMENEXT(HWND hWnd);
void _OnCommand_ID_FILE_FRAMEPREVIOUS(HWND hWnd);
void _OnCommand_ID_FILE_FULLSCREEN(HWND hWnd);
void _OnCommand_ID_FILE_LASTFILE(HWND hWnd);
void _OnCommand_ID_FILE_NEW(HWND hWnd);
void _OnCommand_ID_FILE_NEXT(HWND hWnd);
void _OnCommand_ID_FILE_OPEN(HWND hWnd);
void _OnCommand_ID_FILE_OPENFILELOCATION(HWND hWnd);
void _OnCommand_ID_FILE_PREVIOUS(HWND hWnd);
void _OnCommand_ID_FILE_PROPERTIES(HWND hWnd);
void _OnCommand_ID_FILE_ROTATECLOCKWISE(HWND hWnd);
void _OnCommand_ID_FILE_ROTATECOUNTERCLOCKWISE(HWND hWnd);
void _OnCommand_ID_FILE_SCALETOWINDOW(HWND hWnd);
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CENTER(HWND hWnd);
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CROPTOFIT(HWND hWnd);
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_KEEPASPECT(HWND hWnd);
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_STRETCH(HWND hWnd);
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_TILE(HWND hWnd);
void _OnCommand_ID_FILE_SORT(HWND hWnd, SORTBY SortBy);
void _OnCommand_ID_FILE_SORTBYASCENDING(HWND hWnd);
void _OnCommand_ID_FILE_SORTBYDESCENDING(HWND hWnd);
void _OnCommand_ID_FILE_TOGGLEBACKGROUNDCOLOR(HWND hWnd);
void _OnCommand_ID_HELP_ABOUT(HWND hWnd);
void _OnCommand_RETURNEDFROMCOMMONITEMDIALOGOPEN(HWND hWnd);
void _OnCommand_RETURNEDFROMDELETEFILEWITHIFO(HWND hWnd, UINT codeNotify);

BOOL _OnCreate(HWND hWnd, LPCREATESTRUCT /*lpCreateStruct*/);
void _OnDestroy(HWND /*hWnd*/);
void _OnEndSession(HWND /*hwnd*/, BOOL fEnding);
BOOL _OnEraseBkgnd(HWND /*hWnd*/, HDC /*hDC*/);
void _OnKeyDown(HWND hWnd, UINT vk, BOOL /*fDown*/, int /*cRepeat*/, UINT /*flags*/);
void _OnLButtonDblClk(HWND hWnd, BOOL fDoubleClick, int /*x*/, int /*y*/, UINT /*keyFlags*/);
void _OnLButtonDown(HWND hWnd, BOOL /*fDoubleClick*/, int x, int y, UINT /*keyFlags*/);
void _OnLButtonUp(HWND /*hWnd*/, int /*x*/, int /*y*/, UINT /*keyFlags*/);
void _OnMouseMove(HWND /*hWnd*/, int x, int y, UINT keyFlags);
void _OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT /*fwKeys*/);
void _OnPaint(HWND hWnd);
BOOL _OnQueryEndSession(HWND /*hwnd*/);
void _OnSize(HWND /*hWnd*/, UINT /*state*/, int cx, int cy);
void _OnTimer(HWND hWnd, UINT id);
size_t CountOccurencesOfCharacterInString(wchar_t character, std::wstring * pString);
	
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT DeleteFileWithIFO(__in HWND hWnd, __in std::wstring * FileName, __in bool Permanent, __in bool Silent);
HRESULT RenameFileWithIFO(__in std::wstring * FileName, __in std::wstring * FileNameNew);
HRESULT CommonItemDialogOpen(__in std::wstring * pszTitle, __in COMDLG_FILTERSPEC * rgFilterSpec, __in UINT cFileTypes, __out std::wstring * FileName);
HRESULT DirectoryFromFileName(__out std::wstring * pfileDirectory, __in const wchar_t * filePath);
HRESULT SetAsDesktopBackground(__in std::wstring * FileName, __in DWORD dwStyle);
bool NaturalSort(const std::wstring &lhs, const std::wstring &rhs);
unsigned int __stdcall CreateFileNameVectorFromDirectory(void* _ArgList);
BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
HRESULT CALLBACK TaskDialogCallbackProc(__in  HWND hWnd, __in  UINT uNotification, __in  WPARAM wParam, __in  LPARAM lParam, __in  LONG_PTR dwRefData);
HRESULT CreateRightClickMenu(HMENU * hMenu);
HRESULT GetPhysicalProcessorCount(UINT * Count);
//HRESULT GetThumbnail(LPCWSTR FileName, HBITMAP *phBitmap);