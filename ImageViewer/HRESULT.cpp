#include "stdafx.h"
#include "HRESULT.h"
#include <wincodec.h>
#include <d2derr.h>

void ErrorDescription(HRESULT hr)
{
	WCHAR Severity[SEVERITYLENGTH] = {0};
	WCHAR Facility[FACILITYLENGTH] = {0};
	WCHAR ErrorDescription[ERRORDESCRIPTIONLENGTH] = {0};
	WCHAR pszWindowTitle[] = L"Error";
	WCHAR pszMainInstruction[] = L"Unexpected error";
	WCHAR pszCollapsedControlText[11] = {0}; // 11 chars sufficient to store hexadecimal HRESULT
	WCHAR pszExpandedInformation[ERRORDESCRIPTIONLENGTH] = {0};

	HRESULTDecode(hr, Severity, Facility, ErrorDescription);

	StringCchPrintfW(pszCollapsedControlText, 11, L"0x%08x", hr);
	StringCchPrintfW(pszExpandedInformation, ERRORDESCRIPTIONLENGTH, L"Severity: %ls\nFacility: %ls\nCode: 0x%04x", Severity, Facility, HRESULT_CODE(hr));

	TASKDIALOGCONFIG config			= {0};

	config.cbSize					= sizeof(config);
	config.pszMainIcon				= TD_ERROR_ICON;
	config.pszWindowTitle			= pszWindowTitle;
	config.pszMainInstruction		= pszMainInstruction;
	config.pszContent				= ErrorDescription;
	config.pszCollapsedControlText	= pszCollapsedControlText;
	config.dwFlags					= TDF_EXPAND_FOOTER_AREA;
	config.pszExpandedInformation	= pszExpandedInformation;
	config.dwCommonButtons			= TDCBF_OK_BUTTON;
	config.nDefaultButton			= IDOK;
	
	TaskDialogIndirect(&config, NULL, NULL, NULL);
}

