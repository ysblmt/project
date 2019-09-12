#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <string.h>

static int g_iFdTextFile;
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;
static PT_EncodingOpr g_ptEncodingOprForFile;

static int g_dwFontSize;
static PT_DispOpr g_ptDispOpr;

static unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

static PT_PageDesc g_ptPages   = NULL;
static PT_PageDesc g_ptCurPage = NULL;

typedef struct PageDesc{
	int iPage;
	unsigned char *pucLcdFirstPosAtFile;/* 第一页第一个字符的地址 */
	unsigned char *pucLcdNextPageFirstPosAtFile;/* 下一页第一个字符的地址 */
	struct PageDesc *ptPrePage;/* 双向链表 */
	struct PageDesc *ptNextPage;
}T_PageDesc,*PT_PageDesc;


/* 只读方式打开文件，并获取编码方式 */
int OpenTextFile(char *pcFileName)
{
	struct stat tStat;

	g_iFdTextFile = open(pcFileName,O_RDONLY);
	if(0 > g_iFdTextFile)
	{
		DBG_PRINTF("can't open file %s\n",pcFileName);
		return -1;
	}

	if(fstat(g_iFdTextFile,&tStat)) /* 在描述符上获取打开文件有关信息 */
	{
		DBG_PRINTF("can't get fstat\n");
		return -1;
	}
	
	g_pucTextFileMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ, MAP_SHARED, g_iFdTextFile, 0);
	if (g_pucTextFileMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for text file\n");
		return -1;
	}

	g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size;

	g_ptEncodingOprForFile = SelectEncodingOprForFile(g_pucTextFileMem);/* 设置文件编码格式 */
	if (g_ptEncodingOprForFile)
	{
		g_pucLcdFirstPosAtFile = g_pucTextFileMem + g_ptEncodingOprForFile->iHeadLen;
		return 0;
	}
	else
	{
		return -1;
	}	
}


/* 
 * 设置dwFontSize大小的何种字体
 * 字体有汉字库或者freetyp字体库
 */
int SetTextDetail(char *pcHZKFile, char *pcFileFreetype, unsigned int dwFontSize)
{
	int iError = 0;
	PT_FontOpr ptFontOpr;
	PT_FontOpr ptTmp;
	int iRet = -1;	

	g_dwFontSize = dwFontSize;

	ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead; /* 根据编码方式获取字体指针 */
	while(ptFontOpr)
	{
		if(strcmp(ptFontOpr->name,"ascii") == 0)
		{
			iError = ptFontOpr->FontInit(NULL,dwFontSize);
		}
		else if(strcmp(ptFontOpr->name,"gbk") == 0)
		{
			iError = ptFontOpr->FontInit(pcHZKFile,dwFontSize);
		}
		else
		{
			iError = ptFontOpr->FontInit(pcFileFreetype,dwFontSize);
		}

		DBG_PRINTF("%s,%d\n",ptFontOpr->name,iError);

		ptTmp = ptFontOpr->ptNext;

		if(iError == 0)
		{
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding(g_ptEncodingOprForFile,ptFontOpr);
		}
		pt 
	}
		
}

/* 选择和初始化显示器 */
int SelectAndInitDisplay(char *pcName)
{
	int iError;
	g_ptDispOpr = GetDispOpr(pcName);
	if(!g_ptDispOpr)
		return -1;

	iError = g_ptDispOpr->DeviceInit();
	return iError;
}

/* 获取分辨率，触摸屏初始化调用 */
int GetDispResolution(int *piXres, int *piYres)
{
	if(!g_ptDispOpr)
	{
		*piXres = g_ptDispOpr->iXres;
		*piYres = g_ptDispOpr->iYres;
		return 0;
	}
	else
		return -1;
}

int IncLcdY(int iY) /* 返回下一个Y的值 */
{
	if(iY + g_dwFontSize < g_ptDispOpr->iYres)
		return (iY + g_dwFontSize);
	else
		return 0;
}

int RelocateFontPos(PT_FontBitMap ptFontBitMap)
{
	int iLcdY;
	int iDeltaX;
	int iDeltaY;

	if(ptFontBitMap->iYMax > g_ptDispOpr->iYres)/* 满页了：下一个Y值超出了边界，即最后一行已经显示完了 */
		return -1;
	if(ptFontBitMap->iXMax > g_ptDispOpr->iXres)/* 超过LCD最右边 */
	{
		iLcdY = IncLcdY(ptFontBitMap->iCurOriginY);/* 换行 */
		if(0 == iLcdY)
		{/* 满页了 */
			return -1;
		}
		else
		{/* 没满页 */
			iDeltaX = 0 - ptFontBitMap->iCurOriginX;/* 下一个X的值 */
			iDeltaY = iLcdY - ptFontBitMap->iCurOriginY;/* 下一个Y的值 */

			ptFontBitMap->iCurOriginX += iDeltaX;
			ptFontBitMap->iCurOriginY += iDeltaY;

			ptFontBitMap->iNextOriginX += iDeltaX;
			ptFontBitMap->iNextOriginY += iDeltaY;

			ptFontBitMap->iXLeft += iDeltaX;
			ptFontBitMap->iYMax  += iDeltaY;

			return 0;
		}
	}
	return 0;
}

