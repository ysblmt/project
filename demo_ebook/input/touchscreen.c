#include <config.h>
#include <input_manager.h>
#include <stdlib.h>

#include <tslib.h>
#include <draw.h>

/* 参考tslib中的ts_print.c */

static struct tsdev *g_tTSDev;
static int giXres;
static int giYres;

static int TouchScreenDevInit(void)
{
	char *pcTSName = NULL;

	if ((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL ) 
	{
		g_tTSDev = ts_open(pcTSName, 0);  /* 以阻塞方式打开 */
	}
	else
	{
		g_tTSDev = ts_open("/dev/event0", 1); /* 默认打开 */
	}

	if (!g_tTSDev) {
		DBG_PRINTF(APP_ERR"ts_open error!\n");
		return -1;
	}

	if (ts_config(g_tTSDev)) {
		DBG_PRINTF("ts_config error!\n");
		return -1;
	}

	if (GetDispResolution(&giXres, &giYres)) /* 获取LCD分辨率 */
	{
		return -1;
	}

#ifdef SELECT_ON
	g_tTouchScreenOpr.iFd = ts_fd(g_tTSDev);
#endif

	return 0;
}

static int TouchScreenDevExit(void)
{
	return 0;
}

static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;
	struct ts_sample tSampPressed;
	struct ts_sample tSampReleased;
	int iRet;
	int bStart = 0;
	int iDelta;

	/* 通过记录按下滑动距离翻页 */
	static struct timeval tPreTime;

	while(1)
	{
		iRet = ts_read(g_tTSDev, &tSamp, 1);
		if(iRet == 1)
		{
			if((tSamp.pressure > 0) && (bStart == 0))
			{
				/* 刚按下，记录刚开始压下去的点 */
				tSampPressed = tSamp;
				bStart = 1;
			}

			if(tSamp.pressure <= 0)
			{
				/* 松开 */
				tSampReleased = tSamp;

				if(!bStart)
				{
					return -1;
				}
				else
				{
					iDelta = tSampReleased.x - tSampPressed.x; /* 记录距离 */
					ptInputEvent->tTime = tSampReleased.tv;
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					
					if (iDelta > giXres/5)
					{
						/* 翻到上一页 */
						ptInputEvent->iVal = INPUT_VALUE_UP;
					}
					else if (iDelta < 0 - giXres/5)
					{
						/* 翻到下一页 */
						ptInputEvent->iVal = INPUT_VALUE_DOWN;
					}
					else
					{
						ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
					}
					return 0;
				}
			}
		}
		else
		{
			return -1;
		}
	}
	return 0;
}



static T_InputOpr g_tTouchScreenOpr = {
	.name          = "touchscreen",
	.DeviceInit    = TouchScreenDevInit,
	.DeviceExit    = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}


