#ifndef FONT_H_H
#define FONT_H_H

#include <stdint.h>

#define ASCII_12 1
#define ASCII_16 2
#define CHINESE_12 3
#define CHINESE_16 4

struct fonts_glyph_s
{
    uint8_t fonttype;
    uint8_t height;
    uint8_t width;
    uint8_t stride;
};

int get_fontdata(int fd, char *buf, char *fontcode);

#endif
