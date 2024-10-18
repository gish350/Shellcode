#include <stdio.h>

unsigned int globalInteger;
const char formatStr[] = "globalInteger = %X\n";

void main()
{
	globalInteger = 0xaabbccdd;
	printf(formatStr, globalInteger);
	return;
}