#include <pic_operation.h>
#include <string.h>

/**********************************************************************
 * 函数名称： PicMerge
 * 功能描述： 把小图片合并入大图片里
 * 输入参数： iX,iY      - 小图片合并入大图片的某个区域, iX/iY确定这个区域的左上角座标
 *            ptSmallPic - 内含小图片的象素数据
 *            ptBigPic   - 内含大图片的象素数据
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/

int PicMerge(int iX, int iY, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;
	
	if ((ptSmallPic->iWidth > ptBigPic->iWidth)  ||
		(ptSmallPic->iHeight > ptBigPic->iHeight) ||
		(ptSmallPic->iBpp != ptBigPic->iBpp))  /* 参数判断 */
	{
		return -1;
	}

	pucSrc = ptSmallPic->aucPixelDatas; /* 源 */
	pucDst = ptBigPic->aucPixelDatas + iY * ptBigPic->iLineBytes + iX * ptBigPic->iBpp / 8; /* 目的 */

	for(i=0; i<ptSmallPic->iHeight; i++)
	{
		memcpy(pucDst, pucSrc, ptSmallPic->iLineBytes);
		pucSrc += ptSmallPic->iLineBytes;
		pucDst += ptBigPic->iLineBytes;
	}
	return 0;

}

/* Merge a portion of a new image into a specified area of the old image */
int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PT_PixelDatas ptNewPic, PT_PixelDatas ptOldPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;
    int iLineBytesCpy = iWidth * ptNewPic->iBpp / 8;

    if ((iStartXofNewPic < 0 || iStartXofNewPic >= ptNewPic->iWidth) || \
        (iStartYofNewPic < 0 || iStartYofNewPic >= ptNewPic->iHeight) || \
        (iStartXofOldPic < 0 || iStartXofOldPic >= ptOldPic->iWidth) || \
        (iStartYofOldPic < 0 || iStartYofOldPic >= ptOldPic->iHeight))
    {
        return -1;
    }

    pucSrc = ptNewPic->aucPixelDatas + iStartYofNewPic * ptNewPic->iLineBytes + iStartXofNewPic * ptNewPic->iBpp / 8;
	pucDst = ptOldPic->aucPixelDatas + iStartYofOldPic * ptOldPic->iLineBytes + iStartXofOldPic * ptOldPic->iBpp / 8;
	for (i = 0; i < iHeight; i++)
	{
		memcpy(pucDst, pucSrc, iLineBytesCpy);
		pucSrc += ptNewPic->iLineBytes;
		pucDst += ptOldPic->iLineBytes;
	}
	return 0;
}


