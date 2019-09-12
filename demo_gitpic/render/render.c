#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <render.h>
#include <file.h>
#include <fonts_manager.h>
#include <encoding_manager.h>
#include <picfmt_manager.h>
#include <string.h>

int GetPixelDatasForIcon(char *strFileName, PT_PixelDatas ptPixelDatas)
{
	T_FileMap tFileMap;
	int iError;
	int iXres,iYres,iBpp;

	snprintf(tFileMap.strFileName, 128, "%s%s", ICON_PATH, strFileName);
	tFileMap.strFileName[127] = '\0';

	iError = MapFile(&tFileMap);
	if(iError)
	{
		DBG_PRINTF("MapFile %s error!\n",strFileName);
		return -1;
	}

	iError = Parser("bmp")->isSupport(&tFileMap);
	if(0 == iError)
	{
		DBG_PRINTF("can't support this file：%s\n",strFileName);
		UnMapFile(&tFileMap);
		return -1;
	}

	GetDispResolution(&iXres, &iYres, &iBpp);
	ptPixelDatas->iBpp = iBpp;
	iError = Parser("bmp")->GetPixelDatas(&tFileMap, ptPixelDatas);
	if(iError)
	{
		DBG_PRINTF("GetPixelDatas for %s error!\n",strFileName);
		UnMapFile(&tFileMap);
		return -1;
	}

	UnMapFile(&tFileMap);
	return 0;
}


void FreePixelDatasForIcon(PT_PixelDatas ptPixelDatas)
{
	Parser("bmp")->FreePixelDatas(ptPixelDatas);
}

static void InvertButton(PT_Layout ptLayout)
{
	int iY;
	int i;
	int iButtonWidthBytes;
	unsigned char* pucVideoMem;
	PT_DispOpr ptDisOpr = GetDefaultDispDev();

	pucVideoMem = ptDisOpr->pucDispMem;
	pucVideoMem += ptLayout->iTopLeftY * ptDisOpr->iLineWidth + ptLayout->iTopLeftX * ptDisOpr->iBpp / 8;/* 指向图标首地址 */
	iButtonWidthBytes = (ptLayout->iBotRightX - ptLayout->iTopLeftX + 1) * ptDispOpr->iBpp / 8;
	
	for(iY=ptLayout->iTopLeftX; iY<=ptLayout->iBotRightY; iY++)
	{
		for(i=0; i<iButtonWidthBytes; i++)
		{
			pucVideoMem[i] = ~pucVideoMem[i]; /* 取反 */
		}
		pucVideoMem += ptDisOpr->iLineWidth;
	}
}


/**********************************************************************
 * 函数名称： ReleaseButton
 * 功能描述： 松开图标,只是改变显示设备上的图标按钮颜色
 * 输入参数： ptLayout   - 图标所在矩形区域
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人          修改内容
 ***********************************************************************/
void ReleaseButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}

/**********************************************************************
 * 函数名称： PressButton
 * 功能描述： 按下图标,只是改变显示设备上的图标按钮颜色
 * 输入参数： ptLayout   - 图标所在矩形区域
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人          修改内容
 ***********************************************************************/
void PressButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}

/**********************************************************************
 * 函数名称： SetColorForPixelInVideoMem
 * 功能描述： 设置VideoMem中某个座标象素的颜色
 * 输入参数： iX,iY      - 象素座标
 *            ptVideoMem - 设置VideoMem中的象素
 *            dwColor    - 设置为这个颜色,颜色格式为0x00RRGGBB
 * 输出参数： 无
 * 返 回 值： 这个象素占据多少字节
 ***********************************************************************/
static int SetColorForPixelInVideoMem(int iX, int iY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVideoMem;
	unsigned short *pwVideoMem16bpp;
	unsigned int *pdwVideoMem32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	pucVideoMem      = ptVideoMem->tPixelDatas.aucPixelDatas;
	pucVideoMem      += iY * ptVideoMem->tPixelDatas.iLineBytes + iX * ptVideoMem->tPixelDatas.iBpp / 8;
	pwVideoMem16bpp  = (unsigned short *)pucVideoMem;
	pdwVideoMem32bpp = (unsigned int *)pucVideoMem;

	//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//DBG_PRINTF("x = %d, y = %d\n", iX, iY);
	
	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			*pucVideoMem = (unsigned char)dwColor;
			return 1;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwVideoMem16bpp	= wColor16bpp;
			return 2;
			break;
		}
		case 32:
		{
			*pdwVideoMem32bpp = dwColor;
			return 4;
			break;
		}
		default :
		{			
			return -1;
		}
	}

	return -1;
}



/**********************************************************************
 * 函数名称： ClearRectangleInVideoMem
 * 功能描述： 清除VideoMem中某个矩形区域,设为某颜色
 * 输入参数： iTopLeftX,iTopLeftY   - 矩形区域的左上角座标
 *            iBotRightX,iBotRightY - 矩形区域的右下角座标
 *            ptVideoMem            - 设置VideoMem中的矩形区域
 *            dwColor               - 设置为这个颜色,颜色格式为0x00RRGGBB
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void ClearRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	int x, y;
	for (y = iTopLeftY; y <= iBotRightY; y++)
		for (x = iTopLeftX; x <= iBotRightX; x++)
			SetColorForPixelInVideoMem(x, y, ptVideoMem, dwColor);
}


/**********************************************************************
 * 函数名称： isFontInArea
 * 功能描述： 要显示的字符是否完全在指定矩形区域内
 * 输入参数： iTopLeftX,iTopLeftY   - 矩形区域的左上角座标
 *            iBotRightX,iBotRightY - 矩形区域的右下角座标
 *            ptFontBitMap          - 内含字符的位图信息
 * 输出参数： 无
 * 返 回 值： 0 - 超出了矩形区域,  1 - 完全在区域内
 ***********************************************************************/
