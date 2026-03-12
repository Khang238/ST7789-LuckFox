/*
 * lkfx_gfx.c — Hiện thực vẽ hình học + ảnh
 */

#include "lkfx_gfx.h"
#include <stdlib.h>

/* ================================================================
 * Helper nội bộ
 * ================================================================ */
static inline void _px(int x, int y, uint16_t c) {
    lkfx_fb_set_safe(x, y, c);
}

static inline int _abs(int x) { return x < 0 ? -x : x; }
static inline int _min(int a, int b) { return a < b ? a : b; }
static inline int _max(int a, int b) { return a > b ? a : b; }
static inline void _swap(int *a, int *b) { int t=*a; *a=*b; *b=t; }

/* ================================================================
 * Hình học cơ bản
 * ================================================================ */
void lkfx_draw_hline(int x, int y, int w, uint16_t color) {
    if (y < 0 || y >= LKFX_H) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > LKFX_W) w = LKFX_W - x;
    for (int i = 0; i < w; i++) lkfx_fb_set(x+i, y, color);
}

void lkfx_draw_vline(int x, int y, int h, uint16_t color) {
    if (x < 0 || x >= LKFX_W) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > LKFX_H) h = LKFX_H - y;
    for (int i = 0; i < h; i++) lkfx_fb_set(x, y+i, color);
}

void lkfx_fill_rect(int x, int y, int w, int h, uint16_t color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LKFX_W) w = LKFX_W - x;
    if (y + h > LKFX_H) h = LKFX_H - y;
    if (w <= 0 || h <= 0) return;
    for (int row = y; row < y+h; row++)
        lkfx_draw_hline(x, row, w, color);
}

void lkfx_draw_rect(int x, int y, int w, int h, uint16_t color) {
    lkfx_draw_hline(x,     y,     w, color);
    lkfx_draw_hline(x,     y+h-1, w, color);
    lkfx_draw_vline(x,     y,     h, color);
    lkfx_draw_vline(x+w-1, y,     h, color);
}

