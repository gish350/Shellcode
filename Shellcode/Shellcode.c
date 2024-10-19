

#include <stdio.h>

unsigned int globalInteger;
const char formatStr[] = "globalInteger = %X\n";

void main()
{
	globalInteger = 0xaabbccdd;
	printf(formatStr, globalInteger);
	return;
}

#pragma pack(1)
typedef struct _ADDRESS_TABLE
{
	int* printf;
	unsigned char formatStr[100];
	unsigned long globalInteger;
}ADDRESS_TABLE;
#pragma pack()

#define VAR_DWORD (name)		__asm _emit 0x04 __asm _emit 0x04 \
								__asm _emit 0x04 __asm _emit 0x04

#define STR_DEF_04 (name, a1, a2, a3, a4)	__asm _emit a1 __asm _emit a2 \
												__asm _emit a3 __asm _emit a4


unsigned long AddressTableStorage()
{
	unsigned int tableAddress;
	__asm
	{
		call endOfData

		STR_DEF_04(printfName, 'p', 'r', 'i', 'n', )
		STR_DEF_04(printfName, 't', 'f', '\0', '\0', )
		STR_DEF_04(printfName, '\0', '\0', '\0', '\0', )
		STR_DEF_04(printfName, '\0', '\0', '\0', '\0')

		VAR_DWORD(printf)
		STR_DEF_04(formatStr, '%'. 'X', '\n', '\0')
		VAR_DWORD(globalInteger)

		endOfData;
		pop eax
		mov tableAddress, eax
	}
	return (tableAddress);
}

