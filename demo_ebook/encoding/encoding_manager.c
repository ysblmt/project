#include <config.h>
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)
{
	PT_EncodingOpr ptTmp;

	if (!g_ptEncodingOprHead)
	{
		g_ptEncodingOprHead   = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptEncodingOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	      = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

	return 0;
}


void ShowEncodingOpr(void)
{
	int i = 0;
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/* 选择编码格式 */
PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead)
{
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while(ptTmp)
	{
		if(ptTmp->isSupport(pucFileBufHead))
			return ptTmp;
		else
			ptTmp = ptTmp->ptNext;
	}

	return NULL;
}

/* 添加所能支持编码的字体 */
/* 将字体结构体填充入编码结构中 */
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr,PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptFontOprCpy;

	if(!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if(!ptFontOprCpy)
		{
			return -ERR;
		}
		else
		{
			memcpy(ptFontOprCpy,ptFontOpr,sizeof(T_FontOpr));
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;
			return 0;
		}
	}
}

/* 从链表中删除字体数据 */
int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr,PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;
	PT_FontOpr ptPre;
		
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead; /* 将PT_EncodingOpr结构体中的ptFontOprSupportedHead(PT_FontOpr) 赋给ptTmp */
		if(strcmp(ptTmp->name,ptFontOpr->name) == 0)
		{
			/* 删除头结点 */
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}
		
		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;
		while(ptTmp)
		{
			if(strcmp(ptTmp->name,ptFontOpr->name) == 0) /* 删除当前链表节点 */
			{
				/* 从链表中取出、释放 */
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}
			else /* 指向下一项 */
			{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext;
			}
		}
		return -ERR;
	}
}


int EncodingInit(void)
{
	int iError;

	iError = AsciiEncodingInit();
	if (iError)
	{
		DBG_PRINTF("AsciiEncodingInit error!\n");
		return -1;
	}

	iError = Utf16leEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16leEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf16beEncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf16beEncodingInit error!\n");
		return -1;
	}
	
	iError = Utf8EncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf8EncodingInit error!\n");
		return -1;
	}

	return 0;


	
}


