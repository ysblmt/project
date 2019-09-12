#include <config.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>


/* 声明函数，static实现自调用，起模块化封装作用 */
static int FBDeviceInit(void);
static int FBShowPixel(int iPenX,int iPenY,unsigned int dwColor);
static int FBCleanScreen(void);

static int g_fd;
static struct fb_var_screeninfo g_tFBVar;
static struct fb_fix_screeninfo g_tFBFix;
static unsigned int g_dwScreenSize;
static char *g_pucFBMem;

static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;


/* 构造、设置、注册结构体 */

static T_DispOpr g_tFBDispOpr = {
	.name = "fb",
	.DeviceInit = FBDeviceInit,
	.ShowPixel = FBShowPixel,
	.CleanScreen = FBCleanScreen,
};

/* 初始化函数 */
static int FBDeviceInit(void)
{
	int ret;
	
	/* LCD设备相关初始化；获取LCD参数 */
	g_fd = open(FB_DEVICE_NAME,O_RDWR); /* 打开设备 */
	if(0 > g_fd)
		DBG_PRINTF("can't open %s\n",FB_DEVICE_NAME);
	
	ret = ioctl(g_fd,FBIOGET_VSCREENINFO,&g_tFBVar); /* 获取可变参数 */
	if(ret < 0)
	{
		DBG_PRINTF("can't get fb's var\n");
		return -1;
	}
	
	ret = ioctl(g_fd,FBIOGET_FSCREENINFO,&g_tFBFix); /* 获取固定参数 */
	if(ret < 0)
	{
		DBG_PRINTF("can't get fb's fix\n");
		return -1;
	}
	
	g_dwScreenSize = g_tFBVar.xres * g_tFBVar.yres *g_tFBVar.bits_per_pixel / 8;
	/* void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset); */
	g_pucFBMem = (unsigned char *)mmap(NULL,g_dwScreenSize,PROT_READ | PROT_WRITE,MAP_SHARED,g_fd,0); /* 内存映射 */
	if (0 > g_pucFBMem)	
	{
		DBG_PRINTF("can't mmap\n");
		return -1;
	}
	
	g_tFBDispOpr.iXres  = g_tFBVar.xres; /* 获取像素 */
	g_tFBDispOpr.iYres  = g_tFBVar.yres;
	g_tFBDispOpr.iBpp   = g_tFBVar.bits_per_pixel;
	
	g_dwLineWidth		= g_tFBVar.xres * g_tFBVar.bits_per_pixel / 8;
	g_dwPixelWidth      = g_tFBVar.bits_per_pixel / 8;
	
	return 0;
}

/* 画点函数：根据不同BPP描点 (x,y,color)*/
static int FBShowPixel(int iPenX,int iPenY,unsigned int dwColor)
{
	unsigned char *pucFB8bpp;
	unsigned short *pucFB16bpp;
	unsigned int *pucFB32bpp;
	unsigned short wColor16bpp;
	int iRed;
	int iGreen;
	int iBlue;
	
	if ((iX >= g_tFBVar.xres) || (iY >= g_tFBVar.yres))
	{
		DBG_PRINTF("out of region\n");
		return -1;
	}
	
	pucFB8bpp   = g_pucFBMem + g_dwLineWidth * iY + g_dwPixelWidth * iX;
	pwFB16bpp   = (unsigned short *)pucFB;
	pdwFB32bpp  = (unsigned int *)pucFB;
	

	switch (g_tFBVar.bits_per_pixel)
		{
		case 8:
		{
			*pucFB8bpp = (unsigned char)dwColor;
			break;
		}
		case 16: /* 565 */
		{
			iRed 	  = (dwColor >> (16+3)) & 0x1f;
			iGreen	  = (dwColor >> (8+2)) & 0x3f;
			iBlue     = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwFB16bpp  =  wColor16bpp;
			break;
		}
		case 32:
		{
			*pdwFB32bpp = dwColor;
			break;
		}
		default:
		{
			DBG_PRINTF("can't support %d bpp\n",g_tFBVar.bits_per_pixel);
			return -1;
		}	
	}
	return 0;
}

/* 清除整个屏幕 */
static int FBCleanScreen(unsigned int dwBackColor)
{
	unsigned char *pucFB8bpp;
	unsigned short *pucFB16bpp;
	unsigned int *pucFB32bpp;
	unsigned short wColor16bpp;
	int iRed;
	int iGreen;
	int iBlue;
	int i;
	
	pucFB8bpp   = g_pucFBMem;
	pwFB16bpp   = (unsigned short *)pucFB;
	pdwFB32bpp  = (unsigned int *)pucFB;
	

	switch (g_tFBVar.bits_per_pixel)
		{
		case 8:
		{
			memset(g_pucFBMem, dwBackColor, g_dwScreenSize);
			break;
		}
		case 16: /* 565 */
		{
			iRed 	  = (dwBackColor >> (16+3)) & 0x1f;
			iGreen	  = (dwBackColor >> (8+2)) & 0x3f;
			iBlue     = (dwBackColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while(i < g_dwScreenSize)
			{
				*pwFB16bpp  =  wColor16bpp;
				pwFB16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while(i < g_dwScreenSize)
			{
				*pdwFB32bpp = dwBackColor;
				pdwFB32bpp++;
				i += 4;
			}
			break;
		}
		default:
		{
			DBG_PRINTF("can't support %d bpp\n",g_tFBVar.bits_per_pixel);
			return -1;
		}	
	}
	return 0;
}

/* 注册结构体 */
int FBInit(void)
{
	RegisterDispOpr(&g_tFBDispOpr);
}


