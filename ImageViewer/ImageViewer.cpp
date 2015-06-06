// ImageViewer.cpp : Defines the entry point for the application.
#include "stdafx.h"
#include "ImageViewer.h"
#include "Direct2DRenderer.h"
#include "HRESULT.h"
#include <VersionHelpers.h> //IsWindows7OrGreater

Direct2DRenderer renderer;

unsigned __stdcall DeleteFileWithIFO(void* _ArgList)
{
	unsigned result = 0U;

	DELETEFILEWITHIFO *deletefilewithifo = reinterpret_cast<DELETEFILEWITHIFO*>(_ArgList);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

	if (SUCCEEDED(hr))
    {
        // Create COM instance of IFileOperation
        Microsoft::WRL::ComPtr<IFileOperation> pfo;

        hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

        if (SUCCEEDED(hr))
        {
			DWORD dwOperationFlags = 0;

			dwOperationFlags = ((deletefilewithifo->Permanent) ? FOF_WANTNUKEWARNING : FOF_ALLOWUNDO) | ((deletefilewithifo->Silent) ? FOF_NOCONFIRMATION : 0);

			/*if (deletefilewithifo->Permanent)
			{
				dwOperationFlags = FOF_WANTNUKEWARNING;
			}
			else
			{
				dwOperationFlags = FOF_ALLOWUNDO;
			}

			if (deletefilewithifo->Silent)
			{
				dwOperationFlags = dwOperationFlags | FOF_NOCONFIRMATION;
			}*/

			hr = pfo->SetOwnerWindow(deletefilewithifo->hWnd);

			if (SUCCEEDED(hr))
            {
				// Set parameters for current operation
				hr = pfo->SetOperationFlags(dwOperationFlags);
 
				if (SUCCEEDED(hr))
				{
					// Create IShellItem instance associated to file to delete
					Microsoft::WRL::ComPtr<IShellItem> psiFileToDelete;

					hr = SHCreateItemFromParsingName(
							deletefilewithifo->FileName->c_str(),
							NULL,
							IID_PPV_ARGS(&psiFileToDelete)
							);
 
					if (SUCCEEDED(hr))
					{
							// Declare this shell item (file) to be deleted
							hr = pfo->DeleteItem(psiFileToDelete.Get(), NULL);
					}
				}
 
				if (SUCCEEDED(hr))
				{
					// Perform the deleting operation
					hr = pfo->PerformOperations(); // MSDN: "Note that if the operation was canceled by the user, this method can still return a success code" Emphasis on CAN

					if (SUCCEEDED(hr) || hr == 0x80270000) // turns out if you cancel the operation, it can sometimes return error code of 0x80270000 (ERROR|FACILITY_SHELL|0)
					{
						BOOL bAnyOperationsAborted = false;

						hr = pfo->GetAnyOperationsAborted(&bAnyOperationsAborted);

						if (SUCCEEDED(hr))
						{
							if (bAnyOperationsAborted)
							{
								hr = S_FALSE;
							}
						}
					}
				}
			}
        }
    }

    // Cleanup COM
    CoUninitialize(); // Must be called for each CoInitialize/CoInitializeEx
	
	if (SUCCEEDED(hr))
	{
		result = hr;
	}
	else
	{
		ErrorDescription(hr);
		result = (unsigned)-hr;
	}

	if (deletefilewithifo->hWnd)
	{
		if (!PostMessageW(deletefilewithifo->hWnd, WM_COMMAND, MAKEWPARAM(RETURNEDFROMDELETEFILEWITHIFO, result), NULL))
		{
			g_BlockMovement = false;
			ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

    // Return operation result
    return result;
};

unsigned int __stdcall RenameFileWithIFO(void* _ArgList)
{
	unsigned int result = 0U;

	RENAMEFILEWITHIFO *renamefilewithifo = reinterpret_cast<RENAMEFILEWITHIFO*>(_ArgList);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

	if (SUCCEEDED(hr))
    {
        // Create COM instance of IFileOperation
		Microsoft::WRL::ComPtr<IFileOperation> pfo = nullptr;

        hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

        if (SUCCEEDED(hr))
        {
			//DWORD dwOperationFlags = 0;

			//dwOperationFlags = ((deletefilewithifo->Permanent) ? FOF_WANTNUKEWARNING : FOF_ALLOWUNDO) | ((deletefilewithifo->Silent) ? FOF_NOCONFIRMATION : 0);

			/*if (deletefilewithifo->Permanent)
			{
				dwOperationFlags = FOF_WANTNUKEWARNING;
			}
			else
			{
				dwOperationFlags = FOF_ALLOWUNDO;
			}

			if (deletefilewithifo->Silent)
			{
				dwOperationFlags = dwOperationFlags | FOF_NOCONFIRMATION;
			}*/

			if (SUCCEEDED(hr))
            {
				// Set parameters for current operation
				hr = pfo->SetOperationFlags(FOF_NOCONFIRMATION | FOF_RENAMEONCOLLISION);
 
				if (SUCCEEDED(hr))
				{
					// Create IShellItem instance associated to file to rename
					Microsoft::WRL::ComPtr<IShellItem> psiFileToRename;

					hr = SHCreateItemFromParsingName(
							renamefilewithifo->FileName->c_str(),
							NULL,
							IID_PPV_ARGS(&psiFileToRename)
							);
 
					if (SUCCEEDED(hr))
					{
						// Declare this shell item (file) to be renamed
						hr = pfo->RenameItem(psiFileToRename.Get(), PathFindFileNameW(renamefilewithifo->FileNameNew->c_str()), NULL);
					}
				}
 
				if (SUCCEEDED(hr))
				{
					// Perform the renaming operation
					hr = pfo->PerformOperations(); // MSDN: "Note that if the operation was canceled by the user, this method can still return a success code" Emphasis on CAN

				//	if (SUCCEEDED(hr) || hr == 0x80270000) // turns out if you cancel the operation, it can sometimes return error code of 0x80270000 (ERROR|FACILITY_SHELL|0)
				//	{
				//		BOOL bAnyOperationsAborted = false;

				//		hr = pfo->GetAnyOperationsAborted(&bAnyOperationsAborted);
				//		if (SUCCEEDED(hr))
				//		{
				//			if (bAnyOperationsAborted)
				//			{
				//				hr = S_FALSE;
				//			}
				//		}
				//	}
				}
			}
        }
    }

    // Cleanup COM
    CoUninitialize(); // Must be called for each CoInitialize/CoInitializeEx
	
	if (SUCCEEDED(hr))
	{
		result = hr;
	}
	else
	{
		ErrorDescription(hr);
		result = (unsigned)-hr;
	}

    // Return operation result
    return result;
};

HRESULT RenameFileWithIFO(__in std::wstring * FileName, __in std::wstring * FileNameNew)
{
    // Check input parameter
    if (!FileName || !FileNameNew)
	{
		return E_POINTER;
	}
	
	RENAMEFILEWITHIFO renamefilewithifo;
	renamefilewithifo.FileName = FileName;
	renamefilewithifo.FileNameNew = FileNameNew;

	HANDLE hThreadRenameFileWithIFO = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(RENAMEFILEWITHIFO), // unsigned stack_size
			&RenameFileWithIFO, // unsigned ( __stdcall *start_address )( void * )
			&renamefilewithifo, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

	if (!hThreadRenameFileWithIFO)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (WAIT_FAILED == WaitForSingleObject(hThreadRenameFileWithIFO, INFINITE))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = E_FAIL;

	DWORD dword = 0;

	if (GetExitCodeThread(hThreadRenameFileWithIFO, &dword))
	{
		if (0 == dword)
		{
			hr = S_OK;
		}
		else if (1 == dword)
		{
			hr = S_FALSE;
		}
		else
		{
			hr = -(HRESULT)(dword);
		}
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!CloseHandle(hThreadRenameFileWithIFO))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

    return hr;
}

// Returns S_OK if deleted, S_FALSE if operation aborted
HRESULT DeleteFileWithIFO(__in HWND hWnd, __in std::wstring * FileName, __in bool Permanent, __in bool Silent) // IFileOperation::DeleteItem works only with Unicode UTF-16 strings.
{
    // Check input parameter
    if (!FileName)
	{
		return E_POINTER;
	}
	
	DELETEFILEWITHIFO deletefilewithifo;
	deletefilewithifo.hWnd = hWnd;
	deletefilewithifo.FileName = FileName;
	deletefilewithifo.Permanent = Permanent;
	deletefilewithifo.Silent = Silent;

	HANDLE hThreadDeleteFileWithIFO = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(DELETEFILEWITHIFO), // unsigned stack_size
			&DeleteFileWithIFO, // unsigned ( __stdcall *start_address )( void * )
			&deletefilewithifo, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

	if (!hThreadDeleteFileWithIFO)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (WaitForSingleObject(hThreadDeleteFileWithIFO, INFINITE) == WAIT_FAILED)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = E_FAIL;

	DWORD dword = 0;

	if (GetExitCodeThread(hThreadDeleteFileWithIFO, &dword))
	{
		if (0 == dword)
		{
			hr = S_OK;
		}
		else if (1 == dword)
		{
			hr = S_FALSE;
		}
		else
		{
			hr = -(HRESULT)(dword);
		}
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!CloseHandle(hThreadDeleteFileWithIFO))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

    return hr;
}

unsigned int __stdcall CommonItemDialogOpen(void* _ArgList)
{
	COMMONITEMDIALOGOPEN *commonitemdialogopen = reinterpret_cast<COMMONITEMDIALOGOPEN*>(_ArgList);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

	if (SUCCEEDED(hr))
    {
		Microsoft::WRL::ComPtr<IFileDialog> pfd;
		// CoCreate the dialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

		if (commonitemdialogopen->pszTitle)
		{
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetTitle(commonitemdialogopen->pszTitle->c_str());
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = pfd->SetFileTypes(commonitemdialogopen->cFileTypes, commonitemdialogopen->rgFilterSpec);

			if (SUCCEEDED(hr))
			{
				// Show the dialog
				hr = pfd->Show(commonitemdialogopen->hWnd);

				if (SUCCEEDED(hr))
				{
					Microsoft::WRL::ComPtr<IShellItem> psiResult = nullptr;
					// Obtain the result of the user's interaction with the dialog.
					hr = pfd->GetResult(&psiResult);

					if (SUCCEEDED(hr))
					{
						LPWSTR pszFileSysPath = nullptr;

						hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFileSysPath);

						if (SUCCEEDED(hr))
						{
							commonitemdialogopen->FileName->assign(pszFileSysPath);
							//SHAddToRecentDocs(SHARD_PATHW, pszFileSysPath);
							//SHAddToRecentDocs(SHARD_SHELLITEM, psiResult);
							CoTaskMemFree(pszFileSysPath);
						}
					}
				}
			}
		}
	}

    // Cleanup COM
    CoUninitialize(); // Must be called for each CoInitialize/CoInitializeEx
	
	if (commonitemdialogopen->hWnd)
	{
		if (SUCCEEDED(hr))
		{
			if (!PostMessageW(commonitemdialogopen->hWnd, WM_COMMAND, RETURNEDFROMCOMMONITEMDIALOGOPEN, NULL))
			{
				ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
			}
		}
		else
		{
			// if user closed Open dialog without selection, treat as normal exit
			if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) 
			{
				ErrorDescription(hr);
			}
		}
	}

	unsigned int result = 0U;

	if (SUCCEEDED(hr))
	{
		result = hr;
	}
	else
	{
		result = (unsigned)-hr;
	}

    // Return operation result
    return result;
};

