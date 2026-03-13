/*
 * lkfx_display.c — Hiện thực SPI + ST7789 + Framebuffer
 */

#include "lkfx_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

/* ── Framebuffer toàn cục ── */
uint8_t _lkfx_fb[LKFX_W * LKFX_H * 2];

/* ── File descriptors ── */
static int spi_fd  = -1;
static int dc_fd   = -1;
static int rst_fd  = -1;

/* ================================================================
 * GPIO sysfs
 * ================================================================ */
static void gpio_export(int pin) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", pin);
    if (access(path, F_OK) == 0) return;
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) { perror("gpio export"); return; }
    char buf[8];
    int n = snprintf(buf, sizeof(buf), "%d", pin);
    (void)write(fd, buf, n);
    close(fd);
    usleep(50000);
}

static void gpio_set_dir(int pin, const char *dir) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) { perror("gpio direction"); return; }
    (void)write(fd, dir, strlen(dir));
    close(fd);
}

static int gpio_open_value(int pin) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) perror("gpio value");
    return fd;
}

static inline void gpio_write(int fd, int val) {
    (void)write(fd, val ? "1" : "0", 1);
}

/* ================================================================
 * SPI
 * ================================================================ */
static void spi_write_buf(const uint8_t *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        size_t chunk = len - off > LKFX_SPI_CHUNK ? LKFX_SPI_CHUNK : len - off;
        struct spi_ioc_transfer tr = {
            .tx_buf        = (unsigned long)(buf + off),
            .rx_buf        = 0,
            .len           = chunk,
            .speed_hz      = LKFX_SPI_SPEED_HZ,
            .bits_per_word = 8,
            .delay_usecs   = 0,
        };
        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
            perror("spi transfer");
        off += chunk;
    }
}

/* ================================================================
 * ST7789 low-level
 * ================================================================ */
static inline void st_cmd(uint8_t c) {
    gpio_write(dc_fd, 0);
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)&c,
        .len           = 1,
        .speed_hz      = LKFX_SPI_SPEED_HZ,
        .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

static inline void st_data(const uint8_t *buf, size_t len) {
    gpio_write(dc_fd, 1);
    spi_write_buf(buf, len);
}

static void st_set_window(int x0, int y0, int x1, int y1) {
    uint8_t d[4];
    st_cmd(0x2A);
    d[0]=x0>>8; d[1]=x0&0xFF; d[2]=x1>>8; d[3]=x1&0xFF;
    st_data(d, 4);
    st_cmd(0x2B);
    d[0]=y0>>8; d[1]=y0&0xFF; d[2]=y1>>8; d[3]=y1&0xFF;
    st_data(d, 4);
    st_cmd(0x2C);
}

static void st_init_sequence(void) {
    /* Hardware reset — khớp timing TFT_eSPI: HIGH 5ms → LOW 20ms → HIGH 150ms */
    gpio_write(rst_fd, 1); usleep(5000);
    gpio_write(rst_fd, 0); usleep(20000);
    gpio_write(rst_fd, 1); usleep(150000);

    /* Sleep out */
    st_cmd(0x11); usleep(120000);

    /* Normal display mode on */
    st_cmd(0x13);

    /* MADCTL — RGB order, khớp với TFT_eSPI */
    { uint8_t d = 0x08; st_cmd(0x36); st_data(&d, 1); }

    /* Display function control */
    { uint8_t d[] = {0x0A, 0x82}; st_cmd(0xB6); st_data(d, 2); }

    /* RAMCTRL — 5-to-6-bit conversion, đúng như TFT_eSPI */
    { uint8_t d[] = {0x00, 0xE0}; st_cmd(0xB0); st_data(d, 2); }

    /* COLMOD — RGB565 */
    { uint8_t d = 0x55; st_cmd(0x3A); st_data(&d, 1); } usleep(10000);

    /* PORCTRL — frame rate */
    { uint8_t d[] = {0x0C,0x0C,0x00,0x33,0x33}; st_cmd(0xB2); st_data(d, 5); }

    /* GCTRL — VGH/VGL */
    { uint8_t d = 0x35; st_cmd(0xB7); st_data(&d, 1); }

    /* VCOMS */
    { uint8_t d = 0x19; st_cmd(0xBB); st_data(&d, 1); }

    /* LCMCTRL */
    { uint8_t d = 0x0C; st_cmd(0xC0); st_data(&d, 1); }

    /* VDVVRHEN */
    { uint8_t d[] = {0x01, 0xFF}; st_cmd(0xC2); st_data(d, 2); }

    /* VRHS */
    { uint8_t d = 0x10; st_cmd(0xC3); st_data(&d, 1); }

    /* VDVSET */
    { uint8_t d = 0x20; st_cmd(0xC4); st_data(&d, 1); }

    /* FRCTR2 */
    { uint8_t d = 0x0F; st_cmd(0xC6); st_data(&d, 1); }

    /* PWCTRL1 */
    { uint8_t d[] = {0xA4, 0xA1}; st_cmd(0xD0); st_data(d, 2); }

    /* Positive gamma */
    { uint8_t d[] = {0xD0,0x00,0x02,0x07,0x0A,0x28,0x32,
                     0x44,0x42,0x06,0x0E,0x12,0x14,0x17};
      st_cmd(0xE0); st_data(d, 14); }

    /* Negative gamma */
    { uint8_t d[] = {0xD0,0x00,0x02,0x07,0x0A,0x28,0x31,
                     0x54,0x47,0x0E,0x1C,0x17,0x1B,0x1E};
      st_cmd(0xE1); st_data(d, 14); }

    /* Inversion ON */
    st_cmd(0x21);

    /* Column address set: 0..239 */
    { uint8_t d[] = {0x00,0x00,0x00,0xEF}; st_cmd(0x2A); st_data(d, 4); }

    /* Row address set: 0..239 */
    { uint8_t d[] = {0x00,0x00,0x00,0xEF}; st_cmd(0x2B); st_data(d, 4); }

    usleep(120000);

    /* Display on */
    st_cmd(0x29); usleep(120000);
}

