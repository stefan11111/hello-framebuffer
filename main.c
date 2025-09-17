#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "fbdev.h"
#include "letters.h"

void*
scale_image(void* image, int src_x, int src_y, int dst_x, int dst_y)
{
    uint32_t *dst = malloc(dst_x * dst_y * sizeof(uint32_t));
    uint32_t *src = image;
    if (!dst) {
        return NULL;
    }

#define DST(i,j) dst[i * dst_x + j]
#define SRC(i,j) src[i * src_x + j]

    float x_ratio = src_x / (float)dst_x;
    float y_ratio = src_y / (float)dst_y;
    for(int i = 0; i < dst_y; i++) {
        for(int j = 0; j < dst_x; j++) {
            DST(i,j) = SRC((int)(i * y_ratio), (int)(j * x_ratio));
        }
    }

#undef DST
#undef SRC

    return dst;
}

int main()
{
    fbdev_t *dev = fbdev_open("/dev/fb");
    for(int i = 0; i < 31 && !dev; i++) {
        char device[] = "/dev/fbxx";
        snprintf(device, sizeof(device), "/dev/fb%d", i);
        fbdev_open(device);
    }

    if (!dev) {
        return 0;
    }

    uint32_t blue;
    uint8_t *p = (uint8_t*)&blue;
    p[0] = 0xff;
    p[1] = p[2] = p[3] = 0x00;

    struct timespec duration = {.tv_sec = 0,
                                .tv_nsec = 8000000, /* 1/125 sec */
                               };

#define SCALE_IMAGE(x) \
    void *image_##x = scale_image(scalable_##x.buf, scalable_##x.width, scalable_##x.height, 100, 300); \
    if (!image_##x) { \
        printf("Could not scale image\n"); \
        return 0; \
    }

    SCALE_IMAGE(h);
    SCALE_IMAGE(e);
    SCALE_IMAGE(l);
 /* SCALE_IMAGE(l); */
    SCALE_IMAGE(o);

    SCALE_IMAGE(space);

    SCALE_IMAGE(w);
 /* SCALE_IMAGE(o); */
    SCALE_IMAGE(r);
 /* SCALE_IMAGE(l); */
    SCALE_IMAGE(d);
    SCALE_IMAGE(ex);

#define DRAW_IMAGE(x) \
        if (!fbdev_draw_image(dev, image_##x, 100 * idx, 100, 100, 300)) { \
            printf("Could not paint framebuffer\n"); \
            break; \
        } \
        idx++;



    for(;;) {
        int idx = 0;

        DRAW_IMAGE(h);
        DRAW_IMAGE(e);
        DRAW_IMAGE(l);
        DRAW_IMAGE(l);
        DRAW_IMAGE(o);

        DRAW_IMAGE(space);

        DRAW_IMAGE(w);
        DRAW_IMAGE(o);
        DRAW_IMAGE(r);
        DRAW_IMAGE(l);
        DRAW_IMAGE(d);
        DRAW_IMAGE(ex);

        nanosleep(&duration, NULL);
    }

#undef SCALE_IMAGE
#undef DRAW_IMAGE

    fbdev_close(dev);
    return 0;
}
