#include <stdio.h>



// struct for use relative addresses
#pragma pack(1)
typedef struct _ADDRESS_TABLE
{
	int* printf;
	unsigned char formatStr[4];
	unsigned long globalInteger;
}ADDRESS_TABLE;
#pragma pack()

#define VAR_DWORD __asm _emit 0x04 __asm _emit 0x04 __asm _emit 0x04 __asm _emit 0x04
								

#define STR_DEF_04(a1, a2, a3, a4) __asm _emit a1 __asm _emit a2 __asm _emit a3 __asm _emit a4

// place for shellcode
unsigned long AddressTableStorage()
{
	unsigned int tableAddress;
	__asm
	{
		call endOfData

		STR_DEF_04( 'p', 'r', 'i', 'n')
		STR_DEF_04( 't', 'f', '\0', '\0')
		STR_DEF_04( '\0', '\0', '\0', '\0')
		STR_DEF_04( '\0', '\0', '\0', '\0')

		VAR_DWORD
		STR_DEF_04('%', 'X', '\n', '\0')
		VAR_DWORD

		endOfData:
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
		mov ebx, fs:[0x30];			// go to PEB through TEB
		mov ebx, [ebx + 0x0c];		// address of PEB's _PEB_LDR_DATA substructure
		mov ebx, [ebx + 0x14];		//The _PEB_LDR_DATA structure contains a field named InMemoryOrderModuleList that represents a structure of type LIST_ENTRY
		mov ebx, [ebx];
		mov ebx, [ebx];
		mov ebx, [ebx + 0x10]; // The third element of _LDR_DATA_TABLE_ENTRY linked list corresponds to the kernel32.dll library
		mov address, ebx;
	}
	return address;
}

void main()
{
	getKernel32Base();
	ADDRESS_TABLE* at;
	at = (ADDRESS_TABLE*)AddressTableStorage();
}
