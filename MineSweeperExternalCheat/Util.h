#pragma once

/// <summary>retrieves the process id by processes name</summary>
/// <returns>returns the process id</returns>
DWORD GetProcessIdFromName( wchar_t* szName );

/// <summary>retrieves the virtual address of the specified module from a remote process</summary>
/// <param name="dwTargetPID">the process to get the information from</param>
/// <param name="szModuleName">the name of the module</param>
/// <returns>true on success</returns>
DWORD_PTR GetRemoteModuleAddress(DWORD dwTargetPID, wchar_t *szModuleName );

/// <summary>compares two ASCII strings without case sensitivity</summary>
/// <param name="str1">first string</param>
/// <param name="str2">second string</param>
/// <returns>true on success</returns>
bool StrCmpToLower(char* str1, char* str2 );

/// <summary>compares two wchar strings without case sensitivity</summary>
/// <param name="str1">first string</param>
/// <param name="str2">second string</param>
/// <returns>true on success</returns>
bool StrCmpToLowerW(wchar_t* str1, wchar_t* str2 );