#include <Windows.h>
#include <stdio.h>
#include "GameClasses.h"
#include "Util.h"
#ifdef _AMD64_
#else
#error x86 Is not supported!
#endif

//A small ReadProcessMemory Wrapper:
/// <summary>A ReadProcessMemory Wrapper</summary>
/// <param name="hProcHandle">a handle to the process to read data from</param>
/// <param name="Address">Virtual address to read from</param>
/// <returns>returns the data in the specified data type</returns>
template <class T1, class T2> 
T1 ReadProcMem( HANDLE hProcHandle, T2 Address )
{
	T1 Buffer;
	ZeroMemory( &Buffer, sizeof(T1) );
	ReadProcessMemory( hProcHandle, (void*)(Address), &Buffer, sizeof(T1), NULL );
	return Buffer;
}

/// <summary>Tells if there is a bomb at the field</summary>
/// <param name="hProcessHandle">a handle to the games process</param>
/// <param name="pBoard">address of the "Board" class</param>
/// <param name="FieldPos">field coordinates to check</param>
/// <returns>0 = No bomb | 1 = Bomb | 2 = Error</returns>
BYTE IsBomb( HANDLE hProcessHandle, DWORD64 pBoard, POINT FieldPos )
{

	/*
	mov     rax, [rcx+58h]  ; rcx = Board*
	mov     rcx, [rax+10h]
	mov     rax, [rcx+rbp*8] ; rbp = X
	mov     rcx, [rax+10h]
	cmp     [rsi+rcx], 1 ; rsi = Y
	*/
	DWORD64 Ptr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, pBoard + 0x58 );
	if ( !IsValidPointer( Ptr ) ) return 2;
	Ptr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, Ptr + 0x10 );
	if ( !IsValidPointer( Ptr ) ) return 2;
	Ptr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, Ptr + FieldPos.x * 8 );
	if ( !IsValidPointer( Ptr ) ) return 2;
	Ptr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, Ptr + 0x10 );
	if ( !IsValidPointer( Ptr ) ) return 2;

	return ReadProcMem<BYTE,DWORD64>( hProcessHandle, Ptr + FieldPos.y );
}

/// <summary>Tells if the currently highlighted field is a bomb or not</summary>
/// <param name="hProcessHandle">a handle to the games process</param>
/// <param name="dwBaseAddress">start of the games module</param>
/// <param name="OutFieldPos">optional output of the current highlited field coordinate</param>
/// <returns>0 = No bomb | 1 = Bomb | 2 = Error</returns>
BYTE BombDetector( HANDLE hProcessHandle, DWORD64 dwBaseAddress, POINT* OutFieldPos = NULL )
{
	DWORD64 GameClass = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, dwBaseAddress + OFFSET_GameClass );
	if ( !IsValidPointer( GameClass ) ) return 2;

	DWORD64 pUIBoardCanvas = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, GameClass + 0x10 );
	if ( !IsValidPointer( pUIBoardCanvas ) ) return 2;

	DWORD64 pBoard = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, GameClass + 0x18 );
	if ( !IsValidPointer( pBoard ) ) return 2;

	DWORD64 pUITile_CurrentMouseOver = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, pUIBoardCanvas + 0xF0 );
	if ( !IsValidPointer( pUITile_CurrentMouseOver ) ) return 2;

	POINT FieldPos = ReadProcMem<POINT,DWORD64>( hProcessHandle, pUITile_CurrentMouseOver + 0x30 ); //0x0030
	if ( OutFieldPos != NULL )
		 *(POINT*)OutFieldPos = FieldPos;

	return IsBomb( hProcessHandle, pBoard, FieldPos );
}

