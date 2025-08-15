#include "gst_overlay_timestamp_utils.h"

bool color_hex_string_to_rgb(struct RGB *dest, const char *hex) {
    if (!dest || !hex || hex[0] != '#' || strlen(hex) != 7) {
        return false;
    }

    for (int i = 1; i < 7; ++i) {
        if (!isxdigit(hex[i])) {
            return false;
        }
    }

    unsigned int r, g, b;
    if (sscanf(hex + 1, "%02x%02x%02x", &r, &g, &b) != 3) {
        return false;
    }

    dest->red = (uint8_t)r;
    dest->green = (uint8_t)g;
    dest->blue = (uint8_t)b;

    return true;
}

bool rgb_to_color_hex_string(const struct RGB *src, char *hex_buffer, size_t hex_buffer_size) {
    if (!hex_buffer || hex_buffer_size < 8) {
        return false;
    }

    int written_chars =
        snprintf(hex_buffer, hex_buffer_size, "#%02X%02X%02X", src->red, src->green, src->blue);

    return written_chars == 7;
}

int strfmttime(char *out_buf, size_t buf_size, const char *format, struct timeval *tv) {
    char *p_out = out_buf;
    const char *p_fmt = format;
    size_t remaining = buf_size;
    int written;
    struct tm tm_info = {0};

    if (!out_buf || buf_size == 0 || !format || !tv) {
        return -1;
    }

    localtime_r(&tv->tv_sec, &tm_info);

    while (*p_fmt && remaining > 1) {
        if (*p_fmt == '%') {
            p_fmt++;
            switch (*p_fmt) {
                case 'Y':
                    written = snprintf(p_out, remaining, "%d", tm_info.tm_year + 1900);
                    break;
                case 'm':
                    written = snprintf(p_out, remaining, "%02d", tm_info.tm_mon + 1);
                    break;
                case 'd':
                    written = snprintf(p_out, remaining, "%02d", tm_info.tm_mday);
                    break;
                case 'H':
                    written = snprintf(p_out, remaining, "%02d", tm_info.tm_hour);
                    break;
                case 'M':
                    written = snprintf(p_out, remaining, "%02d", tm_info.tm_min);
                    break;
                case 'S':
                    written = snprintf(p_out, remaining, "%02d", tm_info.tm_sec);
                    break;
                case 'q':
                    written = snprintf(p_out, remaining, "%03ld", tv->tv_usec / 1000);
                    break;
                case '%':
                    written = snprintf(p_out, remaining, "%%");
                    break;
                default:
                    written = snprintf(p_out, remaining, "%%%c", *p_fmt);
                    break;
            }
            if (written > 0 && written < remaining) {
                p_out += written;
                remaining -= written;
            } else if (written >= remaining) {
                // snprintfで実際に書き込まれた文字列長を算出する
                written = remaining - 1;
                p_out += written;
                remaining -= written;
                goto end;
            }
        } else {
            *p_out++ = *p_fmt;
            remaining--;
        }
        p_fmt++;
    }

end:
    // 終端NULL文字は保証する
    if (remaining == 0) {
        out_buf[buf_size - 1] = '\0';
        p_out--;
    } else {
        *p_out = '\0';
    }
    return (p_out - out_buf);
}

int current_strfmttime(char *out_buf, size_t buf_size, const char *format) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return strfmttime(out_buf, buf_size, format, &tv);
}

void calculate_aligned_position(TextAlignment alignment, unsigned int container_width,
                                unsigned int container_height, unsigned int object_width,
                                unsigned int object_height, int margin, int *out_x, int *out_y) {
    if (!out_x || !out_y) {
        return;
    }

    int x = 0, y = 0;

    // 水平方向の計算
    switch (alignment) {
        case TEXT_ALIGN_TOP_CENTER:
        case TEXT_ALIGN_CENTER:
        case TEXT_ALIGN_BOTTOM_CENTER:
            x = (container_width - object_width) / 2;
            break;
        case TEXT_ALIGN_TOP_RIGHT:
        case TEXT_ALIGN_CENTER_RIGHT:
        case TEXT_ALIGN_BOTTOM_RIGHT:
            x = container_width - object_width - margin;
            break;
        case TEXT_ALIGN_TOP_LEFT:
        case TEXT_ALIGN_CENTER_LEFT:
        case TEXT_ALIGN_BOTTOM_LEFT:
        default:
            x = margin;
            break;
    }

    // 垂直方向の計算
    switch (alignment) {
        case TEXT_ALIGN_CENTER_LEFT:
        case TEXT_ALIGN_CENTER:
        case TEXT_ALIGN_CENTER_RIGHT:
            y = (container_height) / 2;
            break;
        case TEXT_ALIGN_BOTTOM_LEFT:
        case TEXT_ALIGN_BOTTOM_CENTER:
        case TEXT_ALIGN_BOTTOM_RIGHT:
            y = container_height - margin;
            break;
        case TEXT_ALIGN_TOP_LEFT:
        case TEXT_ALIGN_TOP_CENTER:
        case TEXT_ALIGN_TOP_RIGHT:
        default:
            y = margin + object_height;
            break;
    }

    *out_x = x;
    *out_y = y;
}
