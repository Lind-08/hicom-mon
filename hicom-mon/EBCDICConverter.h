#pragma once
#include <stdlib.h>

char EBCDICtoASCII(char symbol);
char* EBCDICtoASCII_s(char* str, size_t len);

char ASCIItoEBCDIC(char symbol);
char* ASCIItoEBCDIC_s(char* str, size_t len);


