#include <stdio.h>
#include <windows.h>
#include <string.h>

#define SZ_FORMAT_STR	4
#define SZ_LIB_NAME		16

#pragma pack(1)
typedef struct _USER_MODE_ADDRESS_RESOLUTION
{
	unsigned char LoadLibraryA[SZ_LIB_NAME];
	unsigned char GetProcAddress[SZ_LIB_NAME];
}USER_MODE_ADDRESS_RESOLUTION;
#pragma pack()

// struct for use relative addresses
#pragma pack(1)
typedef struct _ADDRESS_TABLE
{
	//address resolution
	USER_MODE_ADDRESS_RESOLUTION routines;

	//application specific
	unsigned char MSVCR90dll[SZ_LIB_NAME];
	unsigned char printName[SZ_LIB_NAME];
	int* printf;
	unsigned char formatStr[SZ_FORMAT_STR];
	unsigned long globalInteger;
}ADDRESS_TABLE;
#pragma pack()

#define VAR_DWORD __asm _emit 0x04 __asm _emit 0x04 __asm _emit 0x04 __asm _emit 0x04


#define STR_DEF_04(a1, a2, a3, a4) __asm _emit a1 __asm _emit a2 __asm _emit a3 __asm _emit a4


unsigned long AddressTableStorage()
{
	unsigned int tableAddress;
	__asm
	{
		call endOfData

		STR_DEF_04('L', 'o', 'a', 'd')
		STR_DEF_04('L', 'i', 'b', 'r')
		STR_DEF_04('a', 'r', 'y', 'A')
		STR_DEF_04('\0', '\0', '\0', '\0')


		STR_DEF_04('G', 'e', 't', 'P')
		STR_DEF_04('r', 'o', 'c', 'A')
		STR_DEF_04('d', 'd', 'r', 'e')
		STR_DEF_04('s', 's', '\0', '\0')


		STR_DEF_04('c', 'r', 't', 'd')
		STR_DEF_04('l', 'l', '.', 'd')
		STR_DEF_04('l', 'l', '\0', '\0')
		STR_DEF_04('\0', '\0', '\0', '\0')


		STR_DEF_04('p', 'r', 'i', 'n')
		STR_DEF_04('t', 'f', '\0', '\0')
		STR_DEF_04('\0', '\0', '\0', '\0')
		STR_DEF_04('\0', '\0', '\0', '\0')

		VAR_DWORD
		STR_DEF_04('%', 'X', '\n', '\0')
		VAR_DWORD

		endOfData :
		pop eax
			mov tableAddress, eax
	}
	return (tableAddress);
}

unsigned long getKernel32Base()
{
	unsigned long address;
	_asm
	{
		xor ebx, ebx;
		mov ebx, fs: [0x30] ;			// go to PEB through TEB
		mov ebx, [ebx + 0x0c];		// address of PEB's _PEB_LDR_DATA substructure
		mov ebx, [ebx + 0x14];		//The _PEB_LDR_DATA structure contains a field named InMemoryOrderModuleList that represents a structure of type LIST_ENTRY
		mov ebx, [ebx];
		mov ebx, [ebx];
		mov ebx, [ebx + 0x10]; // The third element of _LDR_DATA_TABLE_ENTRY linked list corresponds to the kernel32.dll library
		mov address, ebx;
	}
	return address;
}

int getProcAddress(unsigned long dllBase,  DWORD* out_addr, char procName[])
{
	PIMAGE_DOS_HEADER MZ = (PIMAGE_DOS_HEADER)dllBase;
	PIMAGE_NT_HEADERS PE = (PIMAGE_NT_HEADERS)((LPBYTE)MZ + MZ->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY export = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)dllBase + PE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	char* dllName = (char*)(export->Name + (DWORD)dllBase);
	DWORD* routineNames = (DWORD*)(export->AddressOfNames + (DWORD)dllBase);
	DWORD* rvas = (DWORD*)(export->AddressOfFunctions + (DWORD)dllBase);
	WORD* ordinals = (WORD*)(export->AddressOfNameOrdinals + (DWORD)dllBase);
	DWORD* functionPtr = 0;
	for (int i = 0; i < export->NumberOfFunctions; i++)
	{
		DWORD j;
		DWORD name;
		DWORD entryPointRVA;
		DWORD entryPointActual;

		entryPointRVA = rvas[i];
		if (entryPointRVA == 0) { continue; }

		// for each routine RVA, we find the next sequential ordinal and its name

		for (j = 0; j < export->NumberOfFunctions; j++)
		{
			if (ordinals[j] == i)
			{
				name = (DWORD)(routineNames[j] + (DWORD)dllBase);
				entryPointActual = (DWORD)(entryPointRVA + (DWORD)dllBase);

				if (strcmp((char*)name, procName) == 0)
				{
					*functionPtr = entryPointActual;
					out_addr = functionPtr;
				}
			}
		}
	}
	return 0;
	
}

void main()
{
	DWORD* kernel32Base = getKernel32Base();

	ADDRESS_TABLE* at;
	at = (ADDRESS_TABLE*)AddressTableStorage();

	DWORD* loadLibraryPtr = 0;
	DWORD* handle = 0;

	DWORD* address = 0;
	if (getProcAddress(kernel32Base, &address, (*at).routines.LoadLibraryA))
	{
		loadLibraryPtr = address;
		handle = (DWORD*)((*at).MSVCR90dll);
		if (handle == NULL)
			return;
	}
	else
		return;

	DWORD* procAddrPtr = 0;

	if (getProcAddress(kernel32Base, &address, (*at).routines.GetProcAddress))
	{
		procAddrPtr = (DWORD*)address;
		(*at).printf = (DWORD*)(handle, (*at).printName);

		if ((*at).printf == NULL)
			return;
	}
	else
		return;

	
}
