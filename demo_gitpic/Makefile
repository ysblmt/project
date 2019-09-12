
CROSS_COMPILE = arm-linux-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

#去掉调试信息
STRIP		= $(CROSS_COMPILE)strip
#格式转换
OBJCOPY		= $(CROSS_COMPILE)objcopy
#反汇编
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

#C编译器选项
CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)/include
#链接器参数
LDFLAGS := -lm -lfreetype -lts -lpthread

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := show_file


obj-y += main.o
obj-y += display/
obj-y += draw/
obj-y += encoding/
obj-y += fonts/
obj-y += input/
obj-y += debug/

#进入子目录后才执行make，将一个指定普通文件作为makefile是，加上-f
all : 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	