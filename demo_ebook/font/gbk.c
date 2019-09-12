#include <config.h>
#include <fonts_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int g_iFdHZK;
static unsigned char *g_pucHZKMem;
static unsigned char *g_pucHZKMemEnd;

static int GBKFontInit(char *pcFontFile,unsigned int dwFontSize)
{
	struct stat tStat;
	
	if(16 != dwFontSize)
	{
		return -ERR;
	}

	g_iFdHZK = open(pcFontFile,O_RDONLY);
	if(g_iFdHZK < 0)
	{
		DBG_PRINTF("cna't open %s\n",dwFontSize);
		return -ERR;
	}

	if(fstat(g_iFdHZK,&tStat)) /* 读取字库文件大小 */
	{
		DBG_PRINTF("can't get fstat\n");
		return -ERR;
	}

	g_pucHZKMem = (unsigned char *)mmap(NULL,tStat.st_size,PORT_READ,MAP_SHARED,g_iFdHZK,0);
	if(g_pucHZKMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for hzk16\n");
		return -ERR;
	}
	g_pucHZKMemEnd = g_pucHZKMem + tStat.st_size;

	return 0;
}


static int GBKGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap)
{
	int iArea;
	int iWhere;

	int iPenX = ptFontBitMap->iCurOriginX;
	int iPenY = ptFontBitMap->iCurOriginY;

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	if(dwCode & 0xffff0000)
	{
		DBG_PRINTF("don't support this code: 0x%x\n",dwCode);
		return -ERR;
	}

	iArea  = (int)(dwCode & 0xff) - 0xA1; /* 解码 */
	iWhere = (int)((dwCode >> 8) & 0xff) - 0xA1; 
	
	if ((iArea < 0) || (iWhere < 0))
	{
		DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -ERR;
	}
	
	ptFontBitMap->iXLeft		= iPenX; /* 左下角开始画点 */
	ptFontBitMap->iYTop			= iPenY - 16;
	ptFontBitMap->iXMax			= iPenX + 16;
	ptFontBitMap->iYMax			= iPenY;
	ptFontBitMap->iBpp			= 1;
	ptFontBitMap->iPitch		= 2; /* 2byte */
	ptFontBitMap->pucBuffer		= g_pucHZKMem + (iArea * 94 + iWhere) * 32;

	if (ptFontBitMap->pucBuffer >= g_pucHZKMemEnd)
	{
		return -ERR;
	}

	ptFontBitMap->iNextOriginX	= iPenX + 16;
	ptFontBitMap->iNextOriginY	= iPenY;
	
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}


static T_FontOpr g_tGBFontOpr = {
	.name		    = "gbk",
	.FontInit       = GBKFontInit,
	.GetFontBitmap  = GBKGetFontBitmap,
};

int GBKInit(void)
{
	return RegisterFontOpr(&g_tGBKFontOpr);
}


