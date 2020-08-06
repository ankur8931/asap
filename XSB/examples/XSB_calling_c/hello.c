#include <stdio.h>
#include "cinterf.h"
#define TRUE 1

DllExport int call_conv hello()
{
	printf("Hello XSB world\n");
	return TRUE;
}

