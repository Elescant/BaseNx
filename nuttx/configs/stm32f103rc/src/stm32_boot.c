/************************************************************************************
 * configs/fire-stm32v2/src/stm32_boot.c
 *
 *   Copyright (C) 2009, 2012, 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <nuttx/config.h>

#include <debug.h>

#include <nuttx/board.h>
#include <arch/board/board.h>
#include <sys/boardctl.h>
#include "up_arch.h"
#include "fire-stm32v2.h"
#include "stdio.h"

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/************************************************************************************
 * Private Functions
 ************************************************************************************/

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point.  This entry point
 *   is called early in the intitialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

void stm32_boardinitialize(void)
{
  /* Configure SPI chip selects if 1) SPI is not disabled, and 2) the weak function
   * stm32_spidev_initialize() has been brought into the link.
   */
#if defined(CONFIG_STM32_SPI1) || defined(CONFIG_STM32_SPI2)
  if (stm32_spidev_initialize)
    {
      stm32_spidev_initialize();
    }
#endif

  /* Initialize USB is 1) USBDEV is selected, 2) the USB controller is not
   * disabled, and 3) the weak function stm32_usbinitialize() has been brought
   * into the build.
   */

#if defined(CONFIG_USBDEV) && defined(CONFIG_STM32_USB)
  if (stm32_usbinitialize)
    {
      stm32_usbinitialize();
    }
#endif

  /* Configure on-board LEDs if LED support has been selected. */

#ifdef CONFIG_ARCH_LEDS
  board_autoled_initialize();
#endif
}

int board_app_initialize(uintptr_t arg)
{
  int ret;
  ret = stm32_sdinitialize(0);
  printf("sd initialize %d\n",ret);
  return ret;
}

#include <sys/mount.h>
#include <fcntl.h>

// int test_sd()
// {
//   int ret;
//   int fd;
//   char buf[15]={0};
//   // struct fat_format_s fmt = FAT_FORMAT_INITIALIZER;

//   printf("in main\n");
//   stm32_configgpio(DEBUG_LED);
//   boardctl(BOARDIOC_INIT,0);

//   // ret = mkfatfs(fullpath, &fmt);
//   // if(ret<0)
//   // {
//   //   printf("mkfatfs failed\n");
//   // }
//   ret = mount("/dev/mmcsd0","/mnt/sd0","vfat",0,NULL);
//   if(ret < 0)
//   {
//     printf("mount failed\n");
//   }else
//   {
//     printf("mount success\n");
//   }

//   fd = open("/mnt/sd0/test.txt", O_WRONLY|O_CREAT,0666);
//   printf("fd %d\n",fd);

//   if(fd>0)
//   {
//     ret = write(fd,"hello world!",sizeof("hello world!"));
//     printf("write ret %d\n",ret);
//     close(fd);
//     fd = open("/mnt/sd0/test.txt", O_RDONLY,0666);
//     ret = read(fd,buf,13);
//     printf("read %s\n",buf);
//     lseek(fd,1,SEEK_SET);
//     memset(buf,0,15);;
//     ret = read(fd,buf,12);
//     printf("read2 %s\n",buf);
//     close(fd);
//   }

//   while(1)
//   {
//     printf("in main\n");
//     stm32_gpiowrite(DEBUG_LED,0);
//     sleep(1);
//     stm32_gpiowrite(DEBUG_LED,1);
//     sleep(1);
//   }
// }
int get_fontdata(int fd,char *buf,char * fontcode);

/*
int main(void)
{
  int ret;
  int fd;
  char fontbuf[32]={0};
  // struct fat_format_s fmt = FAT_FORMAT_INITIALIZER;

  printf("in main\n");
  stm32_configgpio(DEBUG_LED);
  boardctl(BOARDIOC_INIT,0);

  // ret = mkfatfs(fullpath, &fmt);
  // if(ret<0)
  // {
  //   printf("mkfatfs failed\n");
  // }
  ret = mount("/dev/mmcsd0","/mnt/sd0","vfat",0,NULL);
  if(ret < 0)
  {
    printf("mount failed\n");
  }else
  {
    printf("mount success\n");
  }

  fd = open("/mnt/sd0/gbk_songti_16x16.Dzk", O_RDONLY,0666);
  printf("fd %d\n",fd);
  if(fd>0)
  {
    get_fontdata(fd,fontbuf,"ƻ");
  }
  uint8_t temp = 0x80;
  for(int y=0;y<32;y++)
  {
      if(y%2==0)
      {
        printf("\n");
      } 
      temp = 0x80;
      for(int i=0;i<8;i++)
      {
        if(fontbuf[y] & temp)
        {
          printf("*");
        }else
        {
          printf("-");
        }
        temp = temp>>1;
      }
  }

  while(1)
  {
    printf("in main\n");
    stm32_gpiowrite(DEBUG_LED,0);
    sleep(1);
    stm32_gpiowrite(DEBUG_LED,1);
    sleep(1);
  }
}

int get_fontdata(int fd,char *buf,char * fontcode)
{
  uint16_t bytesperfont = 32;
  uint32_t baseadd = 0;
  uint32_t oft = 0;
  uint8_t code1,code2;
  uint16_t fontx;

  int ret;

  fontx = *(uint16_t *)fontcode;
  code2 = fontx >> 8;
  code1 = fontx & 0xFF;

  printf("fontx %x,code2 %x,code1 %x\n",fontx,code2,code1);
  if( fontx < 0x80 )
  {
    baseadd = 0;
    oft = fontx*bytesperfont + baseadd;
  }else
  {
    baseadd = 0;
    // code2 = c>>8;
    // code1 = c&0xFF;
    oft = ((code1 - 0x81)*190 + (code2-0x40) - (code2/128))*bytesperfont + baseadd;
  }
  printf("oft %d\n",oft);
  ret = lseek(fd,oft,SEEK_SET);
  printf("lseek %d\n",ret);
  ret = read(fd,buf,bytesperfont);
  printf("read %d\n",ret);
  return ret;
}
*/