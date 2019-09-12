#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

#include <file.h>


/* pixel datas */
typedef struct PixelDatas{
	int iWidth; 		/* Width: How many pixels are in a row */
	int iHeight; 		/* Height：Width: How many pixels are in a column */
	int iBpp; 			/* Bpp：How many bits are used to represent a pixel  */
	int iLineBytes; 	/* How many bytes of a row of data */
	int iTotalBytes; 	/* All bytes */
	unsigned char *aucPixelDatas; /* Where pixel data is stored */
}T_PixelDatas,*PT_PixelDatas;

/* image parser */
typedef struct PicFileParser{
	char *name;
	int (*isSupport)(PT_FileMap ptFileMap); /* whether to support this file */
	int (*GetPixelDatas)(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas);/* Get parsed image data */
	int (*FreePixelDatas)(PT_PixelDatas ptPixelDatas);/* Free up memory occupied by image pixel data */
	struct PicFileParser *ptNext;
}T_PicFileParser,*PT_PicFileParser;

#endif /* _PIC_OPERATION_H */

