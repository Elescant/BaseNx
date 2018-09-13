/****************************************************************************
 * apps/gpsutils/minmea/minmea.c
 *
 * Copyright © 2014 Kosma Moczek <kosma@cloudyourcar.com>
 *
 * Released under the NuttX BSD license with permission from the author:
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/boardctl.h>
#include <sys/mount.h>
#include <fcntl.h>

int get_fontdata(int fd,char *buf,char * fontcode);

int main(void)
{
    int ret,fd;
    char fontbuf[32]={0};

    boardctl(BOARDIOC_INIT,0);

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
   sleep(1);
   printf("hello\n"); 
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