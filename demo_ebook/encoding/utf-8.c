#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int isUtf8Support(unsigned char *pucBufHead)
{
	const char aStrUtf8[] = {0xEE,0xBB,0xBF,0};
	if(strncmp((const char *)pucBufHead,aStrUtf8,3) == 0)
	{
		return 1;
	}
	else
	{
		return -ERR;
	}
}

/* 获取前导为1的个数 */
static int GetPreOneBits(unsigned char ucVal)
{
	int i;
	int j=0;
	for(i=7;i>=0;i--)
	{
		if(!(ucVal & (1<<i)))
			break;
		else
			j++;
	}
	return j;
}

static int Utf8GetCodeFrmBuf(unsigned char *pucBufStart,unsigned char *pucBufEnd,unsigned int *pdwCode)
{
	int i;	
	int iNum;
	unsigned char ucVal;
	unsigned int dwSum = 0;

/* 
    对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
    如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的一个字节，并且不为字符的第一个字节编码;
    如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
    如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
    如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

    因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符;
    根据前二位，可判断该字节是否为一个字符编码的第一个字节; 
    根据前四位（如果前两位均为1），可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示;
    根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误。	
 */
	if(pucBufStart >= pucBufEnd)
	{
		return 0;/* 文件结束 */
	}

	ucVal = pucBufStart[0];
	iNum  = GetPreOneBits(pucBufStart[0]);

	if((pucBufStart + iNum) > pucBufEnd)
	{
		return 0;
	}

	if(iNum == 0)
	{
		*pdwCode = pucBufStart[0];
		return 1;
	}
	else
	{
		ucVal = ucVal << iNum; /* 去除前导码 */
		ucVal = ucVal >> iNum;
		dwSum += ucVal;
		for(i=1;i<iNum;i++)
		{
			ucVal = pucBufStart[i] & 0x3f; /* 除第一个8位，后面的都是10开头，除去即可 */
			dwSum = dwSum << 6;
			dwSum += ucVal;
		}
		*pdwCode = dwSum;
		return iNum;
	}
 
}


static T_EncodingOpr g_tUtf8EncodingOpr  = {
	.name			= "utf-8",
	.iHeadLen		= 3,
	.isSupport  	= isUtf8Support,
	.GetCodeFrmBuf  = Utf8GetCodeFrmBuf,
};

int  Utf8EncodingInit(void)
{
	AddFontOprForEncoding(&g_tUtf8EncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUtf8EncodingOpr, GetFontOpr("ascii"));
	return RegisterEncodingOpr(&g_tUtf8EncodingOpr);
}


