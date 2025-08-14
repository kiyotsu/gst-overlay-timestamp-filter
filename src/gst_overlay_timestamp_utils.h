#ifndef __GST_OVERLAY_TIMESTAMP_UTILS_H__
#define __GST_OVERLAY_TIMESTAMP_UTILS_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

struct RGB
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

typedef enum
{
    TEXT_ALIGN_TOP_LEFT,
    TEXT_ALIGN_TOP_CENTER,
    TEXT_ALIGN_TOP_RIGHT,
    TEXT_ALIGN_CENTER_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_CENTER_RIGHT,
    TEXT_ALIGN_BOTTOM_LEFT,
    TEXT_ALIGN_BOTTOM_CENTER,
    TEXT_ALIGN_BOTTOM_RIGHT
} TextAlignment;

bool color_hex_string_to_rgb(struct RGB *dest, const char *hex);
bool rgb_to_color_hex_string(const struct RGB *src, char *hex_buffer, size_t hex_buffer_size);
int strfmttime(char *out_buf, size_t buf_size, const char *format, struct timeval *tv);
int current_strfmttime(char *out_buf, size_t buf_size, const char *format);
void calculate_aligned_position(TextAlignment alignment,
                                unsigned int container_width, unsigned int container_height,
                                unsigned int object_width, unsigned int object_height,
                                int margin, int *out_x, int *out_y);

#endif // __GST_OVERLAY_TIMESTAMP_UTILS_H__