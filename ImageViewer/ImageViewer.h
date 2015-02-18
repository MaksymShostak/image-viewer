#pragma once

#include "resource.h"

struct COMMONITEMDIALOGOPEN
{
	HWND hWnd;
	LPCWSTR pszTitle;
	COMDLG_FILTERSPEC *rgFilterSpec;
	UINT cFileTypes;
	LPWSTR FileName;

	//COMMONITEMDIALOGOPEN() :
	//	cFileTypes(0U),
	//	rgFilterSpec(nullptr),
	//	FileName(nullptr),
	//	hWnd(nullptr),
	//	pszTitle(nullptr)
	//	{}
};

struct DELETEFILEWITHIFO
{
	HWND hWnd;
	LPCWSTR FileName;
	bool Permanent;
	bool Silent;

	//DELETEFILEWITHIFO() :
	//	hWnd(nullptr),
	//	FileName(nullptr),
	//	Permanent(false),
	//	Silent(false)
	//	{}
};

struct RENAMEFILEWITHIFO
{
	LPCWSTR FileName;
	LPCWSTR FileNameNew;

	//RENAMEFILEWITHIFO() :
	//	FileName(nullptr),
	//	FileNameNew(nullptr)
	//	{}
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
#define WINDOWCLASSSTRINGLENGTH 51
const WCHAR szWindowClass[WINDOWCLASSSTRINGLENGTH] = L"IMAGEVIEWER {445F6F25-065C-411D-B16E-A3E887660B76}"; // the main window class name
WCHAR g_FileName[MAX_PATH_UNICODE] = L"\0"; // L"C:\\Users\\Max\\Desktop\\Optimal_Colony_Layout.png";
WCHAR FileDirectory[MAX_PATH_UNICODE] = {0};
UINT g_FileNamePosition = 0U;
UINT FileNamePositionPrevious = 0U;
UINT FileNamePositionNext = 0U;
std::vector <LPWSTR> g_Directories;
std::vector <FILESTRUCT> g_Files;
HCURSOR hCursorArrow = nullptr;
HCURSOR hCursorHand = nullptr;
HCURSOR hCursorHandClosed = nullptr;
bool bFullscreen = false;
POINT DragStart = {0};
HANDLE hThreadCreateFileNameVectorFromDirectory = nullptr;
COMDLG_FILTERSPEC *FilterSpec = nullptr;
UINT cFileTypes = 0U;
LPWSTR *ArrayOfFileExtensions = nullptr;
UINT NumberOfFileExtensions = 0U;
HMENU hRightClickMenu = nullptr;
HMENU hRightClickMenuTitleBar = nullptr;
UINT g_NumberOfProcessors = 0;
bool g_BlockMovement = false;
COMMONITEMDIALOGOPEN g_commonitemdialogopen = {0};
DELETEFILEWITHIFO g_deletefilewithifo = {0};
extern UINT const DELAY_TIMER_ID = 1U;    // Global ID for the timer, only one timer is used
SORTBY g_SortByCurrent = SORTBYNAME;
bool g_SortByAscending = true;

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
UINT CountOccurencesOfCharacterInString(WCHAR character, LPCWSTR string);
	
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT				DeleteFileWithIFO(__in HWND hWnd, __in LPCWSTR FileName, __in bool Permanent, __in bool Silent);
HRESULT				RenameFileWithIFO(__in LPCWSTR FileName, __in LPCWSTR FileNameNew);
HRESULT				CommonItemDialogOpen(__in LPCWSTR pszTitle, __in COMDLG_FILTERSPEC *rgFilterSpec, __in UINT cFileTypes, __out LPWSTR FileName);
HRESULT				DirectoryFromFileName(__out LPWSTR FileDirectory, __in LPCWSTR FileName);
HRESULT				SetAsDesktopBackground(__in LPCWSTR FileName, __in DWORD dwStyle);
bool				NaturalSort(const std::wstring &lhs, const std::wstring &rhs);
unsigned __stdcall	CreateFileNameVectorFromDirectory(void* _ArgList);
BOOL CALLBACK		EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
HRESULT CALLBACK	TaskDialogCallbackProc(__in  HWND hWnd, __in  UINT uNotification, __in  WPARAM wParam, __in  LPARAM lParam, __in  LONG_PTR dwRefData);
HRESULT				CreateRightClickMenu(HMENU *hMenu);
HRESULT				GetPhysicalProcessorCount(UINT *Count);
HRESULT				GetThumbnail(LPCWSTR FileName, HBITMAP *phBitmap);