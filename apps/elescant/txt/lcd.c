#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/boardctl.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nxfonts.h>

#include "lcd.h"

#define RENDERER nxf_convert_1bpp

static struct lcd_data_s lcd_mana =
    {
        NULL,               /* hnx */
        NULL,               /* hbkgd */
        NULL,               /* hfont */
        false,              /* connected */
        0,                  /* xres */
        0,                  /* yres */
        false,              /* havpos */
        SEM_INITIALIZER(0), /* eventsem */
};
const struct nxfonts_glyph_s chinese_glyph = {NULL,0,16,16,2,{1}}; //chinese width,hight,stride

FAR void *lcd_listener(FAR void *arg);
static int lcd_config(struct lcd_data_s *handle);
void lcd_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                bool more, FAR void *arg);
void lcd_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                  FAR const struct nxgl_point_s *pos,
                  FAR const struct nxgl_rect_s *bounds,
                  FAR void *arg);

static const struct nx_callback_s lcd_cb =
        {
            lcd_redraw,  /* redraw */
            lcd_position /* position */
        };

int lcd_init(void)
{
    int ret;
    nxgl_mxpixel_t color = 0;

    ret = lcd_config(&lcd_mana);
    if (ret < 0 && !lcd_mana.hnx)
    {
        return ERROR;
    }

    lcd_mana.hfont = nxf_getfonthandle(NXFONT_DEFAULT);
    if (!lcd_mana.hfont)
    {
        return ERROR;
    }
    ret = nx_setbgcolor(lcd_mana.hnx, &color);
    ret = nx_requestbkgd(lcd_mana.hnx, &lcd_cb, &lcd_mana);
    while (!lcd_mana.havepos)
    {
        (void)sem_wait(&lcd_mana.eventsem);
    }
    return OK;
}

static int lcd_config(struct lcd_data_s *handle)
{
    struct sched_param param;
    pthread_t thread;
    int ret;

    /* Start the NX server kernel thread */

    ret = boardctl(BOARDIOC_NX_START, 0);
    if (ret < 0)
    {
        return ERROR;
    }
    /* Connect to the server */

    handle->hnx = nx_connect();
    if (handle->hnx)
    {
        pthread_attr_t attr;

        (void)pthread_attr_init(&attr);
        param.sched_priority = 80;
        (void)pthread_attr_setschedparam(&attr, &param);
        (void)pthread_attr_setstacksize(&attr, 2048);

        ret = pthread_create(&thread, &attr, lcd_listener, handle);
        if (ret != 0)
        {
            return ERROR;
        }

        while (!handle->connected)
        {
            (void)sem_wait(&handle->eventsem);
        }
    }
    else
    {
        return ERROR;
    }
    return OK;
}

FAR void *lcd_listener(FAR void *arg)
{
    int ret;
    struct lcd_data_s *handle;

    handle = (struct lcd_data_s *)arg;
    /* Process events forever */
    for (;;)
    {
        /* Handle the next event.  If we were configured blocking, then
       * we will stay right here until the next event is received.  Since
       * we have dedicated a while thread to servicing events, it would
       * be most natural to also select CONFIG_NX_BLOCKING -- if not, the
       * following would be a tight infinite loop (unless we added addition
       * logic with nx_eventnotify and sigwait to pace it).
       */

        ret = nx_eventhandler(handle->hnx);
        if (ret < 0)
        {
            /* An error occurred... assume that we have lost connection with
           * the server.
           */

            printf("nxhello_listener: Lost server connection: %d\n", errno);
            exit(1);
        }

        /* If we received a message, we must be connected */

        if (!handle->connected)
        {
            handle->connected = true;
            sem_post(&handle->eventsem);
            printf("nxhello_listener: Connected\n");
        }
    }
}

void lcd_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                bool more, FAR void *arg)
{
    ginfo("hwnd=%p rect={(%d,%d),(%d,%d)} more=%s\n",
          hwnd, rect->pt1.x, rect->pt1.y, rect->pt2.x, rect->pt2.y,
          more ? "true" : "false");
}

void lcd_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                  FAR const struct nxgl_point_s *pos,
                  FAR const struct nxgl_rect_s *bounds,
                  FAR void *arg)
{
    struct lcd_data_s *handle;

    handle = (struct lcd_data_s *)arg;
    /* Report the position */

    ginfo("hwnd=%p size=(%d,%d) pos=(%d,%d) bounds={(%d,%d),(%d,%d)}\n",
          hwnd, size->w, size->h, pos->x, pos->y,
          bounds->pt1.x, bounds->pt1.y, bounds->pt2.x, bounds->pt2.y);

    /* Have we picked off the window bounds yet? */

    if (!handle->havepos)
    {
        /* Save the background window handle */

        handle->hbkgd = hwnd;

        /* Save the window limits */

        handle->xres = bounds->pt2.x + 1;
        handle->yres = bounds->pt2.y + 1;

        handle->havepos = true;
        sem_post(&handle->eventsem);
        ginfo("Have xres=%d yres=%d\n", handle->xres, handle->yres);
    }
}