/* ================================================================
 * API công khai
 * ================================================================ */
void lkfx_display_init(void) {
    /* GPIO */
    gpio_export(LKFX_DC_PIN);  gpio_set_dir(LKFX_DC_PIN,  "out");
    gpio_export(LKFX_RST_PIN); gpio_set_dir(LKFX_RST_PIN, "out");
    dc_fd  = gpio_open_value(LKFX_DC_PIN);
    rst_fd = gpio_open_value(LKFX_RST_PIN);

    /* SPI */
    spi_fd = open(LKFX_SPI_DEV, O_RDWR);
    if (spi_fd < 0) { perror("open spi"); exit(1); }
    uint8_t  mode  = SPI_MODE_3;
    uint32_t speed = LKFX_SPI_SPEED_HZ;
    uint8_t  bits  = 8;
    ioctl(spi_fd, SPI_IOC_WR_MODE,          &mode);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ,  &speed);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    /* ST7789 */
    st_init_sequence();

    /* Xóa framebuffer */
    memset(_lkfx_fb, 0, sizeof(_lkfx_fb));
}

void lkfx_display_deinit(void) {
    if (spi_fd >= 0) { close(spi_fd); spi_fd = -1; }
    if (dc_fd  >= 0) { close(dc_fd);  dc_fd  = -1; }
    if (rst_fd >= 0) { close(rst_fd); rst_fd = -1; }
}

uint8_t *lkfx_fb(void) {
    return _lkfx_fb;
}

void lkfx_fb_fill(uint16_t color) {
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (int i = 0; i < LKFX_W * LKFX_H * 2; i += 2) {
        _lkfx_fb[i]   = hi;
        _lkfx_fb[i+1] = lo;
    }
}

void lkfx_fb_flush(void) {
    st_set_window(0, 0, LKFX_W-1, LKFX_H-1);
    st_data(_lkfx_fb, sizeof(_lkfx_fb));
}

void lkfx_fb_flush_rect(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LKFX_W) w = LKFX_W - x;
    if (y + h > LKFX_H) h = LKFX_H - y;
    if (w <= 0 || h <= 0) return;

    st_set_window(x, y, x+w-1, y+h-1);
    for (int row = y; row < y+h; row++)
        st_data(_lkfx_fb + (row * LKFX_W + x) * 2, w * 2);
}

void lkfx_set_vcoms(uint8_t val) {
    if (val > 0x3F) val = 0x3F;
    gpio_write(dc_fd, 0);
    uint8_t cmd = 0xBB;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&cmd, .len = 1,
        .speed_hz = LKFX_SPI_SPEED_HZ, .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    gpio_write(dc_fd, 1);
    tr.tx_buf = (unsigned long)&val;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

/* ================================================================
 * Backlight — PWM10 (/sys/class/pwm/pwmchip10)
 * Period: 1000000ns (1kHz), duty 0..1000000
 * ================================================================ */
#define BL_PWM_PATH  "/sys/class/pwm/pwmchip10"
#define BL_PERIOD_NS 1000000

static void bl_write(const char *path, const char *val) {
    FILE *f = fopen(path, "w");
    if (f) { fputs(val, f); fclose(f); }
}

void lkfx_bl_init(void) {
    bl_write(BL_PWM_PATH "/export", "0");
    usleep(50000); /* chờ kernel tạo pwm0/ */
    bl_write(BL_PWM_PATH "/pwm0/period",   "1000000");
    bl_write(BL_PWM_PATH "/pwm0/polarity", "normal");
    bl_write(BL_PWM_PATH "/pwm0/duty_cycle", "0");
    bl_write(BL_PWM_PATH "/pwm0/enable",   "1");
}

void lkfx_bl_set(uint8_t brightness) {
    /* map 0..255 → 0..1000000 ns */
    int duty = (int)brightness * 1000000 / 255;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", duty);
    bl_write(BL_PWM_PATH "/pwm0/duty_cycle", buf);
}

void lkfx_bl_deinit(void) {
    bl_write(BL_PWM_PATH "/pwm0/duty_cycle", "0");
    bl_write(BL_PWM_PATH "/pwm0/enable",     "0");
    bl_write(BL_PWM_PATH "/unexport",        "0");
}
