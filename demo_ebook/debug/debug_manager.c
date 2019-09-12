#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead;
static int g_iDbgLevelLimit = 8;/* 打印上限 */

int RegisterDebugOpr(PT_DebugOpr ptDebugOpr)
{
	PT_DebugOpr ptTmp;

	if (!g_ptDebugOprHead)
	{
		g_ptDebugOprHead   = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptDebugOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}

	return 0;
}


void ShowDebugOpr(void)
{
	int i = 0;
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DebugOpr GetDebugOpr(char *pcName)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;
	
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


/* strBuf = "dbglevel=6" */
int SetDbgLevel(char *strBuf)
{
	g_iDbgLevelLimit = strBuf[9] - '0';
	return 0;
}


/*
 * stdout=0			   : 关闭stdout打印
 * stdout=1			   : 打开stdout打印
 * netprint=0		   : 关闭netprint打印
 * netprint=1		   : 打开netprint打印
 */
int SetDbgChanel(char *strBuf)
{
	char *pStrTmp;
	char strName[100];
	PT_DebugOpr ptTmp;

	pStrTmp = strchr(strBuf, '=');/* 指向首次出现'='的地址 */
	if(!pStrTmp)
	{
		return -1;
	}
	else
	{
		strncpy(strName,strBuf,pStrTmp-strBuf); /* pStrTmp-strBuf=(字符'='地址-字符串首地址)/sizeof(char) */
		strName[pStrTmp-strBuf] = '\0';
		ptTmp = GetDebugOpr(strName);
		if(!ptTmp)
			return -1;

		if(pStrTmp[1] == '0')
		{
			ptTmp->isCanUse = 0;
		}
		else
		{
			ptTmp->isCanUse = 1;
		}
		return 0;
	}

}

int DebugPrint(const char *pcFormat,...);
{
	char strTmpBuf[1000];
	char *pcTmp;
	va_list tArg;
	int iNUm;
	PT_DebugOpr PtTmp = g_ptDebugOprHead;
	int dbglevel = DEFAULT_DBGLEVEL;

	va_start (tArg, pcFormat); /* 将指针指向第一个参数 */
	iNum = vsprintf (strTmpBuf, pcFormat, tArg); /* 将tArg根据按格式pcFormat写入strTmpBuf */
	va_end (tArg); /* 释放指针 */
	strTmpBuf[iNum] = '\0';

	pcTmp = strTmpBuf;
	
	/* 根据打印级别决定是否打印 */
	if ((strTmpBuf[0] == '<') && (strTmpBuf[2] == '>'))
	{
		dbglevel = strTmpBuf[1] - '0';
		if (dbglevel >= 0 && dbglevel <= 9)
		{
			pcTmp = strTmpBuf + 3;
		}
		else
		{
			dbglevel = DEFAULT_DBGLEVEL;
		}
	}

	if (dbglevel > g_iDbgLevelLimit)
	{
		return -1;
	}

	/* 调用链表中所有isCanUse为1的结构体的DebugPrint函数 */
	while (ptTmp)
	{
		if (ptTmp->isCanUse)
		{
			ptTmp->DebugPrint(pcTmp);
		}
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}


int DebugInit(void)
{
	int iError;

	iError = StdoutInit();
	iError |= NetPrintInit();
	return iError;
}

int InitDebugChanel(void)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;
	while (ptTmp)
	{
		if (ptTmp->isCanUse && ptTmp->DebugInit)
		{
			ptTmp->DebugInit();
		}
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}


