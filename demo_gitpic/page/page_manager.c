#include <config.h>
#include <page_manager.h>
#include <render.h>
#include <string.h>
#include <stdlib.h>

static PT_PageAction g_ptPageActionHead;

int RegisterPageAction(PT_PageAction ptPageAction)
{
	PT_PageAction ptTmp;

	if (!g_ptPageActionHead)
	{
		g_ptPageActionHead   = ptPageAction;
		ptPageAction->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptPageActionHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptPageAction;
		ptPageAction->ptNext = NULL;
	}

	return 0;
}

void ShowPages(void)
{
	int i = 0;
	PT_PageAction ptTmp = g_ptPageActionHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_PageAction Page(char *pcName)
{
	PT_PageAction ptTmp = g_ptPageActionHead;
	
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

/**********************************************************************
 * 函数名称： GeneratePage
 * 功能描述： 从图标文件中解析出图像数据并放在指定区域,从而生成页面数据
 * 输入参数： ptPageLayout - 内含多个图标的文件名和显示区域
 *            ptVideoMem   - 在这个VideoMem里构造页面数据
 * 输出参数： 无
 * 返 回 值： 0      - 成功
 *            其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 ********************************************************************/
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem)
{
	T_PixelDatas tOriginIconPixelDatas; /* 原数据 */
	T_PixelDatas tIconPixelDatas; /* 生成后的图标数据 */
	int iError;
	PT_Layout atLayout = ptPageLayout->atLayout;
	
	/* 描画数据：在VideoMem中的页面数据未生成的情况下才执行下面操作 */
	if(ptVideoMem->ePicState != PS_GENERATED)
	{
		ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

		tIconPixelDatas.iBpp          = ptPageLayout->iBpp;
		/* 分配临时内存用于存放缩放后的图标数据 */
		tIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
		if(tIconPixelDatas.aucPixelDatas == NULL)
		{
			return -1;
		}

		while(atLayout->strIconName)
		{
			/* 取出图标文件像素 */
			iError = GetPixelDatasForIcon(atLayout->strIconName, &tOriginIconPixelDatas);
			if(iError)
			{
				DBG_PRINTF("GetPixelDatasForIcon %s error!\n", atLayout->strIconName);
				free(tIconPixelDatas.aucPixelDatas);
				return -1;
			}
			/* 确定显示图标的长宽高大小 */
			tIconPixelDatas.iHeight		= atLayout->iBotRightY - atLayout->iTopLeftY + 1;
			tIconPixelDatas.iWidth		= atLayout->iBotRightX = atLayout->iTopLeftX + 1;
			tIconPixelDatas.iLineBytes	= tIconPixelDatas.iWidth * tIconPixelDatas.iBpp / 8;
			tIconPixelDatas.iTotalBytes	= tIconPixelDatas.iLineBytes * tIconPixelDatas.iHeight;

			/* 将原始图标像素数据缩放到指定大小 */
			PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);

			/* 将缩放后的图标数据，合并到VideoMem的指定区域 */
			PicMerge(atLayout->iTopLeftX, atLayout->iTopLeftY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);

			/* 释放原始的图标像素数据 */
			FreePixelDatasForIcon(&tOriginIconPixelDatas);
			
			atLayout++;
		}

		/* 释放临时内存 */
		free(tIconPixelDatas.aucPixelDatas);
		ptVideoMem->ePicState = PS_GENERATED;
	}
	return 0;
}


/**********************************************************************
 * 函数名称： GenericGetInputEvent
 * 功能描述： 读取输入数据,并判断它位于哪一个图标上
 * 输入参数： ptPageLayout - 内含多个图标的显示区域
 * 输出参数： ptInputEvent - 内含得到的输入数据
 * 返 回 值： -1     - 输入数据不位于任何一个图标之上
 *            其他值 - 输入数据所落在的图标(PageLayout->atLayout数组的哪一项)
 ***********************************************************************/
int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	T_InputEvent tInputEvent;
	int iRet;
	int i = 0;
	PT_Layout atLayout = ptPageLayout->atLayout;

	iRet = GetInputEvent(&tInputEvent);
	if(iRet)
	{
		return -1;
	}

	if(tInputEvent.iType != INPUT_TYPE_TOUCHSCREEN)
	{
		return -1;
	}

	*ptInputEvent = tInputEvent;

	/* 处理数据：确定触点位于哪一个按钮上 */
	while(atLayout[i].strIconName)
	{
		if((tInputEvent.iX >= atLayout[i].iTopLeftX) && (tInputEvent.iX <= atLayout[i].iBotRightX) && \
		   (tInputEvent.iY >= atLayout[i].iTopLeftY) && (tInputEvent.iY <= atLayout[i].iBotRightY))
		{
			return i;
 	    }
		else
		{
			i++;
		}
	}

	return -1; /* 触点没有落在按钮上 */
	
}



int ID(char *strName)
{
	return (int)(strName[0]) + (int)(strName[1]) + (int)(strName[2]) + (int)(strName[3]);
}



