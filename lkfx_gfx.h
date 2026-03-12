/*
 * lkfx_gfx.h — Vẽ hình học cơ bản + ảnh
 *
 * Tất cả hàm đều vẽ vào framebuffer (không tự flush).
 * Sau khi vẽ xong, gọi lkfx_fb_flush() hoặc lkfx_fb_flush_rect().
 *
 * Hệ tọa độ: (0,0) = góc trên trái, x tăng sang phải, y tăng xuống dưới.
 */

#ifndef LKFX_GFX_H
#define LKFX_GFX_H

#include <stdint.h>
#include "lkfx_display.h"

/* ================================================================
 * Hình học cơ bản
 * ================================================================ */

/* Fill hình chữ nhật đặc */
void lkfx_fill_rect(int x, int y, int w, int h, uint16_t color);

/* Viền hình chữ nhật (không fill) */
void lkfx_draw_rect(int x, int y, int w, int h, uint16_t color);

/* Hình chữ nhật bo góc, radius = bán kính góc */
void lkfx_fill_round_rect(int x, int y, int w, int h, int radius, uint16_t color);
void lkfx_draw_round_rect(int x, int y, int w, int h, int radius, uint16_t color);

/* Đường thẳng (Bresenham) */
void lkfx_draw_line(int x0, int y0, int x1, int y1, uint16_t color);

/* Đường thẳng ngang / dọc (nhanh hơn) */
void lkfx_draw_hline(int x, int y, int w, uint16_t color);
void lkfx_draw_vline(int x, int y, int h, uint16_t color);

/* Hình tròn */
void lkfx_fill_circle(int cx, int cy, int r, uint16_t color);
void lkfx_draw_circle(int cx, int cy, int r, uint16_t color);

/* Tam giác */
void lkfx_fill_triangle(int x0,int y0, int x1,int y1, int x2,int y2, uint16_t color);
void lkfx_draw_triangle(int x0,int y0, int x1,int y1, int x2,int y2, uint16_t color);

/* ================================================================
 * Ảnh
 * ================================================================ */

/*
 * Vẽ ảnh RGB565 raw (big-endian, row-major).
 * data: mảng uint16_t hoặc uint8_t[w*h*2]
 * Dùng với ảnh convert bằng image2cpp (16-bit color mode)
 * hoặc tự convert bằng script Python.
 */
void lkfx_draw_image_rgb565(int x, int y, int w, int h, const uint8_t *data);

void lkfx_draw_image_1bpp(int x, int y, int w, int h,
                           const uint8_t *data,
                           uint16_t fg, uint16_t bg);

/*
 * Fade ảnh 1bpp với alpha 0..256
 * Dùng cho hiệu ứng fade-in/out logo
 */
void lkfx_draw_image_1bpp_alpha(int x, int y, int w, int h,
                                 const uint8_t *data,
                                 uint16_t fg, uint16_t bg_fill,
                                 int alpha);

#endif /* LKFX_GFX_H */