HRESULT CommonItemDialogOpen(__in std::wstring * pszTitle, __in COMDLG_FILTERSPEC *rgFilterSpec, __in UINT cFileTypes, __out std::wstring * FileName)
{
	if (!FileName)
	{
		return E_POINTER;
	}

	// NULL hWnd as this wrapper blocks the hWnd thread - if want it to be associated with certain hWnd need to use the thread function directly
	COMMONITEMDIALOGOPEN commonitemdialogopen;

	commonitemdialogopen.hWnd = nullptr;
	if (pszTitle)
	{
		commonitemdialogopen.pszTitle = pszTitle;
	}
	commonitemdialogopen.rgFilterSpec = rgFilterSpec;
	commonitemdialogopen.cFileTypes = cFileTypes;
	commonitemdialogopen.FileName = FileName;

	HANDLE hThreadCommonItemDialogOpen = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(COMMONITEMDIALOGOPEN), // unsigned stack_size
			&CommonItemDialogOpen, // unsigned ( __stdcall *start_address )( void * )
			&commonitemdialogopen, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

	if (!hThreadCommonItemDialogOpen)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (WaitForSingleObject(hThreadCommonItemDialogOpen, INFINITE) == WAIT_FAILED)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = E_FAIL;

	DWORD dword = 0;

	if (GetExitCodeThread(hThreadCommonItemDialogOpen, &dword))
	{
		if (0 == dword)
		{
			hr = S_OK;
		}
		else
		{
			hr = -(HRESULT)(dword);
		}
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!CloseHandle(hThreadCommonItemDialogOpen))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

    return hr;
}

HRESULT DirectoryFromFileName(__out std::wstring * pFileDirectory, __in const wchar_t * filePath)
{
	// Check input parameters
    if (!filePath || !pFileDirectory)
	{
		return E_POINTER;
	}

	WCHAR pWSTR[MAX_PATH_UNICODE];

	// Copy the filePath string to a buffer
	HRESULT hr = StringCchCopyW(pWSTR, MAX_PATH_UNICODE, filePath);

	if (SUCCEEDED(hr))
	{
		// Remove the file name and extension
		hr = PathCchRemoveFileSpec(pWSTR, MAX_PATH_UNICODE);

		if (SUCCEEDED(hr))
		{
			pFileDirectory->assign(pWSTR);
		}
	}

	return hr;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int nCmdShow)
{
	MSG msg = { 0 };
	HACCEL hAccelTable = nullptr;

	// A correct application can continue to run even if this call fails, 
    // so it is safe to ignore the return value
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	//GetPhysicalProcessorCount(&g_NumberOfProcessors);

	/*WCHAR buffer[260];
	hr = StringCchPrintfW(buffer, 260, L"%d", NumberOfProcessors);
	if SUCCEEDED(hr)
	{
		MessageBoxW(NULL, buffer, L"Number of processors", MB_OK);
	}*/

	// Initialise multithreaded COM for Direct2D
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    if (FAILED(hr))
    {
		ErrorDescription(hr);
        return EXIT_FAILURE;
    }

	hr = renderer.CreateDeviceIndependentResources();
	if (FAILED(hr))
    {
		ErrorDescription(hr);
        return EXIT_FAILURE;
    }

	hr = renderer.EnumerateDecoders(nullptr, &cFileTypes);
	if (FAILED(hr))
    {
		ErrorDescription(hr);
        return EXIT_FAILURE;
    }

	FilterSpec = new COMDLG_FILTERSPEC[cFileTypes];

	hr = renderer.EnumerateDecoders(&FilterSpec, &cFileTypes);
	if (FAILED(hr))
    {
		ErrorDescription(hr);
        return EXIT_FAILURE;
    }

	std::wstring fullListOfExtensions(FilterSpec[0].pszSpec);

	NumberOfFileExtensions = static_cast<UINT>(CountOccurencesOfCharacterInString(L'.', &fullListOfExtensions));

	fullListOfExtensions.clear();

	ArrayOfFileExtensions = new LPWSTR[NumberOfFileExtensions];

	LPWSTR AllExtensions = nullptr;
	AllExtensions = new WCHAR[wcslen(FilterSpec[0].pszSpec) + 1];

	hr = StringCchCopyW(AllExtensions, wcslen(FilterSpec[0].pszSpec) + 1, FilterSpec[0].pszSpec);
	if (FAILED(hr))
    {
		ErrorDescription(hr);
        return EXIT_FAILURE;
    }

	WCHAR seps[] = L";";
	LPWSTR token = NULL;
	LPWSTR next_token = NULL;

	UINT CurrentFileExtensionNumber = 0U;

	token = wcstok_s(AllExtensions, seps, &next_token);

	while (token != NULL)
	{
		ArrayOfFileExtensions[CurrentFileExtensionNumber] = new WCHAR[wcslen((token) + 2) + 1];

		hr = StringCchCopyW(ArrayOfFileExtensions[CurrentFileExtensionNumber], wcslen((token) + 2) + 1, (token) + 2);
		if (FAILED(hr))
		{
			ErrorDescription(hr);
			return EXIT_FAILURE;
		}

		token = wcstok_s(NULL, seps, &next_token);
		CurrentFileExtensionNumber++;
	}

	delete [] AllExtensions;

	int	nArgs = 0;
	LPWSTR *lpszArgv = nullptr;
	lpszArgv = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if (lpszArgv == nullptr)
	{
		ErrorDescription(E_POINTER);
		return EXIT_FAILURE;
	}

	for (int i = 1U; i < nArgs; i++)
	{
		bool FileExtensionInList = false;

		LPWSTR suffix = wcsrchr(lpszArgv[i], L'.'); // Returns a pointer to the last occurrence of Ch in string, or NULL if Ch is not found.

		if (suffix)
		{
			suffix++;

			for (UINT j = 0U; j < NumberOfFileExtensions; j++)
			{
				if (0 == _wcsicmp(suffix, ArrayOfFileExtensions[j]))
				{
					FileExtensionInList = true;
					break;
				}
			}
		}
		
		if (FileExtensionInList)
		{
			if (0 == wcscmp(g_FileName.c_str(), L"\0"))
			{
				g_FileName = lpszArgv[i];

				if (g_FileName.empty())
				{
					return EXIT_FAILURE;
				}
				break;
			}
		}
	}

	//for (int i = 1U; i < nArgs; i++)
	//{
	//	bool FileExtensionInList = false;

	//	LPWSTR suffix = wcsrchr(lpszArgv[i], '.'); // Returns a pointer to the last occurrence of Ch in string, or NULL if Ch is not found.

	//	if (suffix)
	//	{
	//		suffix++;

	//		for (UINT j = 0U; j < NumberOfFileExtensions; j++)
	//		{
	//			if (_wcsicmp(suffix, ArrayOfFileExtensions[j]) == 0)
	//			{
	//				FileExtensionInList = true;
	//				break;
	//			}
	//		}
	//	}
	//	
	//	if (FileExtensionInList)
	//	{
	//		if (wcscmp(g_FileName, L"\0") == 0)
	//		{
	//			hr = StringCchCopyW(g_FileName, MAX_PATH_UNICODE, lpszArgv[i]);
	//			if (FAILED(hr))
	//			{
	//				ErrorDescription(hr);
	//				return EXIT_FAILURE;
	//			}
	//		}
	//		else
	//		{
	//			FileNames.push_back(lpszArgv[i]);
	//		}
	//	}
	//}

	//if (FileNames.size() > 0) // if there are more than 1 valid files, add the original back
	//{
	//	FileNames.push_back(g_FileName);

	//	std::sort(FileNames.begin(), FileNames.end(), &NaturalSort);

	//	for (UINT i = 0U; i < FileNames.size(); i++)
	//	{
	//		if (wcscmp(g_FileName, FileNames[i].c_str()) == 0)
	//		{
	//			g_FileNamePosition = i;
	//			break;
	//		}
	//	}
	//}

	//if (nArgs > 1) // i.e. if argument is present
	//{
	//	bool FileExtensionInList = false;

	//	LPWSTR suffix = wcsrchr(lpszArgv[1], '.'); // Returns a pointer to the last occurrence of Ch in string, or NULL if Ch is not found.

	//	if (suffix)
	//	{
	//		suffix++;

	//		for (UINT i = 0U; i < NumberOfFileExtensions; i++)
	//		{
	//			if (_wcsicmp(suffix, ArrayOfFileExtensions[i]) == 0)
	//			{
	//				FileExtensionInList = true;
	//				break;
	//			}
	//		}
	//	}
	//	
	//	if (FileExtensionInList)
	//	{
	//		hr = StringCchCopyW(g_FileName, MAX_PATH_UNICODE, lpszArgv[1]); // use only first argument
	//		if (FAILED(hr))
	//		{
	//			ErrorDescription(hr);
	//			return EXIT_FAILURE;
	//		}
	//	}

	//	//SHAddToRecentDocs(SHARD_PATHW, g_FileName);
	//}

	if (LocalFree(lpszArgv)) // If the function succeeds, the return value is NULL. If the function fails, the return value is equal to a handle to the local memory object.
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	hCursorArrow = static_cast<HCURSOR>(LoadImageW(
		NULL, // __in_opt HINSTANCE hinst
#pragma warning(push)
#pragma warning(disable:4302)
		MAKEINTRESOURCEW(IDC_ARROW), // __in LPCTSTR lpszName
#pragma warning(pop)
		IMAGE_CURSOR, // __in UINT uType
		0, // __in int cxDesired
		0, // __in int cyDesired
		LR_DEFAULTSIZE | LR_SHARED // __in UINT fuLoad
		));

	if (!hCursorArrow)
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	//hCursorHand = (HCURSOR)LoadImageW(
	//	NULL, // __in_opt HINSTANCE hinst
	//	MAKEINTRESOURCEW(IDC_HAND), // __in LPCTSTR lpszName
	//	IMAGE_CURSOR, // __in UINT uType
	//	0, // __in int cxDesired
	//	0, // __in int cyDesired
	//	LR_DEFAULTSIZE | LR_SHARED // __in UINT fuLoad
	//   );
	//if (!hCursorHand)
	//{
	//	ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
	//	return EXIT_FAILURE;
	//}

	hCursorHand = static_cast<HCURSOR>(LoadImageW(
		hInstance, // __in_opt HINSTANCE hinst
		MAKEINTRESOURCEW(IDC_HANDOPEN), // __in LPCTSTR lpszName
		IMAGE_CURSOR, // __in UINT uType
		0, // __in int cxDesired
		0, // __in int cyDesired
		LR_DEFAULTSIZE | LR_SHARED // __in UINT fuLoad
	   ));

	if (!hCursorHand)
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	hCursorHandClosed = static_cast<HCURSOR>(LoadImageW(
		hInstance, // __in_opt HINSTANCE hinst
		MAKEINTRESOURCEW(IDC_HANDCLOSED), // __in LPCTSTR lpszName
		IMAGE_CURSOR, // __in UINT uType
		0, // __in int cxDesired
		0, // __in int cyDesired
		LR_DEFAULTSIZE | LR_SHARED // __in UINT fuLoad
	   ));

	if (!hCursorHandClosed)
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(IDC_IMAGEVIEWER));

	if (!hAccelTable)
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		return EXIT_FAILURE;
	}

	// Main message loop:
	while (GetMessageW(&msg, NULL, 0U, 0U))
	{
		if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	// Ignore return value
	CloseHandle(hThreadCreateFileNameVectorFromDirectory);

	// do not delete the strings in each element as this is taken care of by the destructor of Direct2DRenderer (where the strings are owned)
	//delete [] FilterSpec;

	/*for (UINT i = 0U; i < NumberOfFileExtensions; i++)
	{
		delete [] ArrayOfFileExtensions[i];
	}
	delete [] ArrayOfFileExtensions;*/

	/*for (UINT i = 0U; i < g_Files.size(); i++)
	{
		DeleteObject(g_Files[i].Thumbnail);
	}*/

	if (hRightClickMenu)
	{
		// Ignore return value
		DestroyMenu(hRightClickMenu);
	}

	if (hRightClickMenuTitleBar)
	{
		// Ignore return value
		DestroyMenu(hRightClickMenuTitleBar);
	}

	CoUninitialize(); // Uninitialise COM for Direct2D

	return static_cast<int>(msg.wParam);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_IMAGEVIEWER), IMAGE_ICON, 0, 0, 0U); // LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_IMAGEVIEWER));
	wcex.hCursor		= hCursorArrow;
	wcex.hbrBackground	= NULL; // (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; // MAKEINTRESOURCE(IDC_IMAGEVIEWER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_SMALL)); // (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SMALL), IMAGE_ICON, 0, 0, 0U);

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int /*nCmdShow*/)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowExW(
		0L,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_VISIBLE,
		CW_USEDEFAULT,
		SW_MAXIMIZE,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// required because when returns from CommonItemDialogOpen, does not make foreground window (as can't pass hWnd to that function as it is in another thread)
	// ignore return value
	SetForegroundWindow(hWnd);

	//ShowWindow(hWnd, nCmdShow);
	/*if (!UpdateWindow(hWnd))
	{
		return FALSE;
	}*/

	return TRUE;
}

