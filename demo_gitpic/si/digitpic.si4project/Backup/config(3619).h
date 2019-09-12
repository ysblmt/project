
#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include <debug_manager.h>
#include <assert.h>

#define FB_DEVICE_NAME "/dev/fb0"

#define COLOR_BACKGROUND   0xE7DBB5  /* ·º»ÆµÄÖ½ */
#define COLOR_FOREGROUND   0x514438  /* ºÖÉ«×ÖÌå */
#define ICON_PATH "/etc/digitpic/icons"

#define FROM_SETTINGPAGE 1

//#define DBG_PRINTF(...)  
#define DBG_PRINTF DebugPrint

#endif /* _CONFIG_H */
