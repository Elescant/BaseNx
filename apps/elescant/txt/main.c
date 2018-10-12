/****************************************************************************
 * apps/gpsutils/minmea/minmea.c
 *
 * Copyright Â© 2014 Kosma Moczek <kosma@cloudyourcar.com>
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
#include "lcd.h"
#include <errno.h>
#include "font.h"

void test_lcd(void);
void uart_chinese(void *buf);
int get_fontdata(int fd, char *buf, char *fontcode);

int main(void)
{
  int ret, fd, txtfd;
  char fontbuf[4] = {0};
  char wordbuf[32] = {0};//32 bytes per word 
  struct nxgl_point_s pos = {0, 0};

  lcd_init();
  boardctl(BOARDIOC_INIT, 0);

  ret = mount("/dev/mmcsd0", "/mnt/sd0", "vfat", 0, NULL);
  if (ret < 0)
  {
    printf("mount failed\n");
  }
  else
  {
    printf("mount success\n");
  }
  fd = open("/mnt/sd0/font.bin", O_RDONLY, 0666);
  if (fd < 0)
  {
    printf("fd %d\n", fd);
    return;
    // get_fontdata(fd, fontbuf+32, "¹û");
  }

  txtfd = open("/mnt/sd0/sanguoyanyi.txt", O_RDONLY, 0666);
  if (txtfd < 0)
  {
    printf("txtfd %d\n", txtfd);
  }

  while (1)
  {
    memset(fontbuf, 0, 2);
    memset(wordbuf, 0, 32);
    ret = read(txtfd, fontbuf, 2);
    if (fontbuf[0] == '\n' && ret!=1)
    {
      printf("new line\n");
      lseek(txtfd, -1, SEEK_CUR);
      if (pos.y + 16 <= 63)
      {
        pos.y += 16;
        pos.x = 0;
      }
      else
      {
        sleep(5);
        lcd_clr_screen();
        pos.y = 0;
        pos.x = 0;
      }
      continue;
    }
    if (ret == 2)
    {
      get_fontdata(fd, wordbuf, fontbuf);
      lcd_chinese(pos, wordbuf);
      if (pos.x + 16 <= 127)
      {
        pos.x += 16;
      }
      else
      {
        pos.x = 0;
        if (pos.y + 16 <= 63)
        {
          pos.y += 16;
        }
        else
        {
          sleep(5);
          lcd_clr_screen();
          pos.y = 0;
        }
      }
    }
    else
    {
      printf("clr\n");
      sleep(5);
      lcd_clr_screen();
      lseek(txtfd, 0, SEEK_SET);
      pos.x = 0;
      pos.y = 0;
    }
  }
}



void test_lcd(void)
{
  struct nxgl_point_s pos = {20, 20};
  lcd_init();

  lcd_str(pos, "hello world!");
  while (1)
  {
    sleep(1);
    printf("hello\n");
  }
}

void uart_chinese(void *buf)
{
  uint8_t temp = 0x80;
  uint8_t *fobuf = (uint8_t *)buf;

  for (int y = 0; y < 32; y++)
  {
    if (y % 2 == 0)
    {
      printf("\n");
    }
    temp = 0x80;
    for (int i = 0; i < 8; i++)
    {
      if (fobuf[y] & temp)
      {
        printf("*");
      }
      else
      {
        printf("-");
      }
      temp = temp >> 1;
    }
  }
}
