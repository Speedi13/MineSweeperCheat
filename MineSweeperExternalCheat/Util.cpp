#include <Windows.h>
#include "Util.h"


#include <TlHelp32.h>
/// <summary>retrieves the process id by processes name</summary>
/// <returns>returns the process id</returns>
DWORD GetProcessIdFromName( wchar_t* szName )
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( hSnapshot == INVALID_HANDLE_VALUE || hSnapshot == NULL )
		return NULL;

	PROCESSENTRY32W ProcessEntry32;
	ProcessEntry32.dwSize = sizeof( PROCESSENTRY32W );

	if ( Process32FirstW( hSnapshot, &ProcessEntry32 ) != TRUE )
	{
		CloseHandle( hSnapshot );
		return NULL;
	}
	DWORD dwProcessId = NULL;

	do
	{
		if ( wcscmp( ProcessEntry32.szExeFile, szName ) == NULL )
			dwProcessId = ProcessEntry32.th32ProcessID;

	}
	while( dwProcessId == NULL && Process32NextW( hSnapshot, &ProcessEntry32 ) == TRUE );

	CloseHandle( hSnapshot );

	return dwProcessId;
}

/// <summary>retrieves the virtual address of the specified module from a remote process</summary>
/// <param name="dwTargetPID">the process to get the information from</param>
/// <param name="szModuleName">the name of the module</param>
/// <returns>returns the virtual address of the module</returns>
DWORD_PTR GetRemoteModuleAddress(DWORD dwTargetPID, wchar_t *szModuleName )
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwTargetPID);
	if (hSnap == INVALID_HANDLE_VALUE || hSnap == NULL )
		return NULL;

	MODULEENTRY32W ModuleEntry32;
	ZeroMemory( &ModuleEntry32, sizeof( MODULEENTRY32W ) );
	ModuleEntry32.dwSize = sizeof(MODULEENTRY32W);

	if (Module32First(hSnap, &ModuleEntry32))
	{
		while (true)
		{
			if (StrCmpToLowerW(ModuleEntry32.szModule, szModuleName) == true)
			{
				CloseHandle(hSnap);
				return (DWORD_PTR)ModuleEntry32.modBaseAddr;
			}
			if ( Module32Next(hSnap, &ModuleEntry32) != TRUE ) 
				break;
		}
	}
	CloseHandle(hSnap);
	return NULL;
}

/// <summary>compares two ASCII strings without case sensitivity</summary>
/// <param name="str1">first string</param>
/// <param name="str2">second string</param>
/// <returns>true on success</returns>
bool StrCmpToLower(char* str1, char* str2 )
{
	DWORD i = 0;
	while (true)
	{
		if ( tolower(str1[i]) != tolower(str2[i]) )	
			return false;
		if (str1[i] == NULL ) //both should be the same so checking one is ok
			return true;
		i++;
	}
	return false;
}

/// <summary>compares two wchar strings without case sensitivity</summary>
/// <param name="str1">first string</param>
/// <param name="str2">second string</param>
/// <returns>true on success</returns>
bool StrCmpToLowerW(wchar_t* str1, wchar_t* str2 )
{
	DWORD i = 0;
	while (true)
	{
		if ( towlower(str1[i]) != towlower(str2[i]) )	
			return false;
		if (str1[i] == NULL ) //both should be the same so checking one is ok
			return true;
		i++;
	}
	return false;
}