/// <summary>Makes all bombs visible</summary>
/// <param name="hProcessHandle">a handle to the games process</param>
/// <param name="dwBaseAddress">address of the games module</param>
/// <returns>true on success</returns>
bool ShowBombs( HANDLE hProcessHandle, DWORD64 dwBaseAddress )
{
	/*
	Getting UITitle from X, Y

	r8 = GameClass
	mov     rax, [r8+10h] = pUIBoardCanvas
	mov     rdx, [rax+18h]
	mov     rax, X
	mov     rax, [rdx+rax*8]

	mov     rcx, [rax+10h]
	mov     rcx, [rcx+Y*8]
	rcx => UITile*
	*/
	DWORD64 GameClass = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, dwBaseAddress + OFFSET_GameClass );
	if ( !IsValidPointer(GameClass) ) return false;

	DWORD64 pUIBoardCanvas = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, GameClass + 0x10 );
	if ( !IsValidPointer(pUIBoardCanvas) ) return false;

	DWORD64 pBoard = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, GameClass + 0x18 );
	if ( !IsValidPointer(pBoard) ) return false;

	__int32 FirstClickX = ReadProcMem<__int32,DWORD64>( hProcessHandle, pBoard + 0x28 );
	if ( FirstClickX == -1 )
	{
		printf("-> Waiting for game to start and mines position to be determined...\n");
		while (FirstClickX == -1)
		{
			Sleep( 1000 );
			FirstClickX = ReadProcMem<__int32,DWORD64>( hProcessHandle, pBoard + 0x28 );
		}
	}
	DWORD FieldXSize = ReadProcMem<DWORD,DWORD64>( hProcessHandle, pUIBoardCanvas + 0x08 );
	if ( (__int32)FieldXSize < 0 || FieldXSize > 45 ) return false;

	DWORD64 XArray = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, pUIBoardCanvas + 0x18 );
	if ( !IsValidPointer(XArray) ) return false;
	for (UINT i = 0; i < FieldXSize; i++)
	{
		DWORD64 XPtr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, XArray + i * 8 );
		//last entry is zero in case the array size is invalid
		if ( !IsValidPointer(XPtr) ) break; 

		DWORD YEntryCount = ReadProcMem<DWORD,DWORD64>( hProcessHandle, XPtr + 0x00 );

		DWORD64 YArray = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, XPtr + 0x10 );
		if ( !IsValidPointer(YArray) ) return false;
		for (UINT i = 0; i < YEntryCount; i++)
		{
			DWORD64 YPtr = ReadProcMem<DWORD64,DWORD64>( hProcessHandle, YArray + i * 8 );
			//last entry is zero in case the array size is invalid
			if ( !IsValidPointer(YPtr) ) break; 

			//this is the address of a UITile class
			void* pUITile = (void*)YPtr;

			POINT Pos = ReadProcMem<POINT,DWORD64>( hProcessHandle, (DWORD64)pUITile + 0x30 );

			if ( IsBomb( hProcessHandle, pBoard, Pos) == TRUE )
			{
				printf("--> Revealing mine ( %02u, %02u )\n",Pos.x,Pos.y);
				CreateRemoteThread( hProcessHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)( dwBaseAddress + UITile__ShowMin ), pUITile, NULL, NULL );
			}
		}
	}
	return true;
}

int main()
{
	SetConsoleTitleA( "MineSweeper Cheat" );
	printf("***********************************************************\n");
	printf("*                     MineSweeper Cheat                   *\n");
	printf("*                      from Speedi13                      *\n");
	printf("*                                                         *\n");
	printf("*               https://github.com/Speedi13               *\n");
	printf("***********************************************************\n\n");

	printf("-> Looking for MineSweeper process...");

	DWORD dwProcessId;
	while (dwProcessId = GetProcessIdFromName( L"MineSweeper.exe" ), 
			dwProcessId == NULL)
		Sleep( 2000 );

	printf("\n-> Process found with Id %u\n",dwProcessId);	

	HANDLE hProcessHandle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );
	if ( hProcessHandle == INVALID_HANDLE_VALUE || hProcessHandle == NULL )
	{
		printf("[!] Failed to access process! (0x%X)\n",GetLastError());
		system("pause");
		return 0;
	}
	
	DWORD64 dwBaseAddress = GetRemoteModuleAddress( dwProcessId, L"MineSweeper.exe" );
	if ( dwBaseAddress == NULL )
	{
		printf("[!] Failed get base address! (0x%X)\n",GetLastError());
		system("pause");
		return 0;
	}

	printf("-> Process base is located at 0x%I64X\n",dwBaseAddress);	

	printf(	"\n"
		"Options:\n"
		"\t 1 - Reveal all bombs\n"
		"\t 2 - Mine detector\n"
		"\t 3 - Mine detector (beeping)\n"
		"\n"
		);

	DWORD SelectedOption = NULL;
	do
	{
		scanf_s( "%u",&SelectedOption );
		if ( SelectedOption < 1 || 3 < SelectedOption )
		{
			printf("[!] Invalid Input!\n");
			SelectedOption = NULL;
		}
	}
	while ( !SelectedOption );

	if ( SelectedOption == 1 )
	{
		ShowBombs( hProcessHandle, dwBaseAddress );
		system("pause");
		return 0;
	}
	else
	if ( SelectedOption == 2 || SelectedOption == 3 )
	{
		printf("\nScanning: ");
		
		HANDLE hStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		GetConsoleScreenBufferInfo( hStdOutHandle, &ConsoleScreenBufferInfo );

		BYTE Status = NULL;
		POINT CursorPos;
		BYTE RotatingSymboleState = 0;
		char RotationStates[] = { '-', '\\', '|', '/' };
		do
		{
			Status = BombDetector( hProcessHandle, dwBaseAddress, &CursorPos );

			SetConsoleCursorPosition( hStdOutHandle, ConsoleScreenBufferInfo.dwCursorPosition );
			printf("%c [ %02u, %02u ] => %u", RotationStates[RotatingSymboleState], CursorPos.x, CursorPos.y, Status );

			RotatingSymboleState++;
			if ( RotatingSymboleState >= 4 )
				RotatingSymboleState = NULL;

			if ( Status == TRUE && SelectedOption == 3 )
				Beep( 400, 100 );
			else
				Sleep( 100 );

		}
		while (Status != 2);

		puts("\n-> Game closed\n");
		system("pause");
		return 0;
	}

	return 0;
}

