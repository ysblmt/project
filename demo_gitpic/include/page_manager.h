#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <input_manager.h>
#include <disp_manager.h>

/* 参数管理，需要设置页面ID */
typedef struct PageParams {
    int iPageID;                  /* 页面的ID */
    char strCurPictureFile[256];  /* 要处理的第1个图片文件 */
}T_PageParams, *PT_PageParams;

/* 图标区域 */
typedef struct PageLayout{
	int iTopLeftX;        /* 这个区域的左上角、右下角坐标 */
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	int iBpp;
	int iMaxTotalBytes; /* 分配图标数据暂存内存时，开辟最大图标空间的内存，以防止内存越界 */
	pt_Layout atLayout; /* 数组: 这个区域分成好几个小区域 */
}T_PageLayout, *PT_PageLayout;

typedef struct PageAction{
	char* name;
	void (*Run)(PT_PageParams ptParentPageParams);
	int (*GetInputEvent)(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent);
	int (*Prepare)(void);
	struct PageAction *ptNext;
}T_PageAction, *PT_PageAction;

#endif