BOOL SystemTimeToVariantTimeWithMilliseconds(__in SYSTEMTIME st, __out double *dVariantTime)
{
    BOOL retVal = TRUE;

    WORD wMilliSeconds = st.wMilliseconds; // save the milli second information

    st.wMilliseconds = 0; // pass 0 milliseconds to the function and get the converted value without milliseconds

    double dWithoutms;
    retVal = SystemTimeToVariantTime(&st, &dWithoutms) ;

    // manually convert the millisecond information into variant fraction and add it to system converted value
	/*A variant time is stored as an 8-byte real value (double), representing a date between January 1, 1753 
	and December 31, 2078,inclusive. The value 2.0 represents January 1, 1900; 3.0 represents January 2, 1900, and so on. 
	Adding 1 to the value increments the date by a day. The fractional part of the value represents the time of day. 
	Therefore, 2.5 represents noon on January 1, 1900; 3.25 represents 6:00 A.M. on January 2, 1900, and so on. 
	so 0.5 represents 12 hours ie 12*60*60 seconds, hence 1 second = .0000115740740740*/
    double OneMilliSecond = 0.0000115740740740/1000;
    *dVariantTime = dWithoutms + (OneMilliSecond * wMilliSeconds);

    return retVal;
}

bool FilesSortByDateModified(FILESTRUCT &lhs, FILESTRUCT &rhs)
{
	double VariantTimeLHS = 0.0;
	SystemTimeToVariantTimeWithMilliseconds(lhs.DateModified, &VariantTimeLHS);

	double VariantTimeRHS = 0.0;
	SystemTimeToVariantTimeWithMilliseconds(rhs.DateModified, &VariantTimeRHS);
	
	return (VariantTimeLHS < VariantTimeRHS);
}

bool FilesSortBySize(FILESTRUCT &lhs, FILESTRUCT &rhs)
{
	return (lhs.SizeInBytes < rhs.SizeInBytes);
}

bool FilesSortByNameNatural(FILESTRUCT &lhs, FILESTRUCT &rhs)
{
	return (StrCmpLogicalW(lhs.FullPath.c_str(), rhs.FullPath.c_str()) < 0);
}

bool NaturalSort(LPCWSTR &lhs, LPCWSTR &rhs)
{
    return (StrCmpLogicalW(lhs, rhs) < 0);
}

//bool NaturalSort(const std::wstring &lhs, const std::wstring &rhs)
//{
//    return (StrCmpLogicalW(lhs.c_str(), rhs.c_str()) < 0);
//}

struct FILENAMEVECTORFROMDIRECTORY
{
	std::vector<FILESTRUCT> * Files;
	std::wstring * Directory;
	LPWSTR *ArrayOfFileExtensions;
	UINT NumberOfFileExtensions;
};

bool PathEndsInSlash(std::wstring * pPath)
{
	if (L'\\' == pPath->back())
	{
		return true;
	}

	return false;
}

unsigned __stdcall CreateFileNameVectorFromDirectory(void* _ArgList)
{
	unsigned result = 0U;

	FILENAMEVECTORFROMDIRECTORY FileNameVectorFromDirectory = *((FILENAMEVECTORFROMDIRECTORY*)_ArgList);

	WCHAR szDir[MAX_PATH_UNICODE] = {0};
	HANDLE hFind = INVALID_HANDLE_VALUE;
	_WIN32_FIND_DATAW ffd = {0};
	WCHAR wFullFileName[MAX_PATH_UNICODE] = {0};
	DWORD ShowHiddenFilesFoldersAndDrives = 2; // 2 means don't show
	DWORD ShowProtectedOperatingSystemFiles = 0;
	DWORD dwSize = sizeof(DWORD);
	UINT ID = 0U;
	FILETIME FileTimeModfied = {0};
	SYSTEMTIME SystemTimeModified = {0};
	FILESTRUCT File = {0};
	UINT NumberCharsInFullFileName = 0U;
	bool DirectoryIsRoot = PathEndsInSlash(FileNameVectorFromDirectory.Directory) ? true : false;

	HKEY Advanced = nullptr;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 0, KEY_QUERY_VALUE | STANDARD_RIGHTS_READ, &Advanced) == ERROR_SUCCESS)
	{
		RegQueryValueExW(Advanced, L"Hidden", NULL, NULL, (LPBYTE)&ShowHiddenFilesFoldersAndDrives, &dwSize);

		RegQueryValueExW(Advanced, L"ShowSuperHidden", NULL, NULL, (LPBYTE)&ShowProtectedOperatingSystemFiles, &dwSize);

		RegCloseKey(Advanced);
	}

	//To extend this limit to 32,767 wide characters, call the Unicode version of the function and prepend "\\?\" to the path
	HRESULT hr = StringCchCatW(szDir, MAX_PATH_UNICODE, L"\\\\?\\");
	if (SUCCEEDED(hr))
	{
		// Copy the FileName string to a buffer
		// TODO: Fix for long file paths and take network share paths into account https://msdn.microsoft.com/en-us/library/aa365247.aspx
		hr = StringCchCopyW(szDir, MAX_PATH_UNICODE, FileNameVectorFromDirectory.Directory->c_str());
	}

	if (SUCCEEDED(hr))
	{
		// Append '\*' to the directory name
		hr = StringCchCatW(szDir, MAX_PATH_UNICODE, DirectoryIsRoot ? L"*" : L"\\*");
	}

	if (SUCCEEDED(hr))
	{
		DWORD dwAdditionalFlags = 0;
		if (IsWindows7OrGreater())
		{
			dwAdditionalFlags = FIND_FIRST_EX_LARGE_FETCH;
		}

		hFind = FindFirstFileExW(
			szDir, // LPCTSTR lpFileName
			FindExInfoBasic, // FINDEX_INFO_LEVELS fInfoLevelId
			&ffd, // LPVOID lpFindFileData
			FindExSearchNameMatch, //FINDEX_SEARCH_OPS fSearchOp
			NULL, //LPVOID lpSearchFilter
			dwAdditionalFlags // DWORD dwAdditionalFlags
			);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			return (unsigned)-hr;
		}

		// Put all matching files in vector
		do
		{
			// If not directory
			if (!(ffd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY))
				&& ((ShowHiddenFilesFoldersAndDrives == 1) || !(ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
				&& ((ShowProtectedOperatingSystemFiles == 1) || !(ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
				)
			{
				LPWSTR suffix = wcsrchr(ffd.cFileName, L'.'); // Returns a pointer to the last occurrence of Ch in string, or NULL if Ch is not found.
				
				if (suffix)
				{
					suffix++;

					for (UINT i = 0U; i < FileNameVectorFromDirectory.NumberOfFileExtensions; i++)
					{
						if (_wcsicmp(suffix, FileNameVectorFromDirectory.ArrayOfFileExtensions[i]) == 0)
						{
							hr = StringCchCopyW(wFullFileName, MAX_PATH_UNICODE, FileDirectory.c_str());
							if (SUCCEEDED(hr))
							{
								if (!DirectoryIsRoot)
								{
									hr = StringCchCatW(wFullFileName, MAX_PATH_UNICODE, L"\\");
								}

								if (SUCCEEDED(hr))
								{
									hr = StringCchCatW(wFullFileName, MAX_PATH_UNICODE, ffd.cFileName);
									if (SUCCEEDED(hr))
									{
										//GetThumbnail(wFullFileName, &File.Thumbnail);
										//File.Thumbnail = nullptr;

										FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &FileTimeModfied);
																				
										FileTimeToSystemTime(&FileTimeModfied,&SystemTimeModified);

										NumberCharsInFullFileName = static_cast<UINT>(wcsnlen(wFullFileName, MAX_PATH_UNICODE) + 1); // add one to account for terminating null

										LPWSTR FullFileName = nullptr;
										FullFileName = new WCHAR[NumberCharsInFullFileName];

										StringCchCopyW(FullFileName, NumberCharsInFullFileName, wFullFileName);

										File.ID = ID;
										File.FullPath = FullFileName;

										File.SizeInBytes = (((unsigned long long)ffd.nFileSizeHigh) << 32) + ffd.nFileSizeLow;

										File.DateModified = SystemTimeModified;

										FileNameVectorFromDirectory.Files->push_back(File);

										if (g_FileNamePosition == 0U && (wcscmp(g_FileName.c_str(), wFullFileName) == 0)) // will keep comparing if fileposition is legitimately 0
										{
											g_FileNamePosition = ID;
										}

										ID++;

										//FileNameVectorFromDirectory.FileNameVector->push_back(wFullFileName);

										break;
									}
								}
							}
						}
					}
				}
			}
		}
		while (FindNextFile(hFind, &ffd));

		DWORD dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			ErrorDescription(HRESULT_FROM_WIN32(dwError));
		}

		if (!FindClose(hFind))
		{
			ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	std::sort(FileNameVectorFromDirectory.Files->begin(), FileNameVectorFromDirectory.Files->end(), &FilesSortByNameNatural); // always sort by name if in new directory
	g_SortByCurrent = SORTBYNAME;

	/*switch (g_SortByCurrent)
	{
	case SORTBYDATEMODIFIED:
		{
			std::sort(FileNameVectorFromDirectory.Files->begin(), FileNameVectorFromDirectory.Files->end(), &FilesSortByDateModified);
		}
		break;
	case SORTBYNAME:
		{
			std::sort(FileNameVectorFromDirectory.Files->begin(), FileNameVectorFromDirectory.Files->end(), &FilesSortByNameNatural);
		}
		break;
	case SORTBYSIZE:
		{
			std::sort(FileNameVectorFromDirectory.Files->begin(), FileNameVectorFromDirectory.Files->end(), &FilesSortBySize);
		}
		break;
	}*/

	for (UINT i = 0U; i < FileNameVectorFromDirectory.Files->size(); i++)
	{
		if (FileNameVectorFromDirectory.Files->at(i).ID == g_FileNamePosition)
		{
			g_FileNamePosition = i;
			break;
		}
	}

	/*for (UINT i = 0U; i < FileNameVectorFromDirectory.FileNameVector->size(); i++)
	{
		if (wcscmp(g_FileName, (FileNameVectorFromDirectory.FileNameVector->at(i)).c_str()) == 0)
		{
			g_FileNamePosition = i;
			break;
		}
	}*/

	/*std::vector <std::wstring>::iterator Position = std::find(FileNames->begin(), FileNames->end(), g_FileName);
	g_FileNamePosition = Position - FileNames->begin();*/

	if (SUCCEEDED(hr))
	{
		result = 0U;
	}
	else
	{
		result = (unsigned)-hr;
	}

    // Return operation result
    return result;
}

HRESULT CreateRightClickMenu(HMENU *hMenu)
{
	HRESULT hr = S_OK;

	if (!*hMenu)
	{
		*hMenu = CreatePopupMenu();
		if (*hMenu)
		{
			HMENU hSortBydMenu = CreatePopupMenu();
			if (hSortBydMenu)
			{
				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
				mii.fType = MFT_RADIOCHECK | MFT_STRING;

				mii.wID = ID_FILE_SORTBYDATEMODIFIED;
				mii.dwTypeData = L"Date modified";
				mii.cch = static_cast<UINT>(wcslen(mii.dwTypeData) + 1);
				mii.fState = g_SortByCurrent == SORTBYDATEMODIFIED ? MFS_CHECKED : MFS_UNCHECKED;
			
				InsertMenuItemW(hSortBydMenu, (UINT)-1, TRUE, &mii);

				mii.wID = ID_FILE_SORTBYNAME;
				mii.dwTypeData = L"Name";
				mii.cch = static_cast<UINT>(wcslen(mii.dwTypeData) + 1);
				mii.fState = g_SortByCurrent == SORTBYNAME ? MFS_CHECKED : MFS_UNCHECKED;
			
				InsertMenuItemW(hSortBydMenu, (UINT)-1, TRUE, &mii);

				mii.wID = ID_FILE_SORTBYSIZE;
				mii.dwTypeData = L"Size";
				mii.cch = static_cast<UINT>(wcslen(mii.dwTypeData) + 1);
				mii.fState = g_SortByCurrent == SORTBYSIZE ? MFS_CHECKED : MFS_UNCHECKED;
			
				InsertMenuItemW(hSortBydMenu, (UINT)-1, TRUE, &mii);

				InsertMenuW(hSortBydMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);

				mii.wID = ID_FILE_SORTBYASCENDING;
				mii.dwTypeData = L"Ascending";
				mii.cch = static_cast<UINT>(wcslen(mii.dwTypeData) + 1);
				mii.fState = g_SortByAscending ? MFS_CHECKED : MFS_UNCHECKED;
			
				InsertMenuItemW(hSortBydMenu, (UINT)-1, TRUE, &mii);

				mii.wID = ID_FILE_SORTBYDESCENDING;
				mii.dwTypeData = L"Descending";
				mii.cch = static_cast<UINT>(wcslen(mii.dwTypeData) + 1);
				mii.fState = g_SortByAscending ? MFS_UNCHECKED : MFS_CHECKED;
			
				InsertMenuItemW(hSortBydMenu, (UINT)-1, TRUE, &mii);
			
				InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)hSortBydMenu, L"Sort by");
				InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
			}

			HMENU hSetAsDesktopBackgroundMenu = CreatePopupMenu();
			if (hSetAsDesktopBackgroundMenu)
			{
				InsertMenuW(hSetAsDesktopBackgroundMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_SETASDESKTOPBACKGROUND_CENTER, L"Center");
				if (IsWindows7OrGreater())
				{
					InsertMenuW(hSetAsDesktopBackgroundMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_SETASDESKTOPBACKGROUND_CROPTOFIT, L"Crop to fit");
					InsertMenuW(hSetAsDesktopBackgroundMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_SETASDESKTOPBACKGROUND_KEEPASPECT, L"Keep aspect");
				}
				InsertMenuW(hSetAsDesktopBackgroundMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_SETASDESKTOPBACKGROUND_STRETCH, L"Stretch");
				InsertMenuW(hSetAsDesktopBackgroundMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_SETASDESKTOPBACKGROUND_TILE, L"Tile");

				InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)hSetAsDesktopBackgroundMenu, L"Set as desktop background");
				InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
			}

			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_OPENFILELOCATION, L"Open file location");
			//InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
			//InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_AUTOROTATE, L"Auto rotate");
			//InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_ROTATECLOCKWISE, L"Rotate clockwise");
			//InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_ROTATECOUNTERCLOCKWISE, L"Rotate counterclockwise");
			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_COPY, L"Copy");
			//InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_CUT, L"Cut*");
			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_DELETE, L"Delete");
			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
			InsertMenuW(*hMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, ID_FILE_PROPERTIES, L"Properties");

			hr = S_OK;
		}
	}
	
	return hr;
}

