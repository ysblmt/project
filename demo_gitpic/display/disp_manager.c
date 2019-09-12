
#include <config.h>
#include <disp_manager.h>
#include <string.h>

static PT_DispOpr g_ptDispOprHead;
static PT_DispOpr g_ptDefaultDispOpr;
static PT_VideoMem g_ptVideoMemHead;

void FlushVideoMemToDev(PT_VideoMem ptVideoMem)
{
	if(!ptVideoMem->bDevFrameBuffer)
	{
		GetDefaultDispDev()->ShowPage(ptVideoMem);
	}
}


int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTmp;

	if (!g_ptDispOprHead)
	{
		g_ptDispOprHead   = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptDispOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}

	return 0;
}


void ShowDispOpr(void)
{
	int i = 0;
	PT_DispOpr ptTmp = g_ptDispOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char *pcName)
{
	PT_DispOpr ptTmp = g_ptDispOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

int GetDispResolution(int* piXres, int *piYres, int* piBpp)
{
	if(g_ptDefaultDispOpr)
	{
		*piXres = g_ptDefaultDispOpr->iXres;
		*piYres = g_ptDefaultDispOpr->iYres;
		*piBpp	= g_ptDefaultDispOpr->iBpp;
		return 0;
	}
	else
	{
		return -1;
	}
}

/* 调用具体显示设备的初始化函数并清屏 */
void SelectAndInitDefaultDispDev(char* name)
{
	g_ptDefaultDispOpr = GetDispOpr(name);
	if(g_ptDefaultDispOpr)
	{
		g_ptDefaultDispOpr->DeviceInit();
		g_ptDefaultDispOpr->CleanScreen(0);
	}
}


/* 返回所选择的显示模块，该函数在SelectAndInitDefaultDispDev调用 */
PT_DispOpr GetDefaultDispDev(void)
{
	return g_ptDefaultDispOpr;
}


/* 
 *@brief 构造缓存，用于存放显示页面的数据 
 *@param iNum为分配缓存的块数
 *@return 0-success；1-false
 */
int AllocVideoMem(int iNum)
{
	int i;
	int iXres = 0;
	int iYres = 0;
	int iBpp  = 0;
	
	int iVMSize;
	int iLineBytes;

	T_VideoMem ptNew;

	/* 确定framebuffer大小 */
	GetDispResolution(&iXres, &iYres, &iBpp);
	iVMSize = iXres * iYres * iBpp / 8; /* 显存大小 */
	iLineBytes = iXres * iBpp / 8; /* 每一行大小 */

	/* 
	 * 部分设备内存小，无法开辟多的缓存，所以需要将驱动中已经写好的显示设备的frambuffer，并将指针指向该显存，先放入链表 
	 * 刷flash时不存在操作framebuffer
	 */
	ptNew = malloc(sizeof(T_VideoMem));
	assert(NULL != ptNew);

	ptNew->tPixelDatas.aucPixelDatas	= g_ptDefaultDispOpr->pucDispMem;

	ptNew->iID							= 0;
	ptNew->bDevFrameBuffer				= 1; /* 这是设备本身的framebuffer，而不是作为缓存使用 */
	ptNew->eVideoMemState				= VMS_FREE;
	ptNew->ePicState					= PS_BLANK;
	ptNew->tPixelDatas.iWidth			= iXres;
	ptNew->tPixelDatas.iHeight			= iYres;
	ptNew->tPixelDatas.iBpp				= iBpp;
	ptNew->tPixelDatas.iLineBytes		= iLineBytes;
	ptNew->tPixelDatas.iTotalBytes		= iVMSize;
	
	if(0 != iNum)
	{
		/* 当分配空间不为0时，防止将设备本身framebuffer作为缓存分配出去，所以将状态设置为VMS_USED_FOR_CUR */
		ptNew->eVideoMemState	= VMS_USED_FOR_CUR;
	}
	ptNew->ptNext = g_ptVideoMemHead; /* 放入表头 */
	g_ptVideoMemHead = ptNew;
	
	for(i=0; i<iNum; i++)
	{
		ptNew = malloc(sizeof(T_VideoMem) + iVMSize);
		assert(NULL != ptNew);

		ptNew->tPixelDatas.aucPixelDatas = (unsigned char *)(ptNew + 1); /* 指向与framebuffer大小的缓存的首地址 */

		ptNew->iID						= 0;
		ptNew->bDevFrameBuffer			= 0;
		ptNew->eVideoMemState			= VMS_FREE;
		ptNew->ePicState				= PS_BLANK;
		ptNew->tPixelDatas.iWidth  		= iXres;
		ptNew->tPixelDatas.iHeight 		= iYres;
		ptNew->tPixelDatas.iBpp    		= iBpp;
		ptNew->tPixelDatas.iLineBytes 	= iLineBytes;
		ptNew->tPixelDatas.iTotalBytes 	= iVMSize;

		ptNew->ptNext = g_ptVideoMemHead; /* 链表头插法 */
		g_ptVideoMemHead = ptNew;
	}
	return 0;
}

/*
 *@brief 获取一款可操作VideoMem
 */
PT_VideoMem GetVideoMem(int iID, int bCur)
{
	PT_VideoMem ptTmp = g_ptVideoMemHead;

	/* 1、优先取出空闲的，ID相同的videomem */
	while(ptTmp)
	{
		if((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->iID == iID))
		{
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	/* 2、前面不成功，取出一个空闲且没有数据的videomem */
	ptTmp = g_ptVideoMemHead;
	while(ptTmp)
	{
		if((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->ePicState == PS_BLANK))
		{
			ptTmp->iID = iID;
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	/* 3、前面不成功：取出一个任意的空闲的videomem */
	ptTmp = g_ptVideoMemHead;
	while(ptTmp)
	{
		if(ptTmp->eVideoMemState == VMS_FREE)
		{
			ptTmp->iID = iID;
			ptTmp->ePicState = PS_BLANK;
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	/* 4、若没有空闲的videomem且bCur为1，则任取出一个videomem */
	if(bCur)
	{
		ptTmp = g_ptVideoMemHead;
		ptTmp->iID = iID;
		ptTmp->ePicState = PS_BLANK;
		ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
		return ptTmp;
	}

	return NULL;
}

/* 释放掉使用GetVideoMem获取的VideoMem */
void ReleaseVideoMem(PT_VideoMem ptVideoMem)
{
	ptVideoMem->eVideoMemState = VMS_FREE;
	if(ptVideoMem->iID == -1)
	{
		ptVideoMem->ePicState = PS_BLANK;
	}
}

/* 获取显示设备的framebuffer，在该块显存上操作可以直接在LCD上显示 */
PT_VideoMem GetDevVideoMem(void)
{
	PT_VideoMem ptTmp = g_ptVideoMemHead;

	while(ptTmp)
	{
		if(ptTmp->bDevFrameBuffer)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/* 将VideoMem全部清为某种颜色 */
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char* pucVM;
	unsigned short* pdwVM16bpp;
	unsigned int* pdwVM32bpp;
	unsigned short* wColor16bpp;
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucVM = ptVideoMem->tPixelDatas.aucPixelDatas;
	pdwVM16bpp = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

	switch(ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			memset(pucVM, dwColor, ptVideoMem->tPixelDatas.iTotalBytes);
			break;
		}
		case 16:
		{
			iRed	= (dwColor >> (16+3)) & 0x1f;
			iGreen	= (dwColor >> (8+2)) & 0x3f;
			iBlue	= (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while(i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pdwVM16bpp = wColor16bpp;
				pdwVM16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while(i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pdwVM32bpp = dwColor;
				pdwVM32bpp++;
				i += 4;
			}
			break;
		}
		default:
		{
			DBG_PRINTF("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}
	
}

int DisplayInit(void)
{
	int iError;
	
	iError = FBInit();

	return iError;
}