int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
	/* 显示一个字体 */
	int x;
	int y;
	unsigned char ucByte = 0;
	int i = 0;
	int bit;
	
	if (ptFontBitMap->iBpp == 1)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{
			i = (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
			for (x = ptFontBitMap->iXLeft, bit = 7; x < ptFontBitMap->iXMax; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}
				
				if (ucByte & (1<<bit))
				{
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色, 不用描画 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (ptFontBitMap->iBpp == 8)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
			for (x = ptFontBitMap->iXLeft; x < ptFontBitMap->iXMax; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if (ptFontBitMap->pucBuffer[i++])
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
			}
	}
	else
	{
		DBG_PRINTF("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;
}


int ShowOnePage(unsigned char *pucTextFileMemCurPos)
{
	int iLen;
	int iError;
	T_FontBitMap tFontBitMap;
	unsigned char *pucBufStart;
	unsigned int dwCode;
	PT_FontOpr ptFontOpr;

	int bHasGetCode = 0;
	int bHasNotClrSceen = 1;

	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;
	pucBufStart = pucTextFileMemCurPos; /* 指向实际数据首地址 */

	while(1)
	{
		iLen = g_ptEncodingOprForFile->GetCodeFrmBuf(pucBufStart,g_pucTextFileMemEnd,&dwCode);/* 获取编码，返回未处理的码长 */
		if(0 == iLen)
		{
			if(!bHasGetCode)
				return -1;
			else
				return 0;
		}
		bHasGetCode = 1;
		pucBufStart += iLen; /* 跳过已显示的字符 */

		/* 有些文本，\n\r两个一起才表示回车换行   
		 * 碰到连续的\n\r，只处理一次
		 */
		if(dwCode == '\n')
		{
			g_pucLcdNextPosAtFile = pucBufStart;

			/* 回车换行 */
			tFontBitMap.iCurOriginX = 0;
			tFontBitMap.iCurOriginY = IncLcdY(tFontBitMap.iCurOriginY);
			if(0 == tFontBitMap.iCurOriginY)
				return 0;
			else
				continue;
		}else if(dwCode == '\r'){
			continue;
		}else if(dwCode == '\t'){
			dwCode = ' ';
		}

		DBG_PRINTF("dwCode = 0x%x\n",dwCode);

		ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead; /* 获取打开文件时获取的编码方式 */
		while(ptFontOpr)
		{
			DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); /* __FILE__表文件名； __FUNCTION__表函数名； __LINE__表行号 */
			iError = ptFontOpr->GetFontBitmap(dwCode,&tFontBitMap); /* 获取点阵数据 */
			DBG_PRINTF("%s %s %d, ptFontOpr->name = %s, %d\n", __FILE__, __FUNCTION__, __LINE__, ptFontOpr->name, iError);
			if(0 == iError)
			{
				DBG_PRINTF("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
				if(RelocateFontPos(&tFontBitMap)) /* 重新计算位置 */
					return 0;/* 剩下的LCD空间不能满足显示这个字符 */
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

				if(bHasNotClrSceen)
				{
					g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
					bHasNotClrSceen = 0;
				}
				DBG_PRINTF("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);

				if(ShowOneFont(&tFontBitMap))
				{
					return -1;
				}

				tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
				tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
				g_pucLcdNextPosAtFile = pucBufStart;

				break;
			}
			ptFontOpr = ptFontOpr->ptNext;
		}
	}
	return 0;
}

static void RecordPage(PT_PageDesc ptPageNew) /* 将数据存放入双向链表 */
{
	PT_PageDesc ptPage;
		
	if (!g_ptPages)/* 队列为空 */
	{
		g_ptPages = ptPageNew;
	}
	else /* 队列不为空 */
	{
		ptPage = g_ptPages;/* 前页首地址 */
		while (ptPage->ptNextPage)
		{
			ptPage = ptPage->ptNextPage;
		}
		ptPage->ptNextPage   = ptPageNew;
		ptPageNew->ptPrePage = ptPage;
	}
}

int ShowNextPage(void)
{
	int iError;
	PT_PageDesc ptPage;
	unsigned char *pucTextFileMemCurPos;


	if(g_ptCurPage)/* 队列不为空 */
		pucTextFileMemCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;
	else /* 显示第一页 */
		pucTextFileMemCurPos = g_ptCurPage->pucLcdFirstPosAtFile;

	iError = ShowOnePage(pucTextFileMemCurPos); /* 显示该页 */
	DBG_PRINTF("%s %d, %d\n",__FUNCTION__,__LINE__,iError);
	if (iError == 0)
	{
		if (g_ptCurPage && g_ptCurPage->ptNextPage) /* 若为存在下一页 */
		{
			g_ptCurPage = g_ptCurPage->ptNextPage;
			return 0;
		}
		
		ptPage = malloc(sizeof(T_PageDesc)); /* 分配结构体并放入双向链表 */
		if (ptPage)
		{
			ptPage->pucLcdFirstPosAtFile         = pucTextFileMemCurPos;
			ptPage->pucLcdNextPageFirstPosAtFile = g_pucLcdNextPosAtFile;
			ptPage->ptPrePage                    = NULL;
			ptPage->ptNextPage                   = NULL;
			g_ptCurPage = ptPage;
			DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)ptPage->pucLcdFirstPosAtFile);
			RecordPage(ptPage);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return iError;
}

int ShowPrePage(void)
{
	int iError;

	DBG_PRINTF("%s %d\n", __FUNCTION__, __LINE__);
	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)
	{
		return -1;
	}

	DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	iError = ShowOnePage(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	if (iError == 0)
	{
		DBG_PRINTF("%s %d\n", __FUNCTION__, __LINE__);
		g_ptCurPage = g_ptCurPage->ptPrePage;
	}
	return iError;
}


