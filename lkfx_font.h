/*
 * lkfx_font.h — Bitmap font nhiều kích cỡ
 *
 * ── Cách hoạt động ──────────────────────────────────────────────
 * Font được lưu dạng struct lkfx_font_t chứa:
 *  - Mảng bitmap (1 bit/pixel, MSB first)
 *  - Bảng glyph_info: offset + width thực của từng ký tự
 *  - first_char, last_char: phạm vi ASCII hỗ trợ
 *  - height: chiều cao cố định (pixel)
 *
 * ── Font có sẵn ─────────────────────────────────────────────────
 *  extern const lkfx_font_t lkfx_font_6x8;    // nhỏ, monospace
 *  extern const lkfx_font_t lkfx_font_8x13;   // vừa, monospace
 *  extern const lkfx_font_t lkfx_font_12x20;  // lớn, monospace
 *
 * ── Cách thêm font mới ──────────────────────────────────────────
 * 1. Dùng tool "fontconvert" (Adafruit GFX) hoặc script Python
 *    đi kèm (tools/ttf2lkfx.py) để convert TTF → .c
 *
 * 2. Cách thủ công với font bitmap:
 *    a. Thiết kế glyph trong mảng bitmap (xem lkfx_font_6x8.c mẫu)
 *    b. Khai báo lkfx_font_t với đúng width/height/bitmap
 *    c. extern const lkfx_font_t my_font; rồi dùng bình thường
 *
 * ── Scale font ──────────────────────────────────────────────────
 * Tất cả hàm vẽ text đều nhận tham số `scale` (1,2,3,...):
 *   scale=1 → glyph gốc
 *   scale=2 → mỗi pixel vẽ thành 2x2
 *   scale=3 → 3x3, v.v.
 * Đây là "pixel scaling" — nhanh, không cần thư viện ngoài.
 *
 * ── Ví dụ dùng ──────────────────────────────────────────────────
 *   // In chữ cỡ gốc
 *   lkfx_text(10, 20, "Hello!", &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);
 *
 *   // In chữ x2 (to gấp đôi)
 *   lkfx_text(10, 40, "BIG", &lkfx_font_6x8, 2, LKFX_GREEN, LKFX_BLACK);
 *
 *   // In không vẽ nền (transparent background)
 *   lkfx_text(10, 60, "Overlay", &lkfx_font_8x13, 1, LKFX_YELLOW, LKFX_TRANSPARENT);
 *
 *   // Căn giữa
 *   lkfx_text_center(120, 100, "CENTER", &lkfx_font_8x13, 1, LKFX_WHITE, LKFX_BLACK);
 *
 *   // Đo độ rộng chuỗi (để tự căn chỉnh)
 *   int w = lkfx_text_width("Hello", &lkfx_font_8x13, 1);
 */

#ifndef LKFX_FONT_H
#define LKFX_FONT_H

#include <stdint.h>
#include "lkfx_display.h"

/* ── Struct mô tả 1 glyph ── */
typedef struct {
    uint16_t bitmap_offset; /* vị trí byte đầu trong mảng bitmap */
    uint8_t  width;         /* chiều rộng glyph (pixel) */
    uint8_t  height;        /* chiều cao glyph (= font height thường) */
    uint8_t  x_advance;     /* bước tiến ngang sau ký tự này */
    int8_t   x_offset;      /* offset X khi vẽ (cho proportional font) */
    int8_t   y_offset;      /* offset Y khi vẽ (thường âm, tính từ baseline) */
} lkfx_glyph_t;

/* ── Struct mô tả font ── */
typedef struct {
    const uint8_t    *bitmap;    /* mảng bitmap tất cả glyph */
    const lkfx_glyph_t *glyph;  /* bảng glyph info */
    uint16_t first_char;         /* ASCII đầu tiên được hỗ trợ */
    uint16_t last_char;          /* ASCII cuối cùng */
    uint8_t  height;             /* chiều cao dòng (line height, pixel) */
    uint8_t  baseline;           /* khoảng cách từ top đến baseline */
} lkfx_font_t;

/* ── Font có sẵn ── */
extern const lkfx_font_t lkfx_font_6x8;   /* 6×8  — rất nhỏ, monospace */
extern const lkfx_font_t lkfx_font_8x13;  /* 8×13 — vừa, monospace     */
extern const lkfx_font_t lkfx_font_12x20; /* 12×20— lớn, monospace     */

/* ================================================================
 * API
 * ================================================================ */

/*
 * Vẽ 1 ký tự tại (x, y) = góc trên trái của glyph.
 * scale: 1=gốc, 2=x2, 3=x3
 * bg = LKFX_TRANSPARENT để không vẽ nền.
 * Trả về x_advance (bước tiến) × scale.
 */
int lkfx_char(int x, int y, char c,
               const lkfx_font_t *font, int scale,
               uint16_t fg, uint16_t bg);

/*
 * Vẽ chuỗi ký tự tại (x, y).
 * Trả về x sau ký tự cuối cùng.
 */
int lkfx_text(int x, int y, const char *str,
               const lkfx_font_t *font, int scale,
               uint16_t fg, uint16_t bg);

/*
 * Vẽ chuỗi căn giữa theo chiều ngang tại (cx, y).
 * cx = tọa độ X trung tâm.
 */
void lkfx_text_center(int cx, int y, const char *str,
                       const lkfx_font_t *font, int scale,
                       uint16_t fg, uint16_t bg);

/*
 * Vẽ chuỗi căn phải, kết thúc tại x = rx.
 */
void lkfx_text_right(int rx, int y, const char *str,
                      const lkfx_font_t *font, int scale,
                      uint16_t fg, uint16_t bg);

/*
 * Đo độ rộng chuỗi (pixel) ở scale cho trước.
 * Dùng để tự tính toán vị trí.
 */
int lkfx_text_width(const char *str, const lkfx_font_t *font, int scale);

/*
 * Chiều cao dòng của font ở scale cho trước.
 */
int lkfx_text_height(const lkfx_font_t *font, int scale);

/*
 * Vẽ chuỗi với xuống dòng tự động trong vùng (x,y,w,h).
 * Trả về số dòng đã vẽ.
 */
int lkfx_text_wrap(int x, int y, int w, int h,
                    const char *str,
                    const lkfx_font_t *font, int scale,
                    uint16_t fg, uint16_t bg);

#endif /* LKFX_FONT_H */
