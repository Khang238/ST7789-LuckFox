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
    gpio_write(rst_fd, 0); usleep(100000);
    gpio_write(rst_fd, 1); usleep(150000);
    st_cmd(0x01); usleep(150000);           /* SW reset */
    st_cmd(0x11); usleep(150000);           /* Sleep out */
    st_cmd(0x3A); { uint8_t d=0x55; st_data(&d,1); } usleep(10000); /* RGB565 */
    st_cmd(0x36); { uint8_t d=0x00; st_data(&d,1); }                /* MADCTL */
    st_cmd(0x21); usleep(10000);            /* Inversion ON */
    st_cmd(0x13); usleep(10000);            /* Normal mode */
    st_cmd(0x29); usleep(100000);           /* Display ON */
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
    /* Gửi từng dòng trong vùng */
    for (int row = y; row < y+h; row++)
        st_data(_lkfx_fb + (row * LKFX_W + x) * 2, w * 2);
}
