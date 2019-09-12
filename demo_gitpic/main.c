#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <string.h>

int main(int argc, char **argv)
{

	
	/* 注册调试通道 */
	DebugInit();

	/* 初始化调试通道 */
	InitDebugChanel();
	
	AllocVideoMem(5);

	/* 注册显示设备 */
	DisplayInit();
	/* 可能可支持多个显示设备: 选择和初始化指定的显示设备 */
	SelectAndInitDefaultDispDev("fb");

	Page("main")->Run(NULL);
	
	return 0;
}

