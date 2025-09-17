#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "fbdev.h"

void
fbdev_close(fbdev_t *dev)
{
    if (!dev) {
        return;
    }
    if (dev->map != MAP_FAILED) {
        munmap(dev->map, dev->maplen);
    }
    if (dev->fd != -1) {
        close(dev->fd);
    }
    free(dev);
}

fbdev_t*
fbdev_open(const char* restrict name)
{
    if (!name) {
        printf("No name was provided\n");
        return NULL;
    }

    fbdev_t *ret = malloc(sizeof(fbdev_t));
    if (!ret) {
        perror("malloc");
        return NULL;
    }

    ret->map = MAP_FAILED;
    ret->fd = open(name, O_RDWR);;
    if (ret->fd == -1) {
        perror("open");
        fbdev_close(ret);
        return NULL;
    }

    int pagesize = getpagesize();

    if (ioctl(ret->fd, FBIOGET_VSCREENINFO, &ret->var) < 0) {
        perror("FBIOGET_VSCREENINFO");
        fbdev_close(ret);
        return NULL;
    }

    if (ioctl(ret->fd, FBIOGET_FSCREENINFO, &ret->fix) < 0) {
        perror("FBIOGET_FSCREENINFO");
        fbdev_close(ret);
        return NULL;
    }

    ret->maplen = ((ret->fix.smem_len + pagesize - 1) / pagesize) * pagesize;

    ret->map = mmap(NULL, ret->maplen, PROT_READ | PROT_WRITE,
                    MAP_SHARED, ret->fd, 0);
    if (ret->map == MAP_FAILED) {
        perror("mmap");
        fbdev_close(ret);
        return NULL;
    }

    ret->bpp = ret->var.bits_per_pixel / CHAR_BIT;
    if (ret->bpp != 4 || CHAR_BIT != 8) {
        printf("Depths other than 32-bit with 8-bit bytes are not supported\n");
        fbdev_close(ret);
        return NULL;
    }

    ret->pitch = ret->fix.line_length ? ret->fix.line_length / ret->bpp : ret->var.xres_virtual;

    return ret;
}

int
fbdev_draw_image(fbdev_t* dev, void *image, int x_start, int y_start, int width, int height)
{
    int pitch = dev->pitch;
    int fb_width = dev->var.xres;
    int fb_height = dev->var.yres;
    if (fb_width < width + x_start ||
        fb_height < height + y_start) {
        return 0;
    }

    uint32_t *src = image;
    uint32_t *dst = dev->map;

    for(int i = 0; i < height; i++) {
        memcpy(dst + (i + y_start) * pitch + x_start, src + i * width, width * sizeof(uint32_t));
    }
    return 1;
}

int
fbdev_fill_color(fbdev_t* dev, uint32_t color, int x_start, int y_start, int width, int height)
{
    int pitch = dev->pitch;
    int fb_width = dev->var.xres;
    int fb_height = dev->var.yres;
    if (fb_width < width + x_start ||
        fb_height < height + y_start) {
        return 0;
    }

    uint32_t *mem = malloc(width * sizeof(uint32_t));
    if (!mem) {
        return 0;
    }

    uint32_t *src = mem;
    uint32_t *dst = dev->map;

    for(int i = 0; i < width; i++) {
        src[i] = color;
    }

    for(int i = 0; i < height; i++) {
        memcpy(dst + (i + y_start) * pitch + x_start, src, width * sizeof(uint32_t));
    }

    free(mem);
    return 1;
}
