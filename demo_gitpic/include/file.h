#ifndef _FILE_H
#define _FILE_H

typedef struct FileMap{
	char strFileName[256]; /* file name */
	FILE* tFp; /* File handle */
	int iFileSize; /* File size */
	unsigned char* pucFileMapMem; /* Memory obtained by mapping files using mmap function */
}T_FileMap,*PT_FileMap;

/* 文件类别 */
typedef enum {
	FILETYPE_DIR = 0,  /* 目录 */
	FILETYPE_FILE,     /* 文件 */
}E_FileType;

/* 目录里的内容 */
typedef struct DirContent {
	char strName[256];     /* 名字 */
	E_FileType eFileType;  /* 类别：Unix环境高级编程中给定了7种文件类型，这里只判断两种 */	
}T_DirContent, *PT_DirContent;


#endif

