#include <config.h>
#include <file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 
 * File for memory mapping
 */
int MapFile(PT_FileMap ptFileMap)
{
	int iFd;
	FILE *tFp;
	struct stat tStat;

	/* open file */
	tFp = fopen(ptFileMap->strFileName, "r+");
	if(NULL == tFp)
	{
		DBG_PRINTF("can't open %s\n", ptFileMap->strFileName);
		return -1;
	}
	ptFileMap->tFp = tFp;
	iFd = fileno(tFp);

	fstat(iFd, &tStat);
	ptFileMap->iFileSize = tStat.st_size;
	ptFileMap->pucFileMapMem = (unsigned char *)mmap(NULL, tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
	if (ptFileMap->pucFileMapMem == (unsigned char *)-1)
	{
		DBG_PRINTF("mmap error!\n");
		return -1;
	}
	return 0;
}

void UnMapFile(PT_FileMap ptFileMap)
{
	munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
	fclose(ptFileMap->tFp);
}

/**********************************************************************
 * 函数名称： isDir
 * 功能描述： 判断一个文件是否为目录
 * 输入参数： strFilePath - 文件的路径
 *            strFileName - 文件的名字
 * 输出参数： 无
 * 返 回 值： 0 - 不是目录
 *            1 - 是目录
 * 修改日期        版本号     修改人	      修改内容
 ***********************************************************************/
static int isDir(char *strFilePath, char *strFileName)
{
	char strTmp[256];
	struct stat tStat;

	snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName); /* eg: strTmp[0] = "/bin"; */
	strTmp[255] = '\0';
	
	if ((stat(strTmp, &tStat) == 0) && S_ISDIR(tStat.st_mode)) /* 是否是目录 */
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**********************************************************************
 * 函数名称： isRegFile
 * 功能描述： 判断一个文件是否常规的文件,设备节点/链接文件/FIFO文件等是特殊文件
 * 输入参数： strFilePath - 文件的路径
 *            strFileName - 文件的名字
 * 输出参数： 无
 * 返 回 值： 0 - 不是常规文件
 *            1 - 是常规文件
 * 修改日期        版本号     修改人	      修改内容
 ***********************************************************************/
static int isRegFile(char *strFilePath, char *strFileName)
{
	char strTmp[256];
	struct stat tStat;

	snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName); /* eg: strTmp[0] = "/exp.jpg"; */
	strTmp[255] = '\0';
	
	if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**********************************************************************
 * 函数名称： GetDirContents
 * 功能描述： 把某目录下所含的顶层子目录、顶层文件都记录下来,并且按名字排序
 * 输入参数： strDirName - 目录名(含绝对路径)
 * 输出参数： pptDirContents - (*pptDirContents)指向一个PT_DirContent数组,
 *                             (*pptDirContents)[0,1,...]指向T_DirContent结构体,
 *                             T_DirContent中含有"目录/文件"的名字等信息
 *            piNumber       - strDirName下含有多少个"顶层子目录/顶层文件",
 *                             即数组(*pptDirContents)[0,1,...]有多少项
 * 返 回 值： 0 - 成功
 *            1 - 失败
 ***********************************************************************/
int GetDirContents(char* strDirName, PT_DirContent** pptDirContents, int* piNumber)
{
	PT_DirContent* aptDirContents; /* 二级指针 */
	struct dirent** aptNameList;
	int iNumber;
	int i,j;
	
	/* 扫描目录,结果按名字排序,存在aptNameList[0],aptNameList[1],... */
	iNumber = scandir(strDirName, &aptNameList, 0, alphasort); /* 按字母排序 */
	if(iNumber < 0)
	{
		DBG_PRINTF("scandir error :%s!\n", strDirName);
		return -1;
	}

	/* 忽略".",".."这两个目录 */
	aptDirContents = malloc(sizeof(PT_DirContent) * (iNumber - 2)); /* 为二级指针分配内存空间 */
	if(NULL == aptDirContents)
	{
		DBG_PRINTF("malloc error!\n");
		return -1;
	}
	*pptDirContents = aptDirContents;

	for(i=0; i<iNumber-2; i++)
	{
		aptDirContents[i] = malloc(sizeof(T_DirContent)); /* 为一级指针分配空间 */
		if (NULL == aptDirContents)
		{
			DBG_PRINTF("malloc error!\n");
			return -1;
		}
	}

	/* 先把目录挑出来存入aptDirContents */
	for(i=0,j=0; i<iNumber;i++)
	{
		/* 跳过".",".."两个目录 */
		if((0 == strcmp(aptNameList[i]->d_name, ".") && (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue; /* 终止本轮循环 */
		/* 部分文件系统不支持d_type，所以不能直接判断d_type */
		if(isDir(strDirName, aptNameList[i]->d_name)) /* 比较根目录下面的文件 */
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';
			aptDirContents[j]->eFileType = FILETYPE_DIR;
			free(aptNameList[i]);
			aptNameList[i] = NULL; /* 防止野指针 */
			j++;
		}
	}

	/* 再把常规目录挑出来存入aptDirContents */
	for(i=0; i<iNumber; i++)
	{
		if (aptNameList[i] == NULL)
            continue;

		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
			
		/* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		if(isRegFile(strDirName, aptNameList[i]->d_name))
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';
			aptDirContents[j]->eFileType = FILETYPE_DIR;
			free(aptNameList[i]);
			aptNameList[i] = NULL; /* 防止野指针 */
			j++;
		}
	}

	/* 释放aptDirContents中未使用的项 */
	for (i = j; i < iNumber - 2; i++)
	{
		free(aptDirContents[i]);
	}

	/* 释放scandir函数分配的内存 */
	for (i = 0; i < iNumber; i++)
	{
        if (aptNameList[i])
        {
    		free(aptNameList[i]);
        }
	}
	free(aptNameList);

	*piNumber = j;
	
	return 0;	
}