void HRESULTDecode(HRESULT hr, LPWSTR Severity, LPWSTR Facility, LPWSTR ErrorDescription)
{
	if (Severity)
	{
		switch (HRESULT_SEVERITY(hr))
		{
		case 0:
			{
				StringCchCopyW(Severity, SEVERITYLENGTH, L"Success");
			}
			break;
		case 1:
			{
				StringCchCopyW(Severity, SEVERITYLENGTH, L"Error");
			}
			break;
		}
	}

	if (Facility)
	{
		switch (HRESULT_FACILITY(hr))
		{
		case FACILITY_XPS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_XPS");
			}
			break;
		case FACILITY_WINRM:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINRM");
			}
			break;
		case FACILITY_WINDOWSUPDATE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINDOWSUPDATE");
			}
			break;
		case FACILITY_WINDOWS_DEFENDER:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINDOWS_DEFENDER");
			}
			break;
		case FACILITY_WINDOWS_CE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINDOWS_CE");
			}
			break;
		case FACILITY_WINDOWS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINDOWS");
			}
			break;
		case FACILITY_USERMODE_VOLMGR:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_VOLMGR");
			}
			break;
		case FACILITY_USERMODE_VIRTUALIZATION:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_VIRTUALIZATION");
			}
			break;
		case FACILITY_USERMODE_VHD:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_VHD");
			}
			break;
		case FACILITY_URT:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_URT");
			}
			break;
		case FACILITY_UMI:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_UMI");
			}
			break;
		case FACILITY_UI:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_UI");
			}
			break;
		case FACILITY_TPM_SOFTWARE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_TPM_SOFTWARE");
			}
			break;
		case FACILITY_TPM_SERVICES:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_TPM_SERVICES");
			}
			break;
		case FACILITY_SXS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SXS");
			}
			break;
		case FACILITY_STORAGE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_STORAGE");
			}
			break;
		case FACILITY_STATE_MANAGEMENT:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_STATE_MANAGEMENT");
			}
			break;
		case FACILITY_SSPI:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SSPI/SECURITY");
			}
			break;
		case FACILITY_SCARD:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SCARD");
			}
			break;
		case FACILITY_SHELL:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SHELL");
			}
			break;
		case FACILITY_SETUPAPI:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SETUPAPI");
			}
			break;
		case FACILITY_SDIAG:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_SDIAG");
			}
			break;
		case FACILITY_RPC:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_RPC");
			}
			break;
		case FACILITY_RAS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_RAS");
			}
			break;
		case FACILITY_PLA:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_PLA");
			}
			break;
		case FACILITY_OPC:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_OPC");
			}
			break;
		case FACILITY_WIN32:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WIN32");
			}
			break;
		case FACILITY_CONTROL:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_CONTROL");
			}
			break;
		case FACILITY_WEBSERVICES:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WEBSERVICES");
			}
			break;
		case FACILITY_NULL:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_NULL");
			}
			break;
		case FACILITY_NDIS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_NDIS");
			}
			break;
		case FACILITY_METADIRECTORY:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_METADIRECTORY");
			}
			break;
		case FACILITY_MSMQ:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_MSMQ");
			}
			break;
		case FACILITY_MEDIASERVER:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_MEDIASERVER");
			}
			break;
		case FACILITY_MBN:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_MBN");
			}
			break;
		case FACILITY_INTERNET:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_INTERNET");
			}
			break;
		case FACILITY_ITF:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_ITF");
			}
			break;
		case FACILITY_USERMODE_HYPERVISOR:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_HYPERVISOR");
			}
			break;
		case FACILITY_HTTP:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_HTTP");
			}
			break;
		case FACILITY_GRAPHICS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_GRAPHICS");
			}
			break;
		case FACILITY_FWP:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_FWP");
			}
			break;
		case FACILITY_FVE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_FVE");
			}
			break;
		case FACILITY_USERMODE_FILTER_MANAGER:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_FILTER_MANAGER");
			}
			break;
		case FACILITY_DPLAY:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_DPLAY");
			}
			break;
		case FACILITY_DISPATCH:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_DISPATCH");
			}
			break;
		case FACILITY_DIRECTORYSERVICE:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_DIRECTORYSERVICE");
			}
			break;
		case FACILITY_CONFIGURATION:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_CONFIGURATION");
			}
			break;
		case FACILITY_COMPLUS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_COMPLUS");
			}
			break;
		case FACILITY_USERMODE_COMMONLOG:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_USERMODE_COMMONLOG");
			}
			break;
		case FACILITY_CMI:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_CMI");
			}
			break;
		case FACILITY_CERT:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_CERT");
			}
			break;
		case FACILITY_BCD:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_BCD");
			}
			break;
		case FACILITY_BACKGROUNDCOPY:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_BACKGROUNDCOPY");
			}
			break;
		case FACILITY_ACS:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_ACS");
			}
			break;
		case FACILITY_AAF:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_AAF");
			}
			break;
		case FACILITY_WINCODEC_ERR:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_WINCODEC_ERR");
			}
			break;
		case FACILITY_D2D:
			{
				StringCchCopyW(Facility, FACILITYLENGTH, L"FACILITY_D2D");
			}
			break;
		default:
			{
				StringCchPrintfW(Facility, FACILITYLENGTH, L"Unknown facility (0x%x)", HRESULT_FACILITY(hr));
			}
		}
	}

	if (ErrorDescription)
	{
		if (!FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			(FACILITY_WINDOWS == HRESULT_FACILITY(hr)) ? HRESULT_CODE(hr) : hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			ErrorDescription,
			ERRORDESCRIPTIONLENGTH,
			NULL
			))
		{
			switch (hr)
			{
			case WINCODEC_ERR_WRONGSTATE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Wrong state");
				}
				break;
			case WINCODEC_ERR_VALUEOUTOFRANGE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Value out of range");
				}
				break;
			case WINCODEC_ERR_UNKNOWNIMAGEFORMAT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unknown image format");
				}
				break;
			case WINCODEC_ERR_UNSUPPORTEDVERSION:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unsupported version");
				}
				break;
			case WINCODEC_ERR_NOTINITIALIZED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Not initialized");
				}
				break;
			case WINCODEC_ERR_ALREADYLOCKED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Already locked");
				}
				break;
			case WINCODEC_ERR_PROPERTYNOTFOUND:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Property not found");
				}
				break;
			case WINCODEC_ERR_PROPERTYNOTSUPPORTED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Property not supported");
				}
				break;
			case WINCODEC_ERR_PROPERTYSIZE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Property size");
				}
				break;
			case WINCODEC_ERR_CODECPRESENT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Codec present");
				}
				break;
			case WINCODEC_ERR_CODECNOTHUMBNAIL:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Codec no thumbnail");
				}
				break;
			case WINCODEC_ERR_PALETTEUNAVAILABLE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Palette unavailable");
				}
				break;
			case WINCODEC_ERR_CODECTOOMANYSCANLINES:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Codec too many scanlines");
				}
				break;
			case WINCODEC_ERR_INTERNALERROR:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Internal error");
				}
				break;
			case WINCODEC_ERR_SOURCERECTDOESNOTMATCHDIMENSIONS:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Source rect does not match dimensions");
				}
				break;
			case WINCODEC_ERR_COMPONENTNOTFOUND:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Component not found");
				}
				break;
			case WINCODEC_ERR_IMAGESIZEOUTOFRANGE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Image size out of range");
				}
				break;
			case WINCODEC_ERR_TOOMUCHMETADATA:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Too much metadata");
				}
				break;
			case WINCODEC_ERR_BADIMAGE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Bad image");
				}
				break;
			case WINCODEC_ERR_BADHEADER:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Bad header");
				}
				break;
			case WINCODEC_ERR_FRAMEMISSING:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Frame missing");
				}
				break;
			case WINCODEC_ERR_BADMETADATAHEADER:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Bad metadata header");
				}
				break;
			case WINCODEC_ERR_BADSTREAMDATA:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Bad stream data");
				}
				break;
			case WINCODEC_ERR_STREAMWRITE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Stream write");
				}
				break;
			case WINCODEC_ERR_STREAMREAD:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Stream read");
				}
				break;
			case WINCODEC_ERR_STREAMNOTAVAILABLE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Stream not available");
				}
				break;
			case WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unsupported pixel format");
				}
				break;
			case WINCODEC_ERR_UNSUPPORTEDOPERATION:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unsupported operation");
				}
				break;
			case WINCODEC_ERR_INVALIDREGISTRATION:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Invalid registration");
				}
				break;
			case WINCODEC_ERR_COMPONENTINITIALIZEFAILURE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Component initialize failure");
				}
				break;
			case WINCODEC_ERR_INSUFFICIENTBUFFER:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Insufficient buffer");
				}
				break;
			case WINCODEC_ERR_DUPLICATEMETADATAPRESENT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Duplicate metadata present");
				}
				break;
			case WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Property unexpected type");
				}
				break;
			case WINCODEC_ERR_UNEXPECTEDSIZE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unexpected size");
				}
				break;
			case WINCODEC_ERR_INVALIDQUERYREQUEST:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Invalid query request");
				}
				break;
			case WINCODEC_ERR_UNEXPECTEDMETADATATYPE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Unexpected metadata type");
				}
				break;
			case WINCODEC_ERR_REQUESTONLYVALIDATMETADATAROOT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Request only valid metadata root");
				}
				break;
			case WINCODEC_ERR_INVALIDQUERYCHARACTER:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Invalid query character");
				}
				break;
			case WINCODEC_ERR_WIN32ERROR:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Win32 error");
				}
				break;
			case WINCODEC_ERR_INVALIDPROGRESSIVELEVEL:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Invalid progressive level");
				}
				break;
			case D2DERR_BAD_NUMBER:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The number is invalid.");
				}
				break;
			case D2DERR_DISPLAY_FORMAT_NOT_SUPPORTED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The display format to render is not supported by the hardware device.");
				}
				break;
			case D2DERR_DISPLAY_STATE_INVALID:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"A valid display state could not be determined.");
				}
				break;
			case D2DERR_EXCEEDS_MAX_BITMAP_SIZE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The requested size is larger than the guaranteed supported texture size.");
				}
				break;
			case D2DERR_INCOMPATIBLE_BRUSH_TYPES:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The brush types are incompatible for the call.");
				}
				break;
			case D2DERR_INTERNAL_ERROR:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The application should close this instance of Direct2D and restart it as a new process.");
				}
				break;
			case D2DERR_INVALID_CALL:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"A call to this method is invalid.");
				}
				break;
			case D2DERR_LAYER_ALREADY_IN_USE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The application attempted to reuse a layer resource that has not yet been popped off the stack.");
				}
				break;
			case D2DERR_MAX_TEXTURE_SIZE_EXCEEDED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The requested DX surface size exceeds the maximum texture size.");
				}
				break;
			case D2DERR_NO_HARDWARE_DEVICE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"There is no hardware rendering device available for this operation.");
				}
				break;
			case D2DERR_NOT_INITIALIZED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The object has not yet been initialized.");
				}
				break;
			case D2DERR_POP_CALL_DID_NOT_MATCH_PUSH:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The application attempted to pop a layer off the stack when a clip was at the top, or pop a clip off the stack when a layer was at the top.");
				}
				break;
			case D2DERR_PUSH_POP_UNBALANCED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The application did not pop all clips and layers off the stack, or it attempted to pop too many clips or layers off the stack.");
				}
				break;
			case D2DERR_RECREATE_TARGET:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"A presentation error has occurred that may be recoverable. The caller needs to re-create the render target then attempt to render the frame again.");
				}
				break;
			case D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The requested operation cannot be performed until all layers and clips have been popped off the stack.");
				}
				break;
			case D2DERR_SCANNER_FAILED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The geometry scanner failed to process the data.");
				}
				break;
			case D2DERR_SCREEN_ACCESS_DENIED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Direct2D could not access the screen.");
				}
				break;
			case D2DERR_SHADER_COMPILE_FAILED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Shader compilation failed.");
				}
				break;
			case D2DERR_TARGET_NOT_GDI_COMPATIBLE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The render target is not compatible with GDI.");
				}
				break;
			case D2DERR_TEXT_EFFECT_IS_WRONG_TYPE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"A text client drawing effect object is of the wrong type.");
				}
				break;
			case D2DERR_TEXT_RENDERER_NOT_RELEASED:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"An application is holding a reference to the IDWriteTextRenderer interface after the corresponding DrawText or DrawTextLayout call has returned.");
				}
				break;
			case D2DERR_TOO_MANY_SHADER_ELEMENTS:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Shader construction failed because it was too complex.");
				}
				break;
			case D2DERR_UNSUPPORTED_OPERATION:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The requested operation is not supported.");
				}
				break;
			case D2DERR_UNSUPPORTED_VERSION:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The requested Direct2D version is not supported.");
				}
				break;
			case D2DERR_WIN32_ERROR:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"An unknown Win32 failure occurred.");
				}
				break;
			case D2DERR_WRONG_FACTORY:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Objects used together were not all created from the same factory instance.");
				}
				break;
			case D2DERR_WRONG_RESOURCE_DOMAIN:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The resource used was created by a render target in a different resource domain.");
				}
			case D2DERR_WRONG_STATE:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The object was not in the correct state to process the method.");
				}
			case D2DERR_ZERO_VECTOR:
				{
					StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"The supplied vector is zero.");
				}
			default:
				StringCchCopyW(ErrorDescription, ERRORDESCRIPTIONLENGTH, L"Could not find a description for error.");
			}
		}
	}
}