#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "windows.h"

using namespace std;

typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;


int SaveBMP( BYTE * pData, int width, int height, char * filename,WORD bitCount );