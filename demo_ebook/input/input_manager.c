
#include <config.h>
#include <input_manager.h>
#include <string.h>

#ifdef SELECT_ON
	static fd_set g_tRFds;
	static int g_iMaxFd = -1;
#endif

static PT_InputOpr g_ptInputOprHead;
static T_InputEvent g_tInputEvent;

static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;

int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTmp;

	if (!g_ptInputOprHead)
	{
		g_ptInputOprHead   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptInputOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}

	return 0;
}


void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

static void *InputEventTreadFunction(void *pVoid) /* 子线程处理函数 */
{
	T_InputEvent tInputEvent;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid; /* 类型转换，将底层的输入设备的GetInputEvent传过来 */

	while (1)
	{
		if(0 == GetInputEvent(&tInputEvent))
		{
			/* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			/* 访问临界资源(全局变量)前，先获得互斥量 */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  唤醒主线程 */
			pthread_cond_signal(&g_tConVar);

			/* 释放互斥量 */
			pthread_mutex_unlock(&g_tMutex);
		}
	}
	return NULL;
}

int AllInputDevicesInit(void)
{
#ifdef SELECT_ON
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	FD_ZERO(&g_tRFds);

	while (ptTmp)
	{
		if (0 == ptTmp->DeviceInit())
		{
			FD_SET(ptTmp->iFd, &g_tRFds);
			if (g_iMaxFd < ptTmp->iFd)
				g_iMaxFd = ptTmp->iFd;
			iError = 0;
		}
		ptTmp = ptTmp->ptNext;
	}

	g_iMaxFd++; /* 最大文件句柄+1 */
	return iError;
#endif

	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	while (ptTmp)
	{
		if (0 == ptTmp->DeviceInit()) /* 初始化设备 */
		{
			/* 创建子线程：将ptTmp->GetInputEvent作参数传给InputEventTreadFunction函数 */
			pthread_create(&ptTmp->tTreadID, NULL, InputEventTreadFunction, ptTmp->GetInputEvent);			
			iError = 0;
		}
		ptTmp = ptTmp->ptNext;
	}
	return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent) /* 主函数调用 */
{
#ifdef SELECT_ON
	/* 用select函数监测stdin,touchscreen,
	   有数据时再调用它们的GetInputEvent或获得具体事件
	 */

	PT_InputOpr ptTmp = g_ptInputOprHead;
	fd_set tRFds;
	int iRet;

	tRFds = g_tRFds;

	iRet = select(g_iMaxFd, &tRFds, NULL, NULL, NULL);
	if (iRet > 0)
	{
		while (ptTmp)
		{
			if (FD_ISSET(ptTmp->iFd, &tRFds))
			{
				if(0 == ptTmp->GetInputEvent(ptInputEvent))
				{
					return 0;
				}
			}
			ptTmp = ptTmp->ptNext;
		}
	}

	return -1;
#endif

	/* 休眠 */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* 被唤醒后,返回数据 */
	*ptInputEvent = g_tInputEvent;
	pthread_mutex_unlock(&g_tMutex);
	return 0;	
}

int InputInit(void)
{
	int iError;
	iError = StdinInit();
	iError |= TouchScreenInit();
	return iError;
}


