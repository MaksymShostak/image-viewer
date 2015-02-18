#pragma once

#define SEVERITYLENGTH 8
#define FACILITYLENGTH 33
#define ERRORDESCRIPTIONLENGTH SHRT_MAX

void ErrorDescription(HRESULT hr);
void HRESULTDecode(HRESULT hr, LPWSTR Severity, LPWSTR Facility, LPWSTR ErrorDescription);
void TaskDialogErrorDescription(LPCWSTR pszWindowTitle, LPCWSTR pszMainInstruction, LPCWSTR pszContent, LPCWSTR pszCollapsedControlText, LPCWSTR pszExpandedInformation);