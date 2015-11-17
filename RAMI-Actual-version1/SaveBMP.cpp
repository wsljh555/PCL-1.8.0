#include "SaveBMP.h"

int SaveBMP( BYTE * pData, int width, int height, char * filename,WORD bitCount )
{
	int size = width*height*bitCount/8; 
	

	DWORD NumColors;	//实际用到的颜色数,即调色板数组中的颜色个数 
	DWORD dwPaletteSize = 0;
	//RGBQUAD *pal;


	if (bitCount < 8) {  
		dwPaletteSize = ( 1 << bitCount ) * sizeof(RGBQUAD);   //调色板大小  
	}
	// 位图第一部分，文件信息
	BITMAPFILEHEADER bfh;
	bfh.bfType = 0x4d42; //bm
	bfh.bfSize = size // data size
		+ sizeof( BITMAPFILEHEADER ) // first section size
		+ sizeof( BITMAPINFOHEADER ) // second section size
		+ dwPaletteSize
		;
	bfh.bfReserved1 = 0; // reserved 
	bfh.bfReserved2 = 0; // reserved
	bfh.bfOffBits = bfh.bfSize - size;

	// 位图第二部分，数据信息
	BITMAPINFOHEADER bih;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biPlanes = 1;
	bih.biBitCount = bitCount;
	bih.biCompression = 0;
	bih.biSizeImage = size;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;       
		
	if(bih.biClrUsed!=0)	 //如果bi.biClrUsed不为零,即为实际用到的颜色数 
	{
		NumColors=(DWORD)bih.biClrUsed;   
	}
	else	 //否则,用到的颜色数为2的biBitCount次幂 	
	{
		switch(bih.biBitCount) 
		{ 
		case   1: 
			NumColors=2; 		
			break; 
		case   4: 
			NumColors=16; 
			break; 
		case   8: 
			//NumColors=256; 
			NumColors=0;
			break; 
		case   24: 
			NumColors=0;	//对于真彩色图,没用到调色板 
			break; 
		case   32: 
			NumColors=0;	//对于真彩色图,没用到调色板 
			break; 
		default:   //不处理其它的颜色数，认为出错。 
			MessageBoxA(NULL, "Invalid color numbers! ", "Error ",MB_OK|MB_ICONEXCLAMATION); 
			return FALSE;   //返回FALSE 
		} 
	}


	FILE* fp;
	fopen_s(&fp, filename,"wb");
	if( !fp ) return FALSE;

	fwrite( &bfh, 1, sizeof(BITMAPFILEHEADER), fp );

	fwrite( &bih, 1, sizeof(BITMAPINFOHEADER), fp );

	if(NumColors!=0)   
	{ 
		switch(NumColors)
		{
		case 2:
			NumColors=2; 
			BYTE pal[8]={0,0,0,0,255,255,255,0};  
			fwrite( pal, 1, 8,fp);
			BYTE *bw = new BYTE[size];
			BYTE *pByte = pData;
			int index = 0;
			for (int i =0;  i<size; i++)
			{
				bw[i] = *pByte & 0x80 |
					*(pByte+1) & 0x40 |
					*(pByte+2) & 0x20 |
					*(pByte+3) & 0x10 |
					*(pByte+4) & 0x08 |
					*(pByte+5) & 0x04 |
					*(pByte+6) & 0x02 |
					*(pByte+7) & 0x01 ;
				pByte+=8;
				
			}
			fwrite( (BYTE*)bw , 1, size, fp );
			fclose( fp );
			delete [] bw;
			bw = 0;
			pByte = 0;
			return TRUE;					
		}
	}
	fwrite( pData, 1, size, fp );
	fclose( fp );
	return TRUE;
}