HRESULT CreateRightClickMenuTitleBar(HMENU * hMenu, LPCWSTR filePath, std::vector<std::wstring> * directories)
{
	// Initialise to S_OK so that we return 'successful' if the hMenu already exists
	HRESULT hr = S_OK;

	if (!*hMenu)
	{
		*hMenu = CreatePopupMenu();

		hr = *hMenu ? S_OK : HRESULT_FROM_WIN32(GetLastError());

		if (SUCCEEDED(hr))
		{
			MENUINFO MenuInfo = { 0 };
			MenuInfo.cbSize = sizeof(MENUINFO);
			MenuInfo.fMask = MIM_STYLE;
			MenuInfo.dwStyle = MNS_NOTIFYBYPOS;

			hr = SetMenuInfo(*hMenu, &MenuInfo) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

			if (SUCCEEDED(hr))
			{
				WCHAR filePathTemp[MAX_PATH_UNICODE] = { 0 };

				hr = StringCchCopyW(filePathTemp, MAX_PATH_UNICODE, filePath);

				while (SUCCEEDED(hr))
				{
					hr = PathCchRemoveFileSpec(filePathTemp, MAX_PATH_UNICODE);

					// If PathCchRemoveFileSpec returns S_FALSE there was nothing to remove
					if (S_FALSE == hr)
					{
						// so return from the loop
						break;
					}

					if (SUCCEEDED(hr))
					{
						directories->push_back(filePathTemp);

						MENUITEMINFO mii = { 0 };
						mii.cbSize = sizeof(MENUITEMINFO);
						mii.fMask = MIIM_BITMAP | MIIM_FTYPE | MIIM_STRING;
						mii.fType = MFT_STRING;
						mii.dwTypeData = (PathCchIsRoot(filePathTemp) ? filePathTemp : wcsrchr(filePathTemp, L'\\') + 1);
						mii.cch = static_cast<UINT>(wcslen(filePathTemp));

						Microsoft::WRL::ComPtr<IShellItemImageFactory> pShellItemImageFactory;

						if (SUCCEEDED(SHCreateItemFromParsingName(filePathTemp, NULL, IID_PPV_ARGS(&pShellItemImageFactory))))
						{
							SIZE sizeOfIcon = { GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON) };

							// Ignore return value
							pShellItemImageFactory->GetImage(
								sizeOfIcon,
								SIIGBF_RESIZETOFIT,
								&mii.hbmpItem);
						}

						if (1U == directories->size())
						{
							mii.fMask = mii.fMask | MIIM_STATE;
							mii.fState = MFS_DEFAULT;
						}

						hr = InsertMenuItemW(*hMenu, UINT_MAX, TRUE, &mii) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
					}
				}
			}
		}
	}

	return hr;
}

struct SETASDESKTOPBACKGROUND
{
	LPCWSTR Filename;
	DWORD dwStyle;
};

unsigned int __stdcall SetAsDesktopBackground(void* _ArgList)
{
	SETASDESKTOPBACKGROUND setasdesktopbackground = *(reinterpret_cast<SETASDESKTOPBACKGROUND*>(_ArgList));

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

	if (SUCCEEDED(hr))
    {
		Microsoft::WRL::ComPtr<IActiveDesktop> iADesktop;

		hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_ALL, IID_PPV_ARGS(&iADesktop));

		if (SUCCEEDED(hr))
		{
			WALLPAPEROPT wOption;

			ZeroMemory(&wOption, sizeof(WALLPAPEROPT));
			wOption.dwSize = sizeof(WALLPAPEROPT);

			wOption.dwStyle = setasdesktopbackground.dwStyle;

			hr = iADesktop->SetWallpaper(setasdesktopbackground.Filename, 0);

			if (SUCCEEDED(hr))
			{
				hr = iADesktop->SetWallpaperOptions(&wOption, 0);

				if (SUCCEEDED(hr))
				{
					hr = iADesktop->ApplyChanges(AD_APPLY_ALL);
				}
			}
		}
	}

    // Cleanup COM
    CoUninitialize(); // Must be called for each CoInitialize/CoInitializeEx
	
	unsigned int result = 0U;

	if (SUCCEEDED(hr))
	{
		result = 0;
	}
	else
	{
		result = (unsigned)-hr;
	}

    // Return operation result
    return result;
};

HRESULT SetAsDesktopBackground(__in std::wstring * FileName, __in DWORD dwStyle)
{	
	SETASDESKTOPBACKGROUND setasdesktopbackground = {FileName->c_str(), dwStyle};

	HANDLE hThreadSetAsDesktopBackground = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(SETASDESKTOPBACKGROUND), // unsigned stack_size
			&SetAsDesktopBackground, // unsigned ( __stdcall *start_address )( void * )
			&setasdesktopbackground, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

	if (!hThreadSetAsDesktopBackground)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (WAIT_FAILED == WaitForSingleObject(hThreadSetAsDesktopBackground, INFINITE))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = E_FAIL;

	DWORD dword = 0;

	if (GetExitCodeThread(hThreadSetAsDesktopBackground, &dword))
	{
		if (0 == dword)
		{
			hr = S_OK;
		}
		else
		{
			hr = -(HRESULT)(dword);
		}
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!CloseHandle(hThreadSetAsDesktopBackground))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

    return hr;
}

void _OnCommand(HWND hWnd, int id, HWND /*hwndCtl*/, UINT codeNotify)
{
	// Parse the menu selections:
	switch(id)
	{
	case ID_FILE_ACTUALSIZE:
		{
			_OnCommand_ID_FILE_ACTUALSIZE(hWnd);
		}
		break;
	case ID_FILE_ANIMATIONSTARTSTOP:
		{
			_OnCommand_ID_FILE_ANIMATIONSTARTSTOP(hWnd);
		}
		break;
	case ID_FILE_AUTOROTATE:
		{
			_OnCommand_ID_FILE_AUTOROTATE(hWnd);
		}
		break;
	case ID_FILE_CLOSEALLWINDOWS:
		{
			_OnCommand_ID_FILE_CLOSEALLWINDOWS(hWnd);
		}
		break;
	case ID_FILE_COPY:
		{
			_OnCommand_ID_FILE_COPY(hWnd);
		}
		break;
	case ID_FILE_CUT:
		{
			_OnCommand_ID_FILE_CUT(hWnd);
		}
		break;
	case ID_FILE_DELETE:
		{
			_OnCommand_ID_FILE_DELETE(hWnd);
		}
		break;
	case ID_FILE_DELETEPERMANENTLY:
		{
			_OnCommand_ID_FILE_DELETEPERMANENTLY(hWnd);
		}
		break;
	case ID_FILE_EXIT:
		{
			_OnCommand_ID_FILE_EXIT(hWnd);
		}
		break;
	case ID_FILE_FIRSTFILE:
		{
			_OnCommand_ID_FILE_FIRSTFILE(hWnd);
		}
		break;
	case ID_FILE_FITTOWINDOW:
		{
			_OnCommand_ID_FILE_FITTOWINDOW(hWnd);
		}
		break;
	case ID_FILE_FRAMENEXT:
		{
			_OnCommand_ID_FILE_FRAMENEXT(hWnd);
		}
		break;
	case ID_FILE_FRAMEPREVIOUS:
		{
			_OnCommand_ID_FILE_FRAMEPREVIOUS(hWnd);
		}
		break;
	case ID_FILE_FULLSCREEN:
		{
			_OnCommand_ID_FILE_FULLSCREEN(hWnd);
		}
		break;
	case ID_FILE_LASTFILE:
		{
			_OnCommand_ID_FILE_LASTFILE(hWnd);
		}
		break;
	case ID_FILE_NEW:
		{
			_OnCommand_ID_FILE_NEW(hWnd);
		}
		break;
	case ID_FILE_NEXT:
		{
			_OnCommand_ID_FILE_NEXT(hWnd);
		}
		break;
	case ID_FILE_OPEN:
		{
			_OnCommand_ID_FILE_OPEN(hWnd);
		}
		break;
	case ID_FILE_OPENFILELOCATION:
		{
			_OnCommand_ID_FILE_OPENFILELOCATION(hWnd);
		}
		break;
	case ID_FILE_PREVIOUS:
		{
			_OnCommand_ID_FILE_PREVIOUS(hWnd);
		}
		break;
	case ID_FILE_PROPERTIES:
		{
			_OnCommand_ID_FILE_PROPERTIES(hWnd);
		}
		break;
	case ID_FILE_ROTATECLOCKWISE:
		{
			_OnCommand_ID_FILE_ROTATECLOCKWISE(hWnd);
		}
		break;
	case ID_FILE_ROTATECOUNTERCLOCKWISE:
		{
			_OnCommand_ID_FILE_ROTATECOUNTERCLOCKWISE(hWnd);
		}
		break;
	case ID_FILE_SCALETOWINDOW:
		{
			_OnCommand_ID_FILE_SCALETOWINDOW(hWnd);
		}
		break;
	case ID_FILE_SETASDESKTOPBACKGROUND_CENTER:
		{
			_OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CENTER(hWnd);
		}
		break;
	case ID_FILE_SETASDESKTOPBACKGROUND_CROPTOFIT:
		{
			_OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CROPTOFIT(hWnd);
		}
		break;
	case ID_FILE_SETASDESKTOPBACKGROUND_KEEPASPECT:
		{
			_OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_KEEPASPECT(hWnd);
		}
		break;
	case ID_FILE_SETASDESKTOPBACKGROUND_STRETCH:
		{
			_OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_STRETCH(hWnd);
		}
		break;
	case ID_FILE_SETASDESKTOPBACKGROUND_TILE:
		{
			_OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_TILE(hWnd);
		}
		break;
	case ID_FILE_SORTBYASCENDING:
		{
			_OnCommand_ID_FILE_SORTBYASCENDING(hWnd);
		}
		break;
	case ID_FILE_SORTBYDATEMODIFIED:
		{
			_OnCommand_ID_FILE_SORT(hWnd, SORTBYDATEMODIFIED);
		}
		break;
	case ID_FILE_SORTBYDESCENDING:
		{
			_OnCommand_ID_FILE_SORTBYDESCENDING(hWnd);
		}
		break;
	case ID_FILE_SORTBYNAME:
		{
			_OnCommand_ID_FILE_SORT(hWnd, SORTBYNAME);
		}
		break;
	case ID_FILE_SORTBYSIZE:
		{
			_OnCommand_ID_FILE_SORT(hWnd, SORTBYSIZE);
		}
		break;
	case ID_FILE_TOGGLEBACKGROUNDCOLOR:
		{
			_OnCommand_ID_FILE_TOGGLEBACKGROUNDCOLOR(hWnd);
		}
		break;
	case ID_HELP_ABOUT:
		{
			_OnCommand_ID_HELP_ABOUT(hWnd);
		}
		break;
	case RETURNEDFROMCOMMONITEMDIALOGOPEN:
		{
			_OnCommand_RETURNEDFROMCOMMONITEMDIALOGOPEN(hWnd);
		}
		break;
	case RETURNEDFROMDELETEFILEWITHIFO:
		{
			_OnCommand_RETURNEDFROMDELETEFILEWITHIFO(hWnd, codeNotify);
		}
		break;
	}
}

