#ifndef LCD_H_H
#define LCD_H_H

#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxfonts.h>

#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

struct lcd_data_s
{
  /* The NX handles */
  NXHANDLE hnx;
  NXHANDLE hbkgd;
  NXHANDLE hfont;
  bool connected;

  /* The screen resolution */

  nxgl_coord_t xres;
  nxgl_coord_t yres;

  volatile bool havepos;
  sem_t eventsem;
//   volatile int code;
};

int lcd_init(void);
uint16_t lcd_str(FAR struct nxgl_point_s pos,char *pstr);
void lcd_chinese(FAR struct nxgl_point_s pos, const void *pchin);
void lcd_chineses(FAR struct nxgl_point_s pos, const void *pchin,uint8_t words);
void lcd_clr_screen(void);
void lcd_clr_rect(struct nxgl_rect_s rect);

#endif
