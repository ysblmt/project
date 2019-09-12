#include <config.h>
#include <pic_operation.h>
#include <picfmt_manager.h>
#include <file.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push) /* 压栈操作 */
#pragma pack(1) /* 1byte对齐 */

typedef struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; /* 文件类型，该值必需是0x4D42，即'BM' */
	unsigned long  bfSize;	/* 该位图大小byte */
	unsigned short bfReserved1;	/* 保留->0 */
	unsigned short bfReserved2; /* 保留->0 */
	unsigned long  bfOffBits;/* 文件头开始到实际数据偏移值 */
} BITMAPFILEHEADER; /* 该结构体需要为14byte，采用pack(1)进行1字节对齐 */

typedef struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;/* BITMAPFILEHEADER字数 */
	unsigned long  biWidth;/* 图像宽度，像素为单位 */
	unsigned long  biHeight;/* 图像高度，像素为单位 */
	unsigned short biPlanes;/* 为目标设备说明位面数，总是设置为1 */
	unsigned short biBitCount;/* 比特数/像素 */
	unsigned long  biCompression;/* 像素压缩类型 */
	unsigned long  biSizeImage;/* 图像大小byte */
	unsigned long  biXPelsPerMeter;/* 水平分辨率 */
	unsigned long  biYPelsPerMeter;/* 垂直分辨率 */
	unsigned long  biClrUsed;/* 颜色索引数 */
	unsigned long  biClrImportant;/* 说明有重要影响的颜色索引数数目，0表示都重要 */
} BITMAPINFOHEADER;

#pragma pack(pop) /* 恢复先前的pack设置 */

static int isBMPFormat(PT_FileMap ptFileMap)
{
	unsigned char *aFileHead = ptFileMap->pucFileMapMem;
    
	if (aFileHead[0] != 0x42 || aFileHead[1] != 0x4d)
		return 0;
	else
		return 1;
}

/* 
 * iSrcBpp：源文件的BPP，这里是BMP格式文件的BPP
 * iDstBpp：目的设备的BPP，这里是LCD的BPP
 */
static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	int i;
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;
	int pos = 0;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	if(24 != iSrcBpp)
	{
		return -1;
	}

	if(24 == iDstBpp)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3); // iWidth * iBMPBpp / 8 = iWidth*3
	}
	else
	{
		for(i=0; i<iWidth; i++)
		{
			dwBlue	 = pudSrcDatas[pos++];
			dwGreen  = pudSrcDatas[pos++];
			dwRed 	 = pudSrcDatas[pos++];
			if(iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if(iDstBpp == 16)
			{
				dwRed	= dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | dwBlue;
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
		}
	}
	return 0;
}



static int GetPixelDatasFrmBMP(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas)
{
	BITMAPFILEHEADER *ptBITMAPFILEHEADER;
	BITMAPINFOHEADER *ptBITMAPINFOHEADER;

	unsigned char* aFileHead;
	int iWidth; 	
	int iHeight; 	
	int iBMPBpp;
	int y;
	
	int iLineWidthAlign;
	int iLineWidthReal;
	unsigned char *pucSrc;
	unsigned char *pucDest;

	
	aFileHead = ptFileMap->pucFileMapMem;

	ptBITMAPFILEHEADER = (BITMAPFILEHEADER *)aFileHead;
	ptBITMAPINFOHEADER = (BITMAPINFOHEADER *)(aFileHead + sizeof(BITMAPFILEHEADER));
	
	iWidth = ptBITMAPINFOHEADER->biWidth;
	iHeight = ptBITMAPINFOHEADER->biHeight;
	iBMPBpp = ptBITMAPINFOHEADER->biBitCount; /* LCD与BMP格式的BPP不一定相同 */

	if(iBMPBpp != 24)
	{
		DBG_PRINTF("iBMPBpp = %d\n", iBMPBpp); /* 若未设置对齐字节，将无法正确得到Bpp */
		DBG_PRINTF("sizeof(BITMAPFILEHEADER) = %d\n", sizeof(BITMAPFILEHEADER));
		return -1;
	}

	ptPixelDatas->iWidth = iWidth;
	ptPixelDatas->iHeight = iHeight;
	ptPixelDatas->iLineBytes	= iWidth * ptPixelDatas->iBpp / 8; /* 每一行 */
	ptPixelDatas->iTotalBytes	= ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;
	ptPixelDatas->aucPixelDatas = malloc(ptPixelDatas->iTotalBytes);
	if (NULL == ptPixelDatas->aucPixelDatas)
	{
		return -1;
	}

	iLineWidthReal = iWidth * iBMPBpp / 8; /* 每一行的宽度 */
	iLineWidthAlign = (iLineWidthReal + 3) & ~0x3; /* 字节数对齐向4取整 */
	
	pucSrc = aFileHead + ptBITMAPFILEHEADER->bfOffBits; /* 指向文件数据的起始 */
	pucSrc = pucSrc + (iHeight - 1) * iLineWidthAlign; /* 指向bmp文件中最后一行的起始位置 */
	
	pucDest = ptPixelDatas->aucPixelDatas;
	for(y = 0; y<iHeight; y++)
	{
		CovertOneLine(iWidth, iBMPBpp, ptPixelDatas->iBpp, pucSrc, pucDest); /* 转换一行数据 */
		pucSrc  -= iLineWidthAlign;
		pucDest += ptPixelDatas->iLineBytes;
	}
}


static int FreePixelDatasForBMP(PT_PixelDatas ptPixelDatas)
{
	free(ptPixelDatas->aucPixelDatas);
	return 0;
}



static T_PicFileParser g_tBMPParser = {
	.name           = "bmp",
	.isSupport      = isBMPFormat,
	.GetPixelDatas  = GetPixelDatasFrmBMP,
	.FreePixelDatas = FreePixelDatasForBMP,	
};


int BMPParserInit(void)
{
	return RegisterPicFileParser(&g_tBMPParser);
}