uint16_t lcd_str(FAR struct nxgl_point_s pos, char *pstr)
{
    FAR const struct nx_font_s *fontset;
    FAR const struct nx_fontbitmap_s *fbm;
    FAR uint8_t *glyph;
    FAR const char *ptr;
    FAR struct nxgl_rect_s dest;
    FAR const void *src[CONFIG_NX_NPLANES];
    unsigned int glyphsize;
    unsigned int mxstride;
    int ret;
    struct lcd_data_s *handle = &lcd_mana;

    fontset = nxf_getfontset(handle->hfont);
    mxstride = (fontset->mxwidth * 1 + 7) >> 3;
    glyphsize = (unsigned int)fontset->mxheight * mxstride;
    glyph = (FAR uint8_t *)malloc(glyphsize);

    for (ptr = pstr; *ptr; ptr++)
    {
        /* Get the bitmap font for this ASCII code */

        fbm = nxf_getbitmap(handle->hfont, *ptr);
        if (fbm)
        {
            uint8_t fheight; /* Height of this glyph (in rows) */
            uint8_t fwidth;  /* Width of this glyph (in pixels) */
            uint8_t fstride; /* Width of the glyph row (in bytes) */

            /* Get information about the font bitmap */

            fwidth = fbm->metric.width + fbm->metric.xoffset;
            fheight = fbm->metric.height + fbm->metric.yoffset;
            fstride = (fwidth * 1 + 7) >> 3;

            /* Initialize the glyph memory to the background color */

            // nxhello_initglyph(glyph, fheight, fwidth, fstride);
            memset(glyph, 0, glyphsize);
            /* Then render the glyph into the allocated memory */

            (void)RENDERER((FAR nxgl_mxpixel_t *)glyph, fheight, fwidth,
                           fstride, fbm, 1);

            /* Describe the destination of the font with a rectangle */

            dest.pt1.x = pos.x;
            dest.pt1.y = pos.y;
            dest.pt2.x = pos.x + fwidth - 1;
            dest.pt2.y = pos.y + fheight - 1;

            /* Then put the font on the display */

            src[0] = (FAR const void *)glyph;

            ret = nx_bitmap((NXWINDOW)handle->hbkgd, &dest, src, &pos, fstride);
            if (ret < 0)
            {
                printf("nx_bitmapwindow failed: %d\n", errno);
            }

            /* Skip to the right the width of the font */

            pos.x += fwidth;
        }
        else
        {
            /* No bitmap (probably because the font is a space).  Skip to the
            * right the width of a space.
            */

            pos.x += fontset->spwidth;
        }
    }

    /* Free the allocated glyph */

    free(glyph);
    return pos.x;
}

void lcd_chinese(FAR struct nxgl_point_s pos, const void *pchin)
{
    FAR struct nxgl_rect_s dest;

    FAR const void *src[1];

    dest.pt1.x = pos.x;
    dest.pt1.y = pos.y;

    dest.pt2.x = pos.x + chinese_glyph.width - 1;
    dest.pt2.y = pos.y + chinese_glyph.height - 1;

    src[0] = (FAR const void *)pchin;
    nx_bitmap((NXWINDOW)lcd_mana.hbkgd, &dest, src, &pos, chinese_glyph.stride);
}

void lcd_chineses(FAR struct nxgl_point_s pos, const void *pchin,uint8_t words)
{
    FAR struct nxgl_rect_s dest;
    FAR const void *src[1];
    uint8_t *pfont;

    pfont = (uint8_t *)pchin;
    if (pos.x >= lcd_mana.xres || pos.y >= lcd_mana.yres)
    {
        return;
    }

    for (int num = 0; num < words; num++)
    {
        dest.pt1.x = pos.x;
        dest.pt1.y = pos.y;

        dest.pt2.x = pos.x + chinese_glyph.width - 1;
        dest.pt2.y = pos.y + chinese_glyph.height - 1;

        src[0] = (FAR const void *)(pfont +(num*chinese_glyph.stride*chinese_glyph.height));
        nx_bitmap((NXWINDOW)lcd_mana.hbkgd, &dest, src, &pos, chinese_glyph.stride);

        pos.x = chinese_glyph.width + pos.x;

        if (pos.x >= lcd_mana.xres)
        {
            break;
        }
    }
}


void lcd_clr_screen(void)
{
    struct nxgl_rect_s rect = {{0, 0}, {0, 0}};
    nxgl_mxpixel_t wcolor[1]={0};

    rect.pt2.x = lcd_mana.xres;
    rect.pt2.y = lcd_mana.yres;
    nx_fill((NXWINDOW)lcd_mana.hbkgd, &rect, wcolor);
}

void lcd_clr_rect(struct nxgl_rect_s rect)
{   
    nxgl_mxpixel_t wcolor[1]={0};
    
    nx_fill((NXWINDOW)lcd_mana.hbkgd, &rect, wcolor);
}