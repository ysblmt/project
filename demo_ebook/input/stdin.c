#include "input_manager.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#ifdef SELECT_ON
	static T_InputOpr g_tStdinOpr;
#endif

static int StdinDevInit(void)
{
	struct termios tTTYState;

	tcgetattr(STDIN_FILENO,&tTTYState);//get the terminal state

	tTTYState.c_lflag &= ~ICANON;//turn off canonical mode
	//Canonical mode means it always wait for enter to confirms the user input
	tTTYState.c_cc[VMIN] = 1;//minimun of number input read
	//The line of ttystate.c_cc[VMIN] is set the minimum number of user input to accept

	tcgetattr(STDIN_FILENO, TCSANOW, &tTTYState);//set the terminal attributes.

#ifdef SELECT_ON
	g_tStdinOpr.iFd = STDIN_FILENO; /* 设置文件句柄 */
#endif	

	return 0;
}

static int StdinDevExit(void)
{
	struct termios tTTYState;

	tcgetattr(STDIN_FILENO,&tTTYState);//get the terminal state

	tTTYState.c_lflag |= ~ICANON;//turn on canonical mode

	tcgetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;
}

static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 如果有数据就读取、处理、返回
	 * 如果没有数据，立即返回，不等待
	 */

#ifdef SELECT_ON /* I/O多路复用 */
	struct timerval tTV;
	fd_set tFDs;
	char c;
	
    tTV.tv_sec = 0; /* 设置超时时间 */
    tTV.tv_usec = 0;

	FD_ZERO(&tFDs);
    FD_SET(STDIN_FILENO, &tFDs); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &tFDs, NULL, NULL, &tTV);
    
	if (FD_ISSET(STDIN_FILENO, &tFDs))
    {
		/* 处理数据 */
		ptInputEvent->iType = INPUT_TYPE_STDIN;
		gettimeofday(&ptInputEvent->tTime, NULL);
		
		c = fgetc(stdin);
		if (c == 'u')
		{
			ptInputEvent->iVal = INPUT_VALUE_UP;
		}
		else if (c == 'n')
		{
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		}
		else if (c == 'q')
		{
			ptInputEvent->iVal = INPUT_VALUE_EXIT;
		}
		else
		{
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}		
		return 0;
    }
	else
	{
		return -1;
	}
		
#else

	 char c;
	 ptInputEvent->iType = INPUT_TYPE_STDIN;

	 c = fgetc(stdin);/* 休眠直到有数据输入 */
	 gettimeofday(&ptInputEvent->tTime, NULL);/* 获取当前系统时间 */

	if (c == 'u')
	{
		ptInputEvent->iVal = INPUT_VALUE_UP;
	}
	else if (c == 'n')
	{
		ptInputEvent->iVal = INPUT_VALUE_DOWN;
	}
	else if (c == 'q')
	{
		ptInputEvent->iVal = INPUT_VALUE_EXIT;
	}
	else
	{
		ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
	}		
	return 0;

#endif
	
}


static T_InputOpr g_tStdinOpr = {
	.name			=	"stdin",
	.DeviceInit 	=	StdinDevInit,
	.DeviceExit 	=	StdinDevExit,
	.GetInputEvent	=	StdinGetInputEvent,
};


int StdinInit(void)
{
	return RegisterInputOpr(&g_tStdinOpr);
}


