#include <config.h>
#include <encoding_manager.h>
#include <string.h>

static int isUtf16beEncoding(unsigned char *pucBufHead)
{
	const char aStrUtf16be[] = {0xFE,0xFF,0};

	if(strncmp((const char *)pucBufHead,aStrUtf16be,2) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int Utf16beGetCodeFrmBuf(unsigned char *pucBufStart,unsigned char *pucBufEnd,unsigned int *pdwCode)
{
	if(pucBufStart + 1 < pucBufEnd)
	{
		*pdwCode = (((unsigned int)pucBufStart[0])<<8) + pucBufStart[1];
		return 2;
	}
	else
	{
		return 0;
	}
}

static T_EncodingOpr g_tUtf16beEncodingOpr = {
	.name			= "utf-16be",
	.iHeadLen		= 2,
	.isSupport		= isUtf16beEncoding,
	.GetCodeFrmBuf	= Utf16beGetCodeFrmBuf,
};

int  Utf16beEncodingInit(void)
{
	AddFontOprForEncoding(&g_tUtf16beEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUtf16beEncodingOpr, GetFontOpr("ascii"));
	return RegisterEncodingOpr(&g_tUtf16beEncodingOpr);
}

