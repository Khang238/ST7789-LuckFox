/*
 * lkfx_input.c — Hiện thực button input
 */

#include "lkfx_input.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/* ── Trạng thái 1 nút ── */
typedef struct {
    int  pin;
    int  active_low;
    int  fd;               /* sysfs value fd */
    int  last_state;       /* trạng thái đã debounce */
    int  raw_state;        /* trạng thái đọc thô */
    long debounce_start;   /* ms khi bắt đầu debounce */
    long press_start;      /* ms khi nhấn xuống */
    int  hold_sent;        /* đã gửi HOLD chưa */
} _btn_t;

static _btn_t  _btns[LKFX_MAX_BTNS];
static int     _btn_count  = 0;
static int     _hold_ms    = 600;

/* ── Thời gian ms ── */
static long _now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

/* ── GPIO sysfs ── */
static void _gpio_export(int pin) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", pin);
    if (access(path, F_OK) == 0) return;
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) return;
    char buf[8]; int n = snprintf(buf, sizeof(buf), "%d", pin);
    (void)write(fd, buf, n); close(fd);
    usleep(50000);
}

static void _gpio_set_in(int pin) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return;
    (void)write(fd, "in", 2); close(fd);
}

static int _gpio_open_read(int pin) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    return open(path, O_RDONLY);
}

/* Đọc raw state: trả về 1 nếu nút đang nhấn */
static int _read_raw(int btn_id) {
    _btn_t *b = &_btns[btn_id];
    char buf[4] = {0};
    lseek(b->fd, 0, SEEK_SET);
    read(b->fd, buf, 1);
    int val = buf[0] == '1';
    return b->active_low ? !val : val;
}

/* ================================================================
 * API
 * ================================================================ */
void lkfx_input_init_default(void) {
    lkfx_input_add_btn(LKFX_PIN_UP,   1);
    lkfx_input_add_btn(LKFX_PIN_DOWN, 1);
    lkfx_input_add_btn(LKFX_PIN_OK,   1);
    lkfx_input_add_btn(LKFX_PIN_BACK, 1);
}

int lkfx_input_add_btn(int pin, int active_low) {
    if (_btn_count >= LKFX_MAX_BTNS) return -1;
    _gpio_export(pin);
    _gpio_set_in(pin);
    int fd = _gpio_open_read(pin);
    if (fd < 0) { perror("gpio read"); return -1; }
    int id = _btn_count++;
    _btns[id].pin          = pin;
    _btns[id].active_low   = active_low;
    _btns[id].fd           = fd;
    _btns[id].last_state   = 0;
    _btns[id].raw_state    = 0;
    _btns[id].debounce_start = 0;
    _btns[id].press_start  = 0;
    _btns[id].hold_sent    = 0;
    return id;
}

void lkfx_input_set_hold_ms(int ms) {
    _hold_ms = ms;
}

int lkfx_input_poll(lkfx_event_t *ev) {
    if (!ev) return 0;
    ev->type   = LKFX_EV_NONE;
    ev->btn_id = -1;

    long now = _now_ms();

    for (int i = 0; i < _btn_count; i++) {
        _btn_t *b = &_btns[i];
        int raw = _read_raw(i);

        /* Debounce */
        if (raw != b->raw_state) {
            b->raw_state     = raw;
            b->debounce_start = now;
        }

        if (now - b->debounce_start < LKFX_DEBOUNCE_MS) continue;

        /* Trạng thái ổn định thay đổi */
        if (raw != b->last_state) {
            b->last_state = raw;
            ev->btn_id = i;
            ev->pin    = b->pin;
            if (raw) {
                /* PRESS */
                b->press_start = now;
                b->hold_sent   = 0;
                ev->type = LKFX_EV_PRESS;
            } else {
                /* RELEASE */
                ev->type = LKFX_EV_RELEASE;
            }
            return 1;
        }

        /* HOLD */
        if (b->last_state && !b->hold_sent &&
            now - b->press_start >= _hold_ms) {
            b->hold_sent = 1;
            ev->type   = LKFX_EV_HOLD;
            ev->btn_id = i;
            ev->pin    = b->pin;
            return 1;
        }
    }
    return 0;
}

lkfx_event_t lkfx_input_wait(void) {
    lkfx_event_t ev;
    while (1) {
        if (lkfx_input_poll(&ev) &&
            (ev.type == LKFX_EV_PRESS || ev.type == LKFX_EV_HOLD))
            return ev;
        usleep(5000);  /* poll mỗi 5ms */
    }
}

int lkfx_input_wait_btn(void) {
    return lkfx_input_wait().btn_id;
}

int lkfx_input_is_pressed(int btn_id) {
    if (btn_id < 0 || btn_id >= _btn_count) return 0;
    return _read_raw(btn_id);
}

void lkfx_input_deinit(void) {
    for (int i = 0; i < _btn_count; i++) {
        if (_btns[i].fd >= 0) close(_btns[i].fd);
    }
    _btn_count = 0;
}
