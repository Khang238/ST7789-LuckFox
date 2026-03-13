/*
 * lkfx_display.h — SPI + ST7789 + Framebuffer
 *
 * Module quản lý toàn bộ phần cứng hiển thị:
 *  - Khởi tạo SPI và ST7789
 *  - Framebuffer RGB565 trong RAM
 *  - Flush toàn màn hình hoặc từng vùng
 *
 * Cách dùng:
 *   lkfx_display_init();          // khởi tạo một lần
 *   lkfx_fb_fill(0x0000);         // xóa màn hình (đen)
 *   lkfx_fb_set(x, y, color);     // set 1 pixel
 *   lkfx_fb_flush();              // đẩy toàn bộ lên màn hình
 *   lkfx_fb_flush_rect(x,y,w,h);  // chỉ flush vùng cần thiết (nhanh hơn)
 */

#ifndef LKFX_DISPLAY_H
#define LKFX_DISPLAY_H

#include <stdint.h>

/* ── Kích thước màn hình ── */
#define LKFX_W   240
#define LKFX_H   240

/* ── Pin ST7789 ── */
#define LKFX_DC_PIN   57
#define LKFX_RST_PIN  56

/* ── SPI ── */
#define LKFX_SPI_DEV      "/dev/spidev0.0"
#define LKFX_SPI_SPEED_HZ  40000000
#define LKFX_SPI_CHUNK     4096

/* ── Macro màu RGB565 hay dùng ── */
#define LKFX_BLACK    0x0000
#define LKFX_WHITE    0xFFFF
#define LKFX_RED      0xF800
#define LKFX_GREEN    0x07E0
#define LKFX_BLUE     0x001F
#define LKFX_CYAN     0x07FF
#define LKFX_MAGENTA  0xF81F
#define LKFX_YELLOW   0xFFE0
#define LKFX_GRAY     0x8410
#define LKFX_DARKGRAY 0x2104

/* Giá trị đặc biệt: màu trong suốt (không vẽ background).
 * Dùng 0x0001 — không trùng với bất kỳ màu RGB565 thông thường nào
 * và đủ xa LKFX_BLACK (0x0000) để không nhầm lẫn. */
#define LKFX_TRANSPARENT 0x0001

/* Tạo màu RGB565 từ R8 G8 B8 */
#define LKFX_RGB(r,g,b) \
    (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

/* Làm tối màu theo alpha 0..256 */
static inline uint16_t lkfx_color_dim(uint16_t c, int alpha) {
    int r = ((c >> 11) & 0x1F) * alpha / 256;
    int g = ((c >>  5) & 0x3F) * alpha / 256;
    int b = ( c        & 0x1F) * alpha / 256;
    return (uint16_t)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F));
}

/* Blend 2 màu RGB565: t=0 → a, t=256 → b */
static inline uint16_t lkfx_color_blend(uint16_t a, uint16_t b, int t) {
    int ra = (a >> 11) & 0x1F, rb = (b >> 11) & 0x1F;
    int ga = (a >>  5) & 0x3F, gb = (b >>  5) & 0x3F;
    int ba =  a        & 0x1F, bb =  b        & 0x1F;
    int r  = ra + (rb - ra) * t / 256;
    int g  = ga + (gb - ga) * t / 256;
    int bv = ba + (bb - ba) * t / 256;
    return (uint16_t)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (bv & 0x1F));
}

/* ── API ── */

/* Khởi tạo SPI + GPIO + ST7789. Gọi 1 lần đầu tiên. */
void lkfx_display_init(void);

/* Dọn dẹp (đóng file descriptor). Gọi trước khi thoát. */
void lkfx_display_deinit(void);

/* Truy cập framebuffer trực tiếp (mảng RGB565 big-endian, W*H*2 bytes) */
uint8_t *lkfx_fb(void);

/* Set 1 pixel (không kiểm tra bounds để tốc độ — dùng lkfx_fb_set_safe nếu cần) */
static inline void lkfx_fb_set(int x, int y, uint16_t color) {
    extern uint8_t _lkfx_fb[LKFX_W * LKFX_H * 2];
    int i = (y * LKFX_W + x) << 1;
    _lkfx_fb[i]   = color >> 8;
    _lkfx_fb[i+1] = color & 0xFF;
}

/* Set 1 pixel có kiểm tra bounds */
static inline void lkfx_fb_set_safe(int x, int y, uint16_t color) {
    if ((unsigned)x < LKFX_W && (unsigned)y < LKFX_H)
        lkfx_fb_set(x, y, color);
}

/* Đọc màu 1 pixel từ framebuffer */
static inline uint16_t lkfx_fb_get(int x, int y) {
    extern uint8_t _lkfx_fb[LKFX_W * LKFX_H * 2];
    int i = (y * LKFX_W + x) << 1;
    return ((uint16_t)_lkfx_fb[i] << 8) | _lkfx_fb[i+1];
}

/* Fill toàn bộ framebuffer bằng 1 màu */
void lkfx_fb_fill(uint16_t color);

/* Đẩy toàn bộ framebuffer lên màn hình */
void lkfx_fb_flush(void);

/* Đẩy chỉ vùng hình chữ nhật (x,y,w,h) — nhanh hơn flush full */
void lkfx_fb_flush_rect(int x, int y, int w, int h);

/* Set VCOMS (0x00..0x3F). Tăng để màu đen đậm hơn, an toàn tối đa 0x33. */
void lkfx_set_vcoms(uint8_t val);

/*
 * Backlight control — PWM10 (/sys/class/pwm/pwmchip10), GPIO54
 * Gọi lkfx_bl_init() sau lkfx_display_init().
 * brightness: 0..255 (0=tắt, 255=sáng tối đa)
 */
void lkfx_bl_init(void);
void lkfx_bl_set(uint8_t brightness);  /* 0..255 */
void lkfx_bl_deinit(void);

#endif /* LKFX_DISPLAY_H */