void _OnCommand_ID_FILE_ACTUALSIZE(HWND /*hWnd*/)
{
	HRESULT hr = renderer.ActualSize();

	if (SUCCEEDED(hr))
	{
		SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
	}
}

void _OnCommand_ID_FILE_ANIMATIONSTARTSTOP(HWND /*hWnd*/)
{
	renderer.OnAnimationStartStop();
}

void _OnCommand_ID_FILE_AUTOROTATE(HWND /*hWnd*/)
{
	if (renderer.RotateAutoEnabled())
	{
		if (GetKeyState(VK_CONTROL) & 0x8000) // CTRL key is down.
		{
			ErrorDescription(HRESULT_FROM_WIN32(0));
			/*for (UINT i = 0U; i < g_Files.size(); i++)
			{
				Direct2DRenderer::RotateJPEG(g_Files[i].FullPath, LPCWSTR FileNameTemporary, USHORT RotationFlag, bool Clockwise)

				renderer.Rotate(true);
			}*/
		}
		renderer.Rotate(true);
	}
	else
	{
		MessageBeep(MB_ICONWARNING);
	}
}

void _OnCommand_ID_FILE_CLOSEALLWINDOWS(HWND /*hWnd*/)
{
	if (!EnumWindows(EnumWindowsProc, MAKELPARAM(WM_CLOSE, 0)))
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void _OnCommand_ID_FILE_COPY(HWND hWnd)
{
	WCHAR buffer[MAX_PATH_UNICODE] = {0};

	HRESULT hr = StringCchCopyW(buffer, MAX_PATH_UNICODE, g_Files[g_FileNamePosition].FullPath.c_str());

	if SUCCEEDED(hr)
	{
		// Append null character to the buffer as HDROP string needs to be double null terminated
		hr = StringCchCatW(buffer, MAX_PATH_UNICODE, L"\0");

		if SUCCEEDED(hr)
		{
 			if (OpenClipboard(hWnd))
			{
				if (EmptyClipboard())
				{
					int nSize = sizeof(DROPFILES) + sizeof(buffer);

					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nSize);

					if (hData != NULL)
					{
						LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(hData);
						pDropFiles->pFiles = sizeof(DROPFILES);
						pDropFiles->fWide = TRUE;

						LPBYTE pData = (LPBYTE)pDropFiles + sizeof(DROPFILES);
						memcpy(pData, buffer, sizeof(buffer));
						GlobalUnlock(hData);

						SetClipboardData(CF_HDROP, hData);
					}
				}
				CloseClipboard();
			}
		}
	}

	/*HGLOBAL clipbuffer;
	static wchar_t *buffer;

 	OpenClipboard(hWnd);

	EmptyClipboard();
					
	clipbuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (lstrlen(FileNames[g_FileNamePosition].c_str())+1) * sizeof(wchar_t));

	buffer = (wchar_t *)GlobalLock(clipbuffer);
					
	lstrcpyW(buffer, FileNames[g_FileNamePosition].c_str());
					
	GlobalUnlock(clipbuffer);
					
	SetClipboardData(CF_UNICODETEXT, clipbuffer);

	CloseClipboard();*/
}

/*HANDLE StringToHandle(wchar_t *szText, int nTextLen)
{
    void  *ptr;
	
    // if text length is -1 then treat as a nul-terminated string
    if(nTextLen == -1)
        nTextLen = lstrlen(szText);
    
    // allocate and lock a global memory buffer. Make it fixed
    // data so we don't have to use GlobalLock
    ptr = (void *)GlobalAlloc(GMEM_FIXED, nTextLen + 1);
	
    // copy the string into the buffer
    memcpy(ptr, szText, nTextLen);
    ptr[nTextLen] = '\0';
	
    return ptr;
}*/

/*inline HRESULT BlockForResult(ISpRecoContext * pRecoCtxt, ISpRecoResult ** ppResult)
{
    HRESULT hr = S_OK;
	CSpEvent event;

    while (SUCCEEDED(hr) &&
           SUCCEEDED(hr = event.GetFrom(pRecoCtxt)) &&
           hr == S_FALSE)
    {
        hr = pRecoCtxt->WaitForNotifyEvent(INFINITE);
    }

    *ppResult = event.RecoResult();
    if (*ppResult)
    {
        (*ppResult)->AddRef();
    }

    return hr;
}*/

void _OnCommand_ID_FILE_CUT(HWND /*hWnd*/)
{
	//ISpRecoContext *pRecoCtxt = nullptr;
	//ISpRecoGrammar *pGrammar = nullptr;
	//ISpVoice *pVoice = nullptr;

	//HRESULT hr = CoCreateInstance(CLSID_SpSharedRecoContext, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pRecoCtxt));
	//if (SUCCEEDED(hr))
	//{
	//	hr = pRecoCtxt->GetVoice(&pVoice);
	//}
	//
	//if (SUCCEEDED(hr))
	//{
	//	hr = pRecoCtxt->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
	//}

	//if (SUCCEEDED(hr))
	//{
	//	hr = pRecoCtxt->SetAudioOptions(SPAO_RETAIN_AUDIO, NULL, NULL);
	//}

	//if (SUCCEEDED(hr))
	//{
	//	hr = pRecoCtxt->CreateGrammar(0, &pGrammar);
	//}

	//if (SUCCEEDED(hr))
	//{
	//	hr = pGrammar->LoadDictation(NULL, SPLO_STATIC);
	//}
	//
	//if (SUCCEEDED(hr))
	//{
	//	hr = pGrammar->SetDictationState(SPRS_ACTIVE);
	//}

	//if (SUCCEEDED(hr))
	//{
	//	ISpRecoResult *pResult = nullptr;

	//	hr = BlockForResult(pRecoCtxt, &pResult);

	//	if (SUCCEEDED(hr))
	//	{
	//		pGrammar->SetDictationState(SPRS_INACTIVE);
	//		
	//		LPWSTR dstrText = nullptr;
	//		dstrText = new WCHAR[1000];
	//		
	//		hr = pResult->GetText((ULONG)SP_GETWHOLEPHRASE, (ULONG)SP_GETWHOLEPHRASE, TRUE, &dstrText, NULL);

	//		if (SUCCEEDED(hr))
	//		{
	//			//pVoice->Speak(L"I heard", SPF_ASYNC, NULL);
	//			//pVoice->Speak(dstrText, SPF_ASYNC, NULL);
	//			SetWindowTextW(hWnd, dstrText);
 //           }

	//		SafeRelease(&pResult);

	//		delete [] dstrText;
 //                   
 //           pGrammar->SetDictationState( SPRS_ACTIVE );
 //       } 
 //   }

	//SafeRelease(&pRecoCtxt);
	//SafeRelease(&pGrammar);
	//SafeRelease(&pVoice);




	/*ISpVoice *pVoice = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pVoice));
    if (SUCCEEDED(hr))
    {
        hr = pVoice->Speak(L"They are not artists because nobody can play the guitar!", SPF_ASYNC, NULL);
    }

	SafeRelease(&pVoice);*/

/*	wchar_t buffer[MAX_PATH];

	HRESULT hr = StringCchCopyW(buffer, MAX_PATH, FileNames[g_FileNamePosition].c_str());

	if SUCCEEDED(hr)
	{
	// Append null character to the buffer as HDROP string needs to be double null terminated
		hr = StringCchCatW(buffer, MAX_PATH, L"");
	}

	if SUCCEEDED(hr)
	{
		int nSize = sizeof(DROPFILES) + sizeof(buffer);

		DROPFILES drop;
		BYTE* lpGlobal;
		HANDLE hGlobal = GlobalAlloc(GMEM_FIXED, nSize);
		lpGlobal = (BYTE*)GlobalLock(hGlobal);
		drop.pFiles = sizeof(drop);
		drop.fWide = TRUE;
		//copy drop to lpGlobal
		memcpy_s(lpGlobal, sizeof(drop), &drop, sizeof(drop));
		//copy the list (wstr)
		memcpy(lpGlobal + sizeof(drop), buffer, sizeof(buffer));
		GlobalUnlock(hGlobal);

		//CREATING FORMATETC AND STGMEDIUM
FORMATETC formatetc;
formatetc.cfFormat = CF_HDROP;
formatetc.ptd = NULL;
formatetc.dwAspect = DVASPECT_CONTENT;
formatetc.lindex = -1;
formatetc.tymed = TYMED_HGLOBAL;
			
STGMEDIUM stgmedium;
stgmedium.tymed = TYMED_HGLOBAL;
stgmedium.pUnkForRelease = NULL;
stgmedium.hGlobal = hGlobal;

//TRYING TO SET IT ON THE CLIPBOARD:
CDataObject pDataObject(&formatetc, &stgmedium, 1);
pDataObject.AddRef();

hr = OleSetClipboard(&pDataObject);
if (FAILED(hr))
{
 //error
}

hr = OleSetClipboard(&pDataObject);
		if (SUCCEEDED(hr))
		{
			MessageBoxW(hWnd, L"OleSetClipboard", L"Info", MB_OK);
		}

hr = OleFlushClipboard();
		if (SUCCEEDED(hr))
		{
			MessageBoxW(hWnd, L"OleFlushClipboard", L"Info", MB_OK);
		}

ReleaseStgMedium(&stgmedium);
pDataObject.Release();
	}*/



	/*HRESULT hr;
	wchar_t buffer[MAX_PATH];

	hr = StringCchCopyW(buffer, MAX_PATH, FileNames[g_FileNamePosition].c_str());
	if SUCCEEDED(hr)
	{
		FORMATETC formatetc_CF_HDROP = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM stgmedium_CF_HDROP = {TYMED_HGLOBAL, {0}, 0};

		int nSize = sizeof(DROPFILES) + sizeof(buffer);
		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nSize);
					if (hData != NULL)
					{
						LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(hData);
						pDropFiles->pFiles = sizeof(DROPFILES);
						pDropFiles->fWide = TRUE;

						LPBYTE pData = (LPBYTE)pDropFiles + sizeof(DROPFILES);
						memcpy(pData, buffer, sizeof(buffer));
						GlobalUnlock(hData);
						stgmedium_CF_HDROP.hGlobal = hData;
					}*/

		/*HANDLE hCF_HDROP = GlobalAlloc(GMEM_FIXED, sizeof(buffer));
		if (hCF_HDROP != NULL)
		{
			memcpy(hCF_HDROP, buffer, sizeof(buffer));
			stgmedium_CF_HDROP.hGlobal = hCF_HDROP;
		}*/

		/*CDataObject DataObject_CF_HDROP(&formatetc_CF_HDROP, &stgmedium_CF_HDROP, 1);

		hr = OleSetClipboard(&DataObject_CF_HDROP);
		if (SUCCEEDED(hr))
		{
			//DataObject_CF_HDROP.Release();
			MessageBoxW(hWnd, L"OleSetClipboard", L"Info", MB_OK);
		}*/

		//ReleaseStgMedium(&stgmedium_CF_HDROP);

		/*UINT PreferredDropEffect = RegisterClipboardFormatW(L"CFSTR_PREFERREDDROPEFFECT");
		if (PreferredDropEffect)
		{
			FORMATETC formatetc_CFSTR_PREFERREDDROPEFFECT = {(CLIPFORMAT)PreferredDropEffect, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			STGMEDIUM stgmedium_CFSTR_PREFERREDDROPEFFECT = {TYMED_HGLOBAL, {0}, 0};

			HANDLE hPreferredDropEffect = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
			if (hPreferredDropEffect != NULL)
			{
				DWORD *dPreferredDropEffect = (DWORD*)GlobalLock(hPreferredDropEffect);
				(*dPreferredDropEffect) = DROPEFFECT_MOVE;
				GlobalUnlock(hPreferredDropEffect);
			}

			CDataObject DataObject_CFSTR_PREFERREDDROPEFFECT(&formatetc_CF_HDROP, &stgmedium_CF_HDROP, 1);

			OleSetClipboard(&DataObject_CFSTR_PREFERREDDROPEFFECT);

			//DataObject_CFSTR_PREFERREDDROPEFFECT.Release();
			//ReleaseStgMedium(&stgmedium_CFSTR_PREFERREDDROPEFFECT);
		}*/
		
		/*hr = OleFlushClipboard();
		if (SUCCEEDED(hr))
		{
			MessageBoxW(hWnd, L"OleFlushClipboard", L"Info", MB_OK);
		}
	}*/
	
	/*HRESULT hr;
	wchar_t buffer[MAX_PATH];

	hr = StringCchCopyW(buffer, MAX_PATH, FileNames[g_FileNamePosition].c_str());
	if SUCCEEDED(hr)
	{
		// Append null character to the buffer as HDROP string needs to be double null terminated
		hr = StringCchCatW(buffer, MAX_PATH, L"");

		if SUCCEEDED(hr)
		{
 			if (OpenClipboard(hWnd))
			{
				if (EmptyClipboard())
				{								
					int nSize = sizeof(DROPFILES) + sizeof(buffer);
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nSize);
					if (hData != NULL)
					{
						LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(hData);
						pDropFiles->pFiles = sizeof(DROPFILES);
						pDropFiles->fWide = TRUE;

						LPBYTE pData = (LPBYTE)pDropFiles + sizeof(DROPFILES);
						memcpy(pData, buffer, sizeof(buffer));
						GlobalUnlock(hData);

						SetClipboardData(CF_HDROP, hData);

						UINT PreferredDropEffect = RegisterClipboardFormatW(L"CFSTR_PREFERREDDROPEFFECT");
						if (PreferredDropEffect)
						{
							HANDLE hPreferredDropEffect = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
							if (hPreferredDropEffect != NULL)
							{
								DWORD *dPreferredDropEffect = (DWORD*)GlobalLock(hPreferredDropEffect);
								(*dPreferredDropEffect) = DROPEFFECT_MOVE;
								GlobalUnlock(hPreferredDropEffect);

								SetClipboardData(PreferredDropEffect, hPreferredDropEffect);
							}
						}
					}
				}
				CloseClipboard();
			}
		}
	}*/
}

