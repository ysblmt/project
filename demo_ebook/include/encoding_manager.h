#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

typedef struct EncodingOpr {
	char *name;
	int iHeadLen;   /* 记录编码头部 */
	PT_FontOpr ptFontOprSupportedHead; /* 一种编码可以对应多种字体，定义字体链表头，获取具体字体点阵 此处指针，必须分配内存空间 */
	int (*isSupport)(unsigned char *pucBufHead); /* 是否支持该种编码 */
	int (*GetCodeFrmBuf)(unsigned char *pucBufStart,unsigned char *pucBufEnd,unsigned int *pdwCode); /* 获取字符编码 */
	struct EncodingOpr ptNext;
}T_EncodingOpr, *PT_EncodingOpr;



#endif /* _ENCODING_MANAGER_H */

