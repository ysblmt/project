/* 输出显示模块 */
#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

typedef struct DispOpr {
	char *name;
	int iXres;
	int iYres;
	int iBpp;
	int (*DeviceInit)(void); //初始化
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);//显示一个像素
	int (*CleanScreen)(unsigned int dwBackColor);//清屏
	struct DispOpr *ptNext; //链表指针
}T_DispOpr, *PT_DispOpr;

int RegisterDispOpr(PT_DispOpr ptDispOpr);
void ShowDispOpr(void);
int DisplayInit(void);
int FBInit(void);

#endif /* _DISP_MANAGER_H */

