#include <iostream>
#include "EBCDICConverter.h"
#include <cstring>

int main()
{
	std::cout << ASCIItoEBCDIC_s(strdup("        "), 9) << std::endl;
	return 0;
}