/* Bresenham line */
void lkfx_draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx =  _abs(x1-x0), sx = x0 < x1 ? 1 : -1;
    int dy = -_abs(y1-y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        _px(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* Mid-point circle */
static void _circle_points(int cx, int cy, int x, int y,
                            uint16_t color, int fill) {
    if (fill) {
        lkfx_draw_hline(cx-x, cy+y, 2*x+1, color);
        lkfx_draw_hline(cx-x, cy-y, 2*x+1, color);
        lkfx_draw_hline(cx-y, cy+x, 2*y+1, color);
        lkfx_draw_hline(cx-y, cy-x, 2*y+1, color);
    } else {
        _px(cx+x, cy+y, color); _px(cx-x, cy+y, color);
        _px(cx+x, cy-y, color); _px(cx-x, cy-y, color);
        _px(cx+y, cy+x, color); _px(cx-y, cy+x, color);
        _px(cx+y, cy-x, color); _px(cx-y, cy-x, color);
    }
}

static void _circle(int cx, int cy, int r, uint16_t color, int fill) {
    int x = 0, y = r, d = 3 - 2*r;
    while (x <= y) {
        _circle_points(cx, cy, x, y, color, fill);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
}

void lkfx_fill_circle(int cx, int cy, int r, uint16_t color) {
    _circle(cx, cy, r, color, 1);
}

void lkfx_draw_circle(int cx, int cy, int r, uint16_t color) {
    _circle(cx, cy, r, color, 0);
}

/* Bo góc dùng quarter-circle */
static void _round_rect_corners(int x, int y, int w, int h,
                                 int r, uint16_t color, int fill) {
    int x1 = x + r, y1 = y + r;
    int x2 = x + w - 1 - r, y2 = y + h - 1 - r;
    int px = 0, py = r, d = 3 - 2*r;
    while (px <= py) {
        if (fill) {
            lkfx_draw_hline(x1-px, y1-py, (x2-x1)+2*px+1, color);
            lkfx_draw_hline(x1-px, y2+py, (x2-x1)+2*px+1, color);
            lkfx_draw_hline(x1-py, y1-px, (x2-x1)+2*py+1, color);
            lkfx_draw_hline(x1-py, y2+px, (x2-x1)+2*py+1, color);
        } else {
            _px(x1-px,y1-py,color); _px(x2+px,y1-py,color);
            _px(x1-px,y2+py,color); _px(x2+px,y2+py,color);
            _px(x1-py,y1-px,color); _px(x2+py,y1-px,color);
            _px(x1-py,y2+px,color); _px(x2+py,y2+px,color);
        }
        if (d < 0) d += 4*px + 6;
        else { d += 4*(px-py) + 10; py--; }
        px++;
    }
}

void lkfx_fill_round_rect(int x, int y, int w, int h, int r, uint16_t color) {
    if (r <= 0) { lkfx_fill_rect(x, y, w, h, color); return; }
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;
    /* Fill phần giữa */
    lkfx_fill_rect(x+r, y,   w-2*r, h,   color);
    lkfx_fill_rect(x,   y+r, r,     h-2*r, color);
    lkfx_fill_rect(x+w-r, y+r, r,   h-2*r, color);
    _round_rect_corners(x, y, w, h, r, color, 1);
}

void lkfx_draw_round_rect(int x, int y, int w, int h, int r, uint16_t color) {
    if (r <= 0) { lkfx_draw_rect(x, y, w, h, color); return; }
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;
    lkfx_draw_hline(x+r, y,     w-2*r, color);
    lkfx_draw_hline(x+r, y+h-1, w-2*r, color);
    lkfx_draw_vline(x,     y+r, h-2*r, color);
    lkfx_draw_vline(x+w-1, y+r, h-2*r, color);
    _round_rect_corners(x, y, w, h, r, color, 0);
}

/* Tam giác (scanline fill) */
void lkfx_draw_triangle(int x0,int y0, int x1,int y1,
                         int x2,int y2, uint16_t color) {
    lkfx_draw_line(x0,y0,x1,y1,color);
    lkfx_draw_line(x1,y1,x2,y2,color);
    lkfx_draw_line(x2,y2,x0,y0,color);
}

void lkfx_fill_triangle(int x0,int y0, int x1,int y1,
                         int x2,int y2, uint16_t color) {
    /* Sắp xếp theo y tăng dần */
    if (y0 > y1) { _swap(&x0,&x1); _swap(&y0,&y1); }
    if (y0 > y2) { _swap(&x0,&x2); _swap(&y0,&y2); }
    if (y1 > y2) { _swap(&x1,&x2); _swap(&y1,&y2); }
    /* Scanline */
    int total = y2 - y0;
    for (int y = y0; y <= y2; y++) {
        int a, b;
        if (y < y1) {
            int seg = y1 - y0 ? y1 - y0 : 1;
            a = x0 + (x2-x0) * (y-y0) / (total ? total : 1);
            b = x0 + (x1-x0) * (y-y0) / seg;
        } else {
            int seg = y2 - y1 ? y2 - y1 : 1;
            a = x0 + (x2-x0) * (y-y0) / (total ? total : 1);
            b = x1 + (x2-x1) * (y-y1) / seg;
        }
        if (a > b) _swap(&a, &b);
        lkfx_draw_hline(a, y, b-a+1, color);
    }
}

/* ================================================================
 * Ảnh
 * ================================================================ */
void lkfx_draw_image_rgb565(int x, int y, int w, int h, const uint8_t *data) {
    for (int row = 0; row < h; row++) {
        int dy = y + row;
        if (dy < 0 || dy >= LKFX_H) continue;
        for (int col = 0; col < w; col++) {
            int dx = x + col;
            if (dx < 0 || dx >= LKFX_W) continue;
            int idx = (row * w + col) * 2;
            uint16_t c = ((uint16_t)data[idx] << 8) | data[idx+1];
            lkfx_fb_set(dx, dy, c);
        }
    }
}

void lkfx_draw_image_1bpp(int x, int y, int w, int h,
                           const uint8_t *data,
                           uint16_t fg, uint16_t bg) {
    for (int row = 0; row < h; row++) {
        int dy = y + row;
        if (dy < 0 || dy >= LKFX_H) continue;
        for (int col = 0; col < w; col++) {
            int dx = x + col;
            if (dx < 0 || dx >= LKFX_W) continue;
            int bit_idx = row * w + col;
            int px = (data[bit_idx >> 3] >> (7 - (bit_idx & 7))) & 1;
            if (px) {
                lkfx_fb_set(dx, dy, fg);
            } else if (bg != LKFX_TRANSPARENT) {
                lkfx_fb_set(dx, dy, bg);
            }
        }
    }
}

void lkfx_draw_image_1bpp_alpha(int x, int y, int w, int h,
                                 const uint8_t *data,
                                 uint16_t fg, uint16_t bg_fill,
                                 int alpha) {
    uint16_t fg_dim = lkfx_color_dim(fg, alpha);
    for (int row = 0; row < h; row++) {
        int dy = y + row;
        if (dy < 0 || dy >= LKFX_H) continue;
        for (int col = 0; col < w; col++) {
            int dx = x + col;
            if (dx < 0 || dx >= LKFX_W) continue;
            int bit_idx = row * w + col;
            int px = (data[bit_idx >> 3] >> (7 - (bit_idx & 7))) & 1;
            lkfx_fb_set(dx, dy, px ? fg_dim : bg_fill);
        }
    }
}
