#include <config.h>
#include <render.h>
#include <stdlib.h>

static T_Layout g_atMainPageIconsLayout[] = {
	{0, 0, 0, 0, "browse_mode.bmp"},
	{0, 0, 0, 0, "continue_mod.bmp"},
	{0, 0, 0, 0, "setting.bmp"},
	{0, 0, 0, 0, NULL},
};

static T_PageLayout g_tMainPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_atMainPageIconsLayout,
};

static void  CalcMainPageLayout(PT_PageLayout ptPageLayout)
{
	int iXres;
	int iYres;
	int iBpp;
	int iStartY;
	int iWidth;
	int iHeight;
	int iTmpTotalBytes;
	pt_Layout atLayout;
	atLayout = ptPageLayout->atLayout;
	GetDispResolution(&iXres, &iYres, &iBpp);
	ptPageLayout->iBpp = iBpp;
	
	/*   
	 *    ----------------------
	 *                           1/2 * iHeight
	 *          browse_mode.bmp  iHeight
	 *                           1/2 * iHeight
	 *         continue_mod.bmp     iHeight
	 *                           1/2 * iHeight
	 *          setting.bmp       iHeight
	 *                           1/2 * iHeight
	 *    ----------------------
	 */
	iHeight = iYres * 2 / 10; /* 每个图标的高度 */
	iWidth	 = iHeight;
	iStartY = iHeight / 2;

	/* browse_mode.bmp  */
	atLayout[0].iTopLeftX 	= (iXres - iWidth * 2) / 2;
	atLayout[0].iTopLeftY 	= iStartY;
	atLayout[0].iBotRightX	= atLayout[0].iTopLeftX + iWidth * 2 - 1;
	atLayout[0].iBotRightY  = atLayout[0].iTopLeftY + iHeight - 1;
	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * iBpp / 8;
	if(ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* continue_mod.bmp */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + iHeight / 2 + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight - 1;
	atLayout[1].iTopLeftX  = (iXres - iWidth * 2) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth * 2 - 1;
	
	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* setting.bmp */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + iHeight / 2 + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight - 1;
	atLayout[2].iTopLeftX  = (iXres - iWidth * 2) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}
	
}


static void ShowMainPage(PT_PageLayout ptPageLayout)
{
	int iError;
	T_VideoMem ptVideoMem;

	pt_Layout atLayout = ptPageLayout->atLayout;

	/* 1、获取显存 */
	ptVideoMem = GetVideoMem(ID("main"), 1);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("can't get video mem for main page!\n");
		return;
	}
	/* 2、描画数据 */
	if(atLayout[0].iTopLeftX == 0)
	{
		/* 没有计算各图标的位置 */
		CalcMainPageLayout(ptPageLayout);
	}
	iError = GeneratePage(ptPageLayout, ptVideoMem);

	/* 3. 刷到设备上去 */
	FlushVideoMemToDev(ptVideoMem); 

	/* 4. 解放显存 */
	ReleaseVideoMem(ptVideoMem);
}

static int MainPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}


static void MainPageRun(PT_PageParams ptParentPageParams)
{
	T_InputEvent tInputEvent;
	T_PageParams tPageParams;

	tPageParams.iPageID = ID("main");
	
	/* 1、显示页面 */
	ShowMainPage(&g_tMainPageLayout);

	/* 2、线程准备数据 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
		iIndex = MainPageGetInputEvent(&g_tMainPageLayout, &tInputEvent);
		if (tInputEvent.iPressure == 0)
		{
			/* 如果是松开 */
			if (bPressed)
			{
				/* 曾经有按钮被按下 */
				ReleaseButton(&g_atMainPageIconsLayout[iIndexPressed]);
				bPressed = 0;

				if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
				{
					switch (iIndexPressed)
					{
						case 0: /* 浏览按钮 */
						{
							Page("browse")->Run(&tPageParams);

							/* 从设置页面返回后显示当首的主页面 */
							ShowMainPage(&g_tMainPageLayout);

							break;
						}
						case 1: /* 连播按钮 */
						{
							Page("auto")->Run(&tPageParams);

							/* 从设置页面返回后显示当首的主页面 */
							ShowMainPage(&g_tMainPageLayout);

							break;
						}
						case 2: /* 设置按钮 */
						{
							Page("setting")->Run(&tPageParams);

							/* 从设置页面返回后显示当首的主页面 */
							ShowMainPage(&g_tMainPageLayout);

							break;
						}
						default:
						{
							break;
						}
					}
				}
				
				iIndexPressed = -1;
			}
		}
		else
		{
			/* 按下状态 */
			if (iIndex != -1)
			{
				if (!bPressed)
				{
					/* 未曾按下按钮 */
					bPressed = 1;
					iIndexPressed = iIndex;
					PressButton(&g_atMainPageIconsLayout[iIndexPressed]); /* 改变显色 */
				}
			}
		}		
	}
}


static T_PageAction g_tMainPageAction = {
	.name          = "main",
	.Run           = MainPageRun,
	.GetInputEvent = MainPageGetInputEvent,
	//.Prepare       = MainPagePrepare;
}; 

int MainPageInit(void)
{
	return RegisterPageAction(&g_tMainPageAction);
}

