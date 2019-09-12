/* 字体模块 */
#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap{
	int iXLeft;  /* 记录字体左上角值 */
	int iYTop;
	int iXMax;	/* 记录字体的长宽 */
	int iYMax;
	int iBpp;
	int iPitch;	/* 山下两行间跨度 */
	int iCurOriginX; /* 当前字体画点 */
	int iCurOriginY;
	int iNextOriginX; /* 下一个字体画点 */
	int iNextOriginY;
	unsigned char *pucBuffer;
}T_FontBitMap,*PT_FontBitMap;

typedef struct FontOpr{
	char *name;
	int (*FontInit)(char *pcFontFile,unsigned int dwFontSize);
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
	struct FontOpr *ptNext;
}T_FontOpr, *PT_FontOpr;

int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
int ASCIIInit(void);
int GBKInit(void);
int FreeTypeInit(void);
PT_FontOpr GetFontOpr(char *pcName);


#endif /* _FONTS_MANAGER_H */

