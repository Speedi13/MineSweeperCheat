#pragma once
#ifdef _AMD64_
#define IsValidPointer( pointer ) ( (DWORD64)pointer >= (DWORD64)0x10000 && (DWORD64)pointer < (DWORD64)0x000F000000000000 )
#else
#define IsValidPointer( pointer ) ( (DWORD)pointer >= (DWORD)0x10000 && (DWORD)pointer < (DWORD)0xFFE00000 )
#endif
#define OFFSET_GameClass 0xAAA38

//A Games function:
#define UITile__ShowMin 0x36C50 // https://i.imgur.com/PogvZef.png // https://i.imgur.com/41Qxin4.png
								//luckily its a function with only one argument at rcx,
								//so no shellcode needed to call it via CreateRemoteThread

//the classes below can be used to write an Internal cheat
//for an external cheat we only need the offsets

// Generated using ReClass
class Game;
class UIBoardCanvas;
class Board;
class UITile;

class Game
{
public:
char _0x0000[16];
	UIBoardCanvas* m_pUIBoardCanvas; //0x0010 
	Board* m_pBoard; //0x0018 
char _0x0020[24];
	BYTE GameState; //0x0038 

	static Game* GetInstance(DWORD_PTR BaseAddress)
	{
		return *(Game**)(BaseAddress + OFFSET_GameClass);
	}
};

class UIBoardCanvas
{
public:
char _0x0000[8];
	DWORD FieldX; //0x0008 
	DWORD unknown; //0x000C 
	DWORD FieldY; //0x0010 
char _0x0014[4];
	class ArrayOfFields* N6D93FF31; //0x0018 
char _0x0020[128];
	__int64 N6D941023; //0x00A0 Some GUI elements
	__int64 N6D941024; //0x00A8 
char _0x00B0[64];
	UITile* CurrentMouseOver; //0x00F0 
char _0x00F8[8];

};//Size=0x0100

class Board
{
public:
char _0x0000[20];
	DWORD PlacedMines; //0x0014 
	DWORD RevealedFields; //0x0018 
	DWORD Clicks; //0x001C 
	float flTimeElapsed; //0x0020 
char _0x0024[4];
	__int32 FirstClickX; //0x0028 (-1 if no click yet)
	__int32 FirstClickY; //0x002C (-1 if no click yet)
char _0x0030[40];
	void* N6DB5BC78; //0x0058 << stores the game field data
char _0x0060[96];

};//Size=0x00C0

class UITile
{
public:
char _0x0000[8];
	void* N6D94497B; //0x0008 some unknown ptr
	void* N6D94497C; //0x0010 
char _0x0018[24];
	DWORD FieldX; //0x0030 
	DWORD FieldY; //0x0034 
char _0x0038[24];
	class NodeSprite* m_FieldSprite; //0x0050 <= will be overwritten by us for showing the bomb
char _0x0058[88];

};//Size=0x00B0