static int isFontInArea(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_FontBitMap ptFontBitMap)
{
    if ((ptFontBitMap->iXLeft >= iTopLeftX) && (ptFontBitMap->iXMax <= iBotRightX) && \
         (ptFontBitMap->iYTop >= iTopLeftY) && (ptFontBitMap->iYMax <= iBotRightY))
         return 1;
    else
        return 0;
        
}

/**********************************************************************
 * 函数名称： MergeOneFontToVideoMem
 * 功能描述： 根据位图中的数据把字符显示到videomem中
 * 输入参数： ptVideoMem   - VideoMem
 *            ptFontBitMap - 内含字符的位图信息
 * 输出参数： 无
 * 返 回 值： 0 - 成功,  其他值 - 失败
 ***********************************************************************/
static int MergeOneFontToVideoMem(PT_FontBitMap ptFontBitMap, PT_VideoMem ptVideoMem)
{
	int i;
	int x, y;
	int bit;
	int iNum;
	unsigned char ucByte;

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
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				if (iNum == -1)
				{
					return -1;
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
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				
				if (iNum == -1)
				{
					return -1;
				}
			}
	}
	else
	{
		DBG_PRINTF("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;
}



/**********************************************************************
 * 函数名称： MergerStringToCenterOfRectangleInVideoMem
 * 功能描述： 在VideoMem的指定矩形居中显示字符串
 *            参考: 03.freetype\02th_arm\06th_show_lines_center
 * 输入参数： iTopLeftX,iTopLeftY   - 矩形区域的左上角座标
 *            iBotRightX,iBotRightY - 矩形区域的右下角座标
 *            pucTextString         - 要显示的字符串
 *            ptVideoMem            - VideoMem
 * 输出参数： 无
 * 返 回 值： 0 - 成功,  其他值 - 失败
 ***********************************************************************/
int MergerStringToCenterOfRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem)
{
	int	iLen;
	int iError;
	unsigned int dwCode;
	unsigned char *pucBufStart;
	unsigned char *pucBufEnd;
	T_FontBitMap tFontBitMap;
	
	int iMinX = 32000, iMaxX = -1;
	int iMinY = 32000, iMaxY = -1;
	int iWidth, iHeight;
	int iStrTopLeftX , iStrTopLeftY;

	int bHasGetCode = 0;

	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = 0;
	pucBufStart = pucTextString;
	pucBufEnd   = pucTextString + strlen((char *)pucTextString);   
	
	/* 0. 清除这个区域 */
	ClearRectangleInVideoMem(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, ptVideoMem, COLOR_BACKGROUND);

	/* 1. 计算字符串显示的总体高度、宽度 */
	while(1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}
		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图, 位图信息里含有字符显示时的左上角、右下角坐标 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{					
			if (iMinX > tFontBitMap.iXLeft)
			{
				iMinX = tFontBitMap.iXLeft;
			}
			if (iMaxX < tFontBitMap.iXMax)
			{
				iMaxX = tFontBitMap.iXMax;
			}

			if (iMinY > tFontBitMap.iYTop)
			{
				iMinY = tFontBitMap.iYTop;
			}
			if (iMaxY < tFontBitMap.iXMax)
			{
				iMaxY = tFontBitMap.iYMax;
			}
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for calc width/height error!\n");
		}
	}

	/* 确定字符串长度与高度 */
	iWidth  = iMaxX - iMinX;
	iHeight = iMaxY - iMinY;
	
    /* 如果字符串过长 */
    if (iWidth > iBotRightX - iTopLeftX)
    {
        iWidth = iBotRightX - iTopLeftX;
    }

    /* 如果字符串过高 */
	if (iHeight > iBotRightY - iTopLeftY)
	{
		DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		//DBG_PRINTF("iHeight = %d, iBotRightY - iTopLeftX = %d - %d = %d\n", iHeight, iBotRightY, iTopLeftY, iBotRightY - iTopLeftY);
		return -1;
	}

	/* 2.确定第一个字符的原点 */
	/* 
	 * 2.1 先计算左上角坐标
	 * 居中显示字体
	 */
	iStrTopLeftX = iTopLeftX + (iBotRightX - iTopLeftX - iWidth) / 2;
	iStrTopLeftY = iTopLeftY + (iBotRightY - iTopLeftY - iHeight) / 2;

	/* 
	 * 2.2 再计算第一个字符的原点坐标 
	 * iMinX - 原来的iCurOriginX(0) = iStrTopLeftX - 新的iCurOriginX
	 * iMinY - 原来的iCurOriginY(0) = iStrTopLeftY - 新的iCurOriginY
	 */
	tFontBitMap.iCurOriginX = iStrTopLeftX - iMinX;
	tFontBitMap.iCurOriginY = iStrTopLeftY - iMinX;

	pucBufStart = pucTextString;
	bHasGetCode = 0;
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			/* 显示一个字符 */
            if (isFontInArea(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, &tFontBitMap))
            {
    			if (MergeOneFontToVideoMem(&tFontBitMap, ptVideoMem))
    			{
    				DBG_PRINTF("MergeOneFontToVideoMem error for code 0x%x\n", dwCode);
    				return -1;
    			}
            }
            else
            {
                return 0;
            }
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for drawing error!\n");
		}
	}

	return 0;
}

