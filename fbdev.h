#ifndef __FBDEV_H__
#define __FBDEV_H__

#include <stdint.h>
#include <linux/fb.h>

typedef struct fbdev {
    void *map;
    int maplen;
    int bpp;
    int pitch;
    int fd;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
} fbdev_t;

void
fbdev_close(fbdev_t *dev);

fbdev_t*
fbdev_open(const char* restrict name);

int
fbdev_draw_image(fbdev_t* dev, void *image, int x_start, int y_start, int width, int height);

int
fbdev_fill_color(fbdev_t* dev, uint32_t color, int x_start, int y_start, int width, int height);

#endif /* __FBDEV_H__ */
