#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <pic_operation.h>

/* 显示区域，若是图标，则需要含有文件名 */
typedef struct Layout{
	int iTopLeftX; /* 左上角顶点 */
	int iTopLeftY;
	int iBotRightX; /* 右下角顶点 */
	int iBotRightY;
	char* strIconName;
}T_Layout,*PT_Layout;

typedef enum {
	VMS_FREE = 0,
	VMS_USED_FOR_PREPARE,
	VMS_USED_FOR_CUR,	
}E_VideoMemState;

typedef enum {
	PS_BLANK = 0,
	PS_GENERATING,
	PS_GENERATED,	
}E_PicState;

typedef struct VideoMem{
	/* page ID: Used to identify different pages */
	int iID;
	/* 1:Display device's own framebuffer; 0:Artificially allocated normal cache */
	int bDevFrameBuffer;
	/* 
	 * State of this VideoMem: 
	 * VMS_FREE: idle
	 * VMS_USED_FOR_PREPARE: it is used by extra threads 
	 * VMS_USED_FOR_CUR: it is used by the current threads
	 */
	E_VideoMemState eVideoMemState;
	/* 
	 * Picture status:
	 * PS_BLANK: blank
	 * PS_GENERATING: it is beging generated
	 * PS_GENERATED: it has been generated
	 */
	E_PicState ePicState;
	/* Image storage space */
	T_PixelDatas tPixelDatas;
	struct VideoMem *ptNext;
}T_VideoMem, *PT_VideoMem;

typedef struct DispOpr {
	char *name;
	int iXres;
	int iYres;
	int iBpp;
	int iLineWidth;
	unsigned char* pucDispMem; /* framebuffer address */
	int (*DeviceInit)(void);
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);
	int (*CleanScreen)(unsigned int dwBackColor);
	int (*ShowPage)(PT_VideoMem ptVideoMem); /* 显示一页,数据源自ptVideoMem */
	struct DispOpr *ptNext;
}T_DispOpr, *PT_DispOpr;


void FlushVideoMemToDev(PT_VideoMem ptVideoMem);
int RegisterDispOpr(PT_DispOpr ptDispOpr);
void ShowDispOpr(void);
PT_DispOpr GetDispOpr(char *pcName);
int GetDispResolution(int* piXres, int *piYres, int* piBpp);
void SelectAndInitDefaultDispDev(char* name);
void SelectAndInitDefaultDispDev(char* name);
int GetDispResolution(int* piXres, int *piYres, int* piBpp);
void SelectAndInitDefaultDispDev(char* name);
PT_DispOpr GetDefaultDispDev(void);
int AllocVideoMem(int iNum);
PT_VideoMem GetVideoMem(int iID, int bCur);
void ReleaseVideoMem(PT_VideoMem ptVideoMem);
PT_VideoMem GetDevVideoMem(void);
void T_LayoutClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);
int DisplayInit(void);
int FBInit(void);

#endif /* _DISP_MANAGER_H */

