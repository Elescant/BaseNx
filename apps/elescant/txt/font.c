#include "font.h"
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

int get_fontdata(int fd, char *buf, char *fontcode)
{
    uint16_t bytesperfont = 32;
    uint32_t baseadd = 0;
    uint32_t oft = 0;
    uint8_t code1, code2;
    uint16_t fontx;
    uint8_t type_font = CHINESE_16;

    int ret;

    fontx = *(uint16_t *)fontcode;
    code2 = fontx >> 8;
    code1 = fontx & 0xFF;

    if (fontx < 0x80)
    {
        switch (type_font)
        {
        case ASCII_12:
            baseadd = 0x00000000;
            bytesperfont = 12*2;
            break;
        case ASCII_16:
            baseadd = 0x00000C00;
            bytesperfont = 16*2;
            break;
        }
        oft = fontx * bytesperfont + baseadd;
    }
    else
    {
        switch (type_font)
        {
        case CHINESE_12:
            baseadd = 0x00001C00;
            bytesperfont = 12*2;
            break;
        case CHINESE_16:
            baseadd = 0x0008E060;
            bytesperfont = 16*2;
            break;
        }
        // code2 = c>>8;
        // code1 = c&0xFF;
        oft = ((code1 - 0x81) * 190 + (code2 - 0x40) - (code2 / 128)) * bytesperfont + baseadd;
    }

    ret = lseek(fd, oft, SEEK_SET);
    ret = read(fd, buf, bytesperfont);

    return ret;
}