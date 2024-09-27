#include <windows.h>
#include <Setupapi.h>


int test();

BOOL FindDiskNumber(LPCWSTR match, STORAGE_DEVICE_NUMBER* lpret);