void _OnCommand_ID_FILE_DELETE(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		g_BlockMovement = true;

		g_deletefilewithifo.hWnd = hWnd;
		g_deletefilewithifo.FileName = &g_Files[g_FileNamePosition].FullPath;
		g_deletefilewithifo.Permanent = false;
		g_deletefilewithifo.Silent = false;

		HANDLE hThreadDeleteFileWithIFO = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(DELETEFILEWITHIFO), // unsigned stack_size
				&DeleteFileWithIFO, // unsigned ( __stdcall *start_address )( void * )
				&g_deletefilewithifo, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

		if (!hThreadDeleteFileWithIFO)
		{
			ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}

void _OnCommand_ID_FILE_DELETEPERMANENTLY(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		g_BlockMovement = true;

		g_deletefilewithifo.hWnd = hWnd;
		g_deletefilewithifo.FileName = &g_Files[g_FileNamePosition].FullPath;
		g_deletefilewithifo.Permanent = true;
		g_deletefilewithifo.Silent = false;

		HANDLE hThreadDeleteFileWithIFO = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(DELETEFILEWITHIFO), // unsigned stack_size
				&DeleteFileWithIFO, // unsigned ( __stdcall *start_address )( void * )
				&g_deletefilewithifo, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

		if (!hThreadDeleteFileWithIFO)
		{
			ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}

void _OnCommand_ID_FILE_EXIT(HWND hWnd)
{
	DestroyWindow(hWnd);
}

void _OnCommand_ID_FILE_FIRSTFILE(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		UINT FileNamePositionFirst = g_SortByAscending ? 0U : static_cast<UINT>(g_Files.size()) - 1U;

		if (g_FileNamePosition != FileNamePositionFirst)
		{
			g_FileNamePosition = FileNamePositionFirst;

			HRESULT hr = renderer.LoadBitmapCurrent(g_Files[g_FileNamePosition].FullPath.c_str());

			if (SUCCEEDED(hr))
			{
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
	}
}

void _OnCommand_ID_FILE_FITTOWINDOW(HWND /*hWnd*/)
{
	HRESULT hr = renderer.FitToWindow();
	if (SUCCEEDED(hr))
	{
		SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
	}
}

void _OnCommand_ID_FILE_FRAMENEXT(HWND /*hWnd*/)
{
	renderer.OnFrameNext();
}

void _OnCommand_ID_FILE_FRAMEPREVIOUS(HWND /*hWnd*/)
{
	renderer.OnFramePrevious();
}

void _OnCommand_ID_FILE_FULLSCREEN(HWND hWnd)
{
	static RECT rc = {0};
	static HMENU hMenu = nullptr;
	static DWORD dwRemove = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

	if (bFullscreen)
	{
		if (LockWindowUpdate(hWnd)) // prevent intermediate redrawing
		{
			if (SetMenu(hWnd, hMenu))
			{
				if (SetWindowLongPtrW(hWnd, GWL_STYLE, dwStyle | dwRemove))
				{
					if (LockWindowUpdate(NULL)) // allow redrawing
					{
						if (SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right -rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED | SWP_NOCOPYBITS))
						{
							bFullscreen = false;
						}
					}
				}
			}
		}
	}
	else // if not fullscreen
	{
		hMenu = GetMenu(hWnd);
		
		if (GetWindowRect(hWnd, &rc))
		{
			// Hide the menu bar, change styles and position and redraw
			if (LockWindowUpdate(hWnd)) // prevent intermediate redrawing
			{
				if (SetMenu(hWnd, NULL))
				{
					if (SetWindowLongPtrW(hWnd, GWL_STYLE, dwStyle & ~dwRemove))
					{
						if (LockWindowUpdate(NULL)) // allow redrawing
						{
							if (SetWindowPos(hWnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_NOCOPYBITS))
							{
								bFullscreen = true;
							}
						}
					}
				}
			}
		}
	}
}

void _OnCommand_ID_FILE_LASTFILE(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		UINT FileNamePositionLast = g_SortByAscending ? static_cast<UINT>(g_Files.size()) - 1U : 0U;

		if (g_FileNamePosition != FileNamePositionLast)
		{
			g_FileNamePosition = FileNamePositionLast;

			HRESULT hr = renderer.LoadBitmapCurrent(g_Files[g_FileNamePosition].FullPath.c_str());

			if (SUCCEEDED(hr))
			{
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
	}
}

void _OnCommand_ID_FILE_NEW(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		size_t NumberOfFiles = g_Files.size();

		if (NumberOfFiles > 1)
		{
			UINT FileNamePositionTemp = g_FileNamePosition;
			size_t counter = 0;

			while (counter != NumberOfFiles)
			{
				if ((FileNamePositionTemp + 1U) < NumberOfFiles)
				{
					FileNamePositionTemp++;
				}
				else
				{
					FileNamePositionTemp = 0U;
				}

				// Use GetFileAttributes instead of PathFileExists to handle long file names
				DWORD fileAttributes = GetFileAttributesW(g_Files[FileNamePositionTemp].FullPath.c_str());

				if (INVALID_FILE_ATTRIBUTES != fileAttributes && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{ // ShellExecuteW strictly needs STA model of COM http://support.microsoft.com/default.aspx?scid=287087
					ShellExecuteW(
						hWnd, // __in_opt  HWND hwnd
						L"open", // __in_opt LPCTSTR lpOperation
						g_Files[FileNamePositionTemp].FullPath.c_str(), // __in LPCTSTR lpFile
						NULL, // __in_opt LPCTSTR lpParameters
						NULL, // __in_opt LPCTSTR lpDirectory
						SW_MAXIMIZE // __in INT nShowCmd
						);
					break;
				}

				counter++;
			}
		}
	}
}

void _OnCommand_ID_FILE_NEXT(HWND /*hWnd*/)
{
	if (!g_BlockMovement)
	{
		if (g_Files.size() > 1)
		{
			if (g_SortByAscending)
			{
				renderer.OnNext();
			}
			else
			{
				renderer.OnPrevious();
			}

			SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
		}
	}
}

void _OnCommand_ID_FILE_OPEN(HWND hWnd)
{
	if (!g_BlockMovement)
	{
		g_commonitemdialogopen.hWnd = hWnd;
		g_commonitemdialogopen.pszTitle = nullptr;
		g_commonitemdialogopen.rgFilterSpec = FilterSpec;
		g_commonitemdialogopen.cFileTypes = cFileTypes;
		g_commonitemdialogopen.FileName = &g_FileName;

		HANDLE hThreadCommonItemDialogOpen = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(COMMONITEMDIALOGOPEN), // unsigned stack_size
				&CommonItemDialogOpen, // unsigned ( __stdcall *start_address )( void * )
				&g_commonitemdialogopen, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

		if (!hThreadCommonItemDialogOpen)
		{
			ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}

void _OnCommand_ID_FILE_OPENFILELOCATION(HWND /*hWnd*/)
{
	ITEMIDLIST __unaligned *pidl = ILCreateFromPathW(g_Files[g_FileNamePosition].FullPath.c_str());

	if (pidl)
	{
		// CoInitialize or CoInitializeEx must be called before using SHOpenFolderAndSelectItems. Not doing so causes SHOpenFolderAndSelectItems to fail.
		HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0U, NULL, NULL);

		if (FAILED(hr))
		{
			ErrorDescription(hr);
		}

		// When using Windows 2000 or later, use CoTaskMemFree rather than ILFree. ITEMIDLIST structures are always allocated with the Component Object Model (COM) task allocator on those platforms.
		CoTaskMemFree(pidl);
	}
}

void _OnCommand_ID_FILE_PREVIOUS(HWND /*hWnd*/)
{
	if (!g_BlockMovement)
	{
		if (g_Files.size() > 1)
		{
			if (g_SortByAscending)
			{
				renderer.OnPrevious();
			}
			else
			{
				renderer.OnNext();
			}

			SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
		}
	}
}

void _OnCommand_ID_FILE_PROPERTIES(HWND hWnd)
{
	if (!SHObjectProperties(
		hWnd, // __in  HWND hwnd
		SHOP_FILEPATH, // __in  DWORD shopObjectType
		g_Files[g_FileNamePosition].FullPath.c_str(), // __in  PCWSTR pszObjectName
		L"Details" // __in  PCWSTR pszPropertyPage
		))
	{
		ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void _OnCommand_ID_FILE_ROTATECLOCKWISE(HWND /*hWnd*/)
{
	if (renderer.RotateEnabled())
	{
		HRESULT hr = renderer.Rotate(true);

		if (FAILED(hr) && WINCODEC_ERR_ABORTED != hr) // if rotation aborted by user, treat as normal exit
		{
			ErrorDescription(hr);
		}
	}
}

void _OnCommand_ID_FILE_ROTATECOUNTERCLOCKWISE(HWND /*hWnd*/)
{
	if (renderer.RotateEnabled())
	{
		HRESULT hr = renderer.Rotate(false);

		if (FAILED(hr) && WINCODEC_ERR_ABORTED != hr) // if rotation aborted by user, treat as normal exit
		{
			ErrorDescription(hr);
		}
	}
}

void _OnCommand_ID_FILE_SCALETOWINDOW(HWND /*hWnd*/)
{
	HRESULT hr = renderer.ScaleToWindow();

	if (SUCCEEDED(hr))
	{
		SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
	}
}

//WPSTYLE_CENTER; // Center the wallpaper image in its original size, filling the remaining area with a solid background color if image is smaller than screen or cropping image if image is larger.
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CENTER(HWND /*hWnd*/)
{
	HRESULT hr = SetAsDesktopBackground(&g_Files[g_FileNamePosition].FullPath, WPSTYLE_CENTER);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

//WPSTYLE_CROPTOFIT; // Windows 7 and later only. Enlarge or shrink the image to fill the screen, retaining the aspect ratio of the original image. If necessary, the image is cropped either on the top and bottom or on the left and right as necessary to fit the screen.
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_CROPTOFIT(HWND /*hWnd*/)
{
	HRESULT hr = SetAsDesktopBackground(&g_Files[g_FileNamePosition].FullPath, WPSTYLE_CROPTOFIT);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

//WPSTYLE_KEEPASPECT; // Windows 7 and later only. Enlarge or shrink the image to fill the screen, retaining the aspect ratio of the original image. If necessary, the image is padded either on the top and bottom or on the right and left with the background color to fill any screen area not covered by the image.
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_KEEPASPECT(HWND /*hWnd*/)
{
	HRESULT hr = SetAsDesktopBackground(&g_Files[g_FileNamePosition].FullPath, WPSTYLE_KEEPASPECT);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

//WPSTYLE_STRETCH; // Stretch the image to cover the full screen. This can result in distortion of the image as the image's aspect ratio is not retained.
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_STRETCH(HWND /*hWnd*/)
{
	HRESULT hr = SetAsDesktopBackground(&g_Files[g_FileNamePosition].FullPath, WPSTYLE_STRETCH);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

//WPSTYLE_TILE; // Tile the wallpaper image, starting in the upper left corner of the screen. This uses the image in its original size.
void _OnCommand_ID_FILE_SETASDESKTOPBACKGROUND_TILE(HWND /*hWnd*/)
{
	HRESULT hr = SetAsDesktopBackground(&g_Files[g_FileNamePosition].FullPath, WPSTYLE_TILE);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

void _OnCommand_ID_FILE_SORT(HWND /*hWnd*/, SORTBY SortBy)
{
	if (SortBy == g_SortByCurrent) // Do nothing if the files are already sorted using that method
	{
		return;
	}

	UINT IDOld = g_Files[g_FileNamePosition].ID;

	switch (SortBy)
	{
	case SORTBYDATEMODIFIED:
		{
			std::sort(g_Files.begin(), g_Files.end(), &FilesSortByDateModified);
		}
		break;
	case SORTBYNAME:
		{
			std::sort(g_Files.begin(), g_Files.end(), &FilesSortByNameNatural);
		}
		break;
	case SORTBYSIZE:
		{
			std::sort(g_Files.begin(), g_Files.end(), &FilesSortBySize);
		}
		break;
	}

	g_SortByCurrent = SortBy;
	
	for (UINT i = 0U; i < g_Files.size(); i++)
	{
		if (g_Files[i].ID == IDOld)
		{
			g_FileNamePosition = i;
			break;
		}
	}

	HRESULT hr = renderer.ReloadAfterSort();
	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}
void _OnCommand_ID_FILE_SORTBYASCENDING(HWND /*hWnd*/)
{
	if (!g_SortByAscending)
	{
		g_SortByAscending = true;
	}
}

void _OnCommand_ID_FILE_SORTBYDESCENDING(HWND /*hWnd*/)
{
	if (g_SortByAscending)
	{
		g_SortByAscending = false;
	}
}

void _OnCommand_ID_FILE_TOGGLEBACKGROUNDCOLOR(HWND /*hWnd*/)
{
	renderer.ToggleBackgroundColor();
}

void _OnCommand_ID_HELP_ABOUT(HWND hWnd)
{
	TASKDIALOGCONFIG config		= {0};

	config.cbSize				= sizeof(config);
	config.dwCommonButtons		= TDCBF_CLOSE_BUTTON;
	config.dwFlags				= TDF_ALLOW_DIALOG_CANCELLATION | TDF_ENABLE_HYPERLINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
	config.hInstance			= hInst;
	config.hwndParent			= hWnd;
	config.nDefaultButton		= IDCLOSE;
	config.pfCallback			= TaskDialogCallbackProc;
	config.pszContent			= L"Version 0.9.0.1\n"
								  L"<A HREF=\"http:\\maksymshostak.com\">Maksym Shostak</A>\n"
								  L"© 2015";
	config.pszMainIcon			= MAKEINTRESOURCEW(IDI_IMAGEVIEWER); // TD_INFORMATION_ICON;
	config.pszMainInstruction	= L"Image Viewer";
	config.pszWindowTitle		= L"About";
	
	HRESULT hr = TaskDialogIndirect(&config, NULL, NULL, NULL);

	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

void _OnCommand_RETURNEDFROMCOMMONITEMDIALOGOPEN(HWND hWnd)
{
	HRESULT hr = DirectoryFromFileName(&FileDirectory, g_FileName.c_str());

	if (SUCCEEDED(hr))
	{
		g_Files.clear();

		g_FileNamePosition = 0U;

		g_Directories.clear();

		DestroyMenu(hRightClickMenuTitleBar);

		hRightClickMenuTitleBar = nullptr;

		FILENAMEVECTORFROMDIRECTORY FileNameVectorFromDirectory = {&g_Files, &FileDirectory, ArrayOfFileExtensions, NumberOfFileExtensions};
		
		hThreadCreateFileNameVectorFromDirectory = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
			NULL, // void *security
			sizeof(FILENAMEVECTORFROMDIRECTORY), // unsigned stack_size
			&CreateFileNameVectorFromDirectory, // unsigned ( __stdcall *start_address )( void * )
			&FileNameVectorFromDirectory, // void *arglist
			0U, // unsigned initflag
			NULL // unsigned *thrdaddr
			));

		hr = hThreadCreateFileNameVectorFromDirectory ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		hr = renderer.LoadBitmapCurrent(g_FileName.c_str());
	}

	if (SUCCEEDED(hr))
	{
		hr = InvalidateRect(hWnd, NULL, FALSE) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	}
	
	if (FAILED(hr))
	{
		ErrorDescription(hr);
	}
}

void _OnCommand_RETURNEDFROMDELETEFILEWITHIFO(HWND /*hWnd*/, UINT codeNotify)
{
	if (codeNotify == 0U)
	{
		if (g_Files.size() > 1)
		{
			renderer.OnDelete();
		}
		else
		{
			renderer.SetCurrentErrorCode(ERROR_NO_MORE_FILES);
		}
	}

	g_BlockMovement = false;
}

void _OnContextMenu(HWND hWnd, HWND /*hWndContext*/, UINT xPos, UINT yPos)
{
	HRESULT hr = CreateRightClickMenu(&hRightClickMenu);

	if (SUCCEEDED(hr))
	{
		SetForegroundWindow(hWnd);

		/*MENUITEMINFO miiRotate = {0};
		miiRotate.cbSize = sizeof(MENUITEMINFO);
		miiRotate.fMask = MIIM_STATE;

		miiRotate.fState = renderer.RotateAutoEnabled() ? MF_ENABLED : MF_GRAYED;
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_AUTOROTATE, FALSE, &miiRotate);

		miiRotate.fState = renderer.RotateEnabled() ? MF_ENABLED : MF_GRAYED;
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_ROTATECLOCKWISE, FALSE, &miiRotate);
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_ROTATECOUNTERCLOCKWISE, FALSE, &miiRotate);*/

		MENUITEMINFO miiSortBy = {0};
		miiSortBy.cbSize = sizeof(MENUITEMINFO);
		miiSortBy.fMask = MIIM_STATE;

		miiSortBy.fState = g_SortByCurrent == SORTBYDATEMODIFIED ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_SORTBYDATEMODIFIED, FALSE, &miiSortBy);

		miiSortBy.fState = g_SortByCurrent == SORTBYNAME ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_SORTBYNAME, FALSE, &miiSortBy);

		miiSortBy.fState = g_SortByCurrent == SORTBYSIZE ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hRightClickMenu, ID_FILE_SORTBYSIZE, FALSE, &miiSortBy);
	
		// If the context menu is generated from the keyboard—for example, if the
		// user types SHIFT+F10 — then the x- and y-coordinates are -1 and the
		// application should display the context menu at the location of the
		// current selection rather than at (xPos, yPos).
		if (USHRT_MAX == xPos && USHRT_MAX == yPos)
		{
			POINT pt = {0};

			if (GetCursorPos(&pt))
			{
				xPos = UINT(pt.x);
				yPos = UINT(pt.y);
			}
			else
			{
				xPos = 0U;
				yPos = 0U;
			}
		}

		if (!TrackPopupMenu(hRightClickMenu, (GetSystemMetrics(SM_MENUDROPALIGNMENT) == 0 ? TPM_LEFTALIGN : TPM_RIGHTALIGN) | TPM_TOPALIGN, xPos, yPos, 0, hWnd, NULL))
		{
			if (ERROR_POPUP_ALREADY_ACTIVE != GetLastError())
			{
				ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
			}
		}
	}
}

BOOL _OnCreate(HWND hWnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
	HRESULT hr = S_OK;

	// if FileName not initialised from command-line argument, get user to select file
	if ((0 == wcscmp(g_FileName.c_str(), L"\0")))
	{
		hr = CommonItemDialogOpen(nullptr, FilterSpec, cFileTypes, &g_FileName);

		if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) // if user closed Open dialog without selection, treat as normal exit
		{
			return FALSE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = renderer.SetHwnd(hWnd);
	}

	if (0 == g_Files.size()) // if FileNames uninitialised
	{
		if (SUCCEEDED(hr))
		{
			hr = DirectoryFromFileName(&FileDirectory, g_FileName.c_str());
		}

		if (SUCCEEDED(hr))
		{
			FILENAMEVECTORFROMDIRECTORY FileNameVectorFromDirectory = {&g_Files, &FileDirectory, ArrayOfFileExtensions, NumberOfFileExtensions};
		
			hThreadCreateFileNameVectorFromDirectory = reinterpret_cast<HANDLE>(_beginthreadex( // NATIVE CODE
				NULL, // void *security
				sizeof(FILENAMEVECTORFROMDIRECTORY), // unsigned stack_size
				&CreateFileNameVectorFromDirectory, // unsigned ( __stdcall *start_address )( void * )
				&FileNameVectorFromDirectory, // void *arglist
				0U, // unsigned initflag
				NULL // unsigned *thrdaddr
				));

			hr = hThreadCreateFileNameVectorFromDirectory ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = renderer.LoadBitmapCurrent(g_FileName.c_str());
	}

	if (SUCCEEDED(hr))
	{
		return TRUE;
	}
	else
	{
		ErrorDescription(hr);
		return FALSE;
	}
}

void _OnDestroy(HWND /*hWnd*/)
{
	PostQuitMessage(0);
}

void _OnEndSession(HWND /*hwnd*/, BOOL fEnding)
{
	if (fEnding)
	{
		// RegisterApplicationRestart?
	}
}

BOOL _OnEraseBkgnd(HWND /*hWnd*/, HDC /*hDC*/)
{
	return TRUE; // Indicates that no further erasing is required, prevents flicker
}

void _OnKeyDown(HWND hWnd, UINT vk, BOOL /*fDown*/, int /*cRepeat*/, UINT /*flags*/)
{
	switch(vk)
	{
	case VK_ESCAPE:
		{
			if (bFullscreen)
			{
				_OnCommand_ID_FILE_FULLSCREEN(hWnd);
			}
		}
		break;
	/*case VK_APPS:
		{
			if (SendMessage(hWnd, WM_NCHITTEST, 0, 0)) //HTCAPTION
			{
				SendMessage(hWnd, WM_NCRBUTTONDOWN, HTCAPTION, 0);
			};
		}
		break;*/
	}
}

void _OnLButtonDblClk(HWND hWnd, BOOL fDoubleClick, int /*x*/, int /*y*/, UINT /*keyFlags*/)
{
	if (fDoubleClick)
	{
		_OnCommand_ID_FILE_FULLSCREEN(hWnd);
	}
}

void _OnLButtonDown(HWND hWnd, BOOL /*fDoubleClick*/, int x, int y, UINT /*keyFlags*/)
{
	SetCapture(hWnd);

	if (renderer.Pannable)
	{
		SetCursor(hCursorHandClosed);

		RECT rc = {0};

		GetWindowRect(hWnd, &rc);

		ClipCursor(&rc);

		DragStart.x = x;

		DragStart.y = y;
	}
}

void _OnLButtonUp(HWND /*hWnd*/, int /*x*/, int /*y*/, UINT /*keyFlags*/)
{
	(void)ReleaseCapture();

	if (renderer.Pannable)
	{
		(void)SetCursor(hCursorHand);

		(void)ClipCursor(nullptr);

		renderer.SetDragEnd();
	}
}

void _OnMouseMove(HWND /*hWnd*/, int x, int y, UINT keyFlags)
{
	if (renderer.Pannable)
	{
		if (keyFlags & MK_LBUTTON)
		{
			(void)renderer.SetTranslate(x - DragStart.x, y - DragStart.y);
		}
	}
}

void _OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT /*fwKeys*/)
{
	POINT pt = {xPos, yPos};
	RECT rc = {0};
	static int zDeltaAccumulator = 0;
	
	if (ScreenToClient(hWnd, &pt))
	{
		if (GetClientRect(hWnd, &rc))
		{
			if (pt.x >= 0 && pt.x < rc.right && pt.y >= 0 && pt.y < rc.bottom)
			{
				zDeltaAccumulator = zDeltaAccumulator + zDelta;

				if (abs(zDeltaAccumulator) >= 120)
				{
					int nScroll = zDeltaAccumulator/120;

					for (int i = 0; i < abs(nScroll); i++)
					{
						if (zDeltaAccumulator > 0)
						{
							(void)renderer.ZoomIn((UINT)pt.x, (UINT)pt.y);
						}
						else if (zDeltaAccumulator < 0)
						{
							(void)renderer.ZoomOut((UINT)pt.x, (UINT)pt.y);
						}

						SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);
					}
					zDeltaAccumulator = zDeltaAccumulator - 120*nScroll;
				}
			}
		}
	}
}

void _OnPaint(HWND hWnd)
{
	if (SUCCEEDED(renderer.OnRender()))
	{
		(void)ValidateRect(hWnd, NULL);
	}
}

BOOL _OnQueryEndSession(HWND /*hwnd*/)
{
	return TRUE;
}

void _OnSize(HWND /*hWnd*/, UINT /*state*/, int cx, int cy)
{
	(void)renderer.OnResize(cx, cy);
}

/*void _OnShowWindow(HWND hWnd, BOOL fShow, UINT status)
{
	if (fShow = TRUE)
	{
		renderer.OnRender();
	}
}*/

void _OnTimer(HWND /*hWnd*/, UINT id)
{
	if (id == DELAY_TIMER_ID)
	{
		renderer.GIF_OnFrameNext();
	}
}

HRESULT CALLBACK TaskDialogCallbackProc(
  __in  HWND hWnd,
  __in  UINT uNotification,
  __in  WPARAM /*wParam*/,
  __in  LPARAM lParam,
  __in  LONG_PTR /*dwRefData*/
)
{
	HRESULT hr = E_FAIL;

	if (TDN_HYPERLINK_CLICKED == uNotification)
	{

#pragma warning(push)
#pragma warning(disable:4302)
#pragma warning(disable:4311)
		
		int returnValue = reinterpret_cast<int>(
			//https://msdn.microsoft.com/en-us/library/windows/desktop/bb762153
			ShellExecuteW(
				hWnd,
				L"open",
				reinterpret_cast<LPCWSTR>(lParam),
				NULL,
				NULL,
				SW_SHOWNORMAL
				)
			);

#pragma warning(pop)

		if (returnValue > 32)
		{
			return S_OK;
		}
		else
		{
			switch (returnValue)
			{
			case 0:
				hr = E_OUTOFMEMORY;
			default:
				hr = HRESULT_FROM_WIN32(returnValue);
			}
		}
	}

	return hr;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, _OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, _OnCommand);
		HANDLE_MSG(hWnd, WM_CONTEXTMENU, _OnContextMenu);
		HANDLE_MSG(hWnd, WM_DESTROY, _OnDestroy);
		HANDLE_MSG(hWnd, WM_ENDSESSION, _OnEndSession);
		HANDLE_MSG(hWnd, WM_ERASEBKGND, _OnEraseBkgnd);
		HANDLE_MSG(hWnd, WM_KEYDOWN, _OnKeyDown);
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, _OnLButtonDblClk);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, _OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONUP, _OnLButtonUp);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, _OnMouseMove);
		HANDLE_MSG(hWnd, WM_MOUSEWHEEL, _OnMouseWheel);
		HANDLE_MSG(hWnd, WM_PAINT, _OnPaint);
		HANDLE_MSG(hWnd, WM_QUERYENDSESSION, _OnQueryEndSession);
		HANDLE_MSG(hWnd, WM_SIZE, _OnSize);
		/*HANDLE_MSG(hWnd, WM_SHOWWINDOW, _OnShowWindow);*/
		HANDLE_MSG(hWnd, WM_TIMER, _OnTimer);

	case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hWnd, NULL, FALSE);
			return 0;
        }
        break;

	case WM_HELP:
		{
			MessageBoxW(
				NULL,
#ifdef _DEBUG
				L"0.9.0.1 (Debug)",
#else
				L"0.9.0.1",
#endif
				L"Info",
				MB_OK);
			return TRUE;
		}
		break;

	case WM_MENUCOMMAND:
		{
			if (reinterpret_cast<HMENU>(lParam) == hRightClickMenuTitleBar)
			{
				(void)ShellExecuteW(
					hWnd,
					L"explore",
					g_Directories[(UINT)wParam].c_str(),
					NULL,
					NULL,
					SW_SHOWNORMAL
					);

				return 0;
			}

			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONDBLCLK:
		{
			if (HTCAPTION == wParam)
			{
				HRESULT hr = CreateRightClickMenuTitleBar(&hRightClickMenuTitleBar, g_FileName.c_str(), &g_Directories);

				if (SUCCEEDED(hr))
				{
					(void)SetForegroundWindow(hWnd);

					if (!TrackPopupMenu(hRightClickMenuTitleBar, (0 == GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_LEFTALIGN : TPM_RIGHTALIGN) | TPM_TOPALIGN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, hWnd, NULL))
					{
						if (ERROR_POPUP_ALREADY_ACTIVE != GetLastError())
						{
							ErrorDescription(HRESULT_FROM_WIN32(GetLastError()));
						}
					}
				}

				return 0;
			}

			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SETCURSOR:
		{
			if (HTCLIENT == LOWORD(lParam))
			{
				(void)SetCursor(renderer.Pannable ? hCursorHand : hCursorArrow);

				return TRUE;
			}
			
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SYSCOMMAND:
		{
			// To obtain the correct result when testing the value of wParam, an application must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.
			if ((0xFFF0 & SC_SCREENSAVE) == wParam)
			{
				// Trap screensaver to prevent from launching
				return 0;
			}
			
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
		{
			switch(GET_XBUTTON_WPARAM(wParam))
			{
			case XBUTTON1:
				{
					_OnCommand_ID_FILE_PREVIOUS(hWnd);
				}
				return TRUE;
			case XBUTTON2:
				{
					_OnCommand_ID_FILE_NEXT(hWnd);
				}
				return TRUE;
			default:
				{
					return DefWindowProcW(hWnd, message, wParam, lParam);
				}
			}
		}
		break;

	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
}

BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam)
{
	if (WM_CLOSE == LOWORD(lParam))
	{
		WCHAR buffer[ARRAYSIZE(szWindowClass)] = {0};

		if (GetClassNameW(hWnd, buffer, ARRAYSIZE(szWindowClass)))
		{
			if (0 == wcscmp(buffer, szWindowClass))
			{
				(void)PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
			}
		}
		return TRUE;
	}
	return FALSE;
}

/*HRESULT GetPhysicalProcessorCount(UINT *Count)
{
	SYSTEM_INFO sysinfo = { 0 };

	GetNativeSystemInfo(&sysinfo);

	*Count = static_cast<UINT>(sysinfo.dwNumberOfProcessors);

	return S_OK;
}*/

/*HRESULT GetThumbnail(LPCWSTR FileName, HBITMAP *phBitmap)
{
	Microsoft::WRL::ComPtr<IShellItemImageFactory> pShellItemImageFactory;

	HRESULT hr = SHCreateItemFromParsingName(FileName, NULL, IID_PPV_ARGS(&pShellItemImageFactory));

	if (SUCCEEDED(hr))
	{
		SIZE size = { 0 };

		hr = pShellItemImageFactory->GetImage(size, SIIGBF_BIGGERSIZEOK | SIIGBF_INCACHEONLY, phBitmap);
	}

	return hr;
}*/

size_t CountOccurencesOfCharacterInString(wchar_t character, std::wstring * pString)
{
	if (nullptr == pString)
	{
		return 0U;
	}

    return std::count(pString->begin(), pString->end(), character);
}

/*wchar_t buffer[260];
HRESULT hr = StringCchPrintfW(buffer, 260, L"%d, %d (%d, %d)", xPos, yPos, USHRT_MAX, USHRT_MAX);
if SUCCEEDED(hr)
{
	MessageBoxW(hWnd, buffer, L"Info", MB_OK);
}*/

/*void _OnSysKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (vk == VK_F10)
	{
		SHORT nVirtKey = GetKeyState(VK_SHIFT);

        if (nVirtKey & 0x8000) // If the high-order bit is 1, the key is down; otherwise, it is up.
		{
			PostMessageW(hWnd, WM_KEYDOWN, VK_APPS, NULL);
		}
	}
}*/