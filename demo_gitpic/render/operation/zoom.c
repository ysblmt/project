#include <config.h>
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

/* 参考http://blog.chinaunix.net/uid-22915173-id-2185545.html
 * 利用优化后的近邻取样差值算法
 */
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic)
{
	unsigned long dwDstWidth = ptOriginPic->iWidth;
	unsigned long* pdwSrcXTable;
	unsigned long x;
	unsigned long y;
	unsigned long dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;

	
	if (ptOriginPic->iBpp != ptZoomPic->iBpp) /* 原图与缩放后图片需一致 */
	{
		return -1;
	}

	pdwSrcXTable = malloc(sizeof(unsigned long) * dwDstWidth);
	assert(NULL != pdwSrcXTable);
	
	for(x=0; x<dwDstWidth; x++) /* 每一行的缩放比例是固定的,预先建立一个缩放映射表格 */
	{
		pdwSrcXTable[x] = (x*ptOriginPic->iWidth/ptZoomPic->iWidth);
	}
	
    for (y = 0; y < ptZoomPic->iHeight; y++) /* 按行处理 */
    {			
        dwSrcY = (y * ptOriginPic->iHeight / ptZoomPic->iHeight);

		pucDest = ptZoomPic->aucPixelDatas + y*ptZoomPic->iLineBytes;
		pucSrc  = ptOriginPic->aucPixelDatas + dwSrcY*ptOriginPic->iLineBytes; /* 计算原图坐标 */
		
        for (x = 0; x <dwDstWidth; x++)
        {
            /* 原图座标: pdwSrcXTable[x]，srcy
             * 缩放座标: x, y
			 */
			 memcpy(pucDest+x*dwPixelBytes, pucSrc+pdwSrcXTable[x]*dwPixelBytes, dwPixelBytes);
        }
    }

    free(pdwSrcXTable);
	return 0;
}

