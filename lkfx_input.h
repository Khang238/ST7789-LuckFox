/*
 * lkfx_input.h — GPIO Button input với debounce + event queue
 *
 * ── Cách dùng cơ bản ────────────────────────────────────────────
 *
 *   // Đăng ký các nút (pin, active_low)
 *   lkfx_input_add_btn(59, 1);   // UP,   active low
 *   lkfx_input_add_btn(58, 1);   // DOWN, active low
 *   lkfx_input_add_btn(51, 1);   // OK
 *   lkfx_input_add_btn(49, 1);   // BACK
 *
 *   // Trong vòng lặp chính:
 *   lkfx_event_t ev;
 *   if (lkfx_input_poll(&ev)) {
 *       if (ev.type == LKFX_EV_PRESS && ev.btn_id == 0) {
 *           // nút UP vừa nhấn
 *       }
 *   }
 *
 * ── Hoặc chờ blocking ───────────────────────────────────────────
 *
 *   lkfx_event_t ev = lkfx_input_wait();  // block đến khi có nút
 *
 * ── Hoặc dùng shorthand ─────────────────────────────────────────
 *
 *   int btn = lkfx_input_wait_btn();  // trả về btn_id (0,1,2,...)
 *
 * ── Tạo layout nút tiện lợi ─────────────────────────────────────
 *
 *   // Khởi tạo 4 nút mặc định của board
 *   lkfx_input_init_default();
 *   // Sau đó dùng hằng:
 *   //   LKFX_BTN_UP, LKFX_BTN_DOWN, LKFX_BTN_OK, LKFX_BTN_BACK
 */

#ifndef LKFX_INPUT_H
#define LKFX_INPUT_H

#include <stdint.h>

/* ── ID nút mặc định (khi dùng lkfx_input_init_default) ── */
#define LKFX_BTN_UP    0
#define LKFX_BTN_DOWN  1
#define LKFX_BTN_OK    2
#define LKFX_BTN_BACK  3

/* Pin mặc định */
#define LKFX_PIN_UP    48
#define LKFX_PIN_DOWN  51
#define LKFX_PIN_OK    59
#define LKFX_PIN_BACK  58

#define LKFX_MAX_BTNS  8
#define LKFX_DEBOUNCE_MS 30

/* ── Loại sự kiện ── */
typedef enum {
    LKFX_EV_NONE  = 0,
    LKFX_EV_PRESS,      /* vừa nhấn xuống */
    LKFX_EV_RELEASE,    /* vừa thả ra */
    LKFX_EV_HOLD,       /* giữ lâu (> hold_ms) */
} lkfx_ev_type_t;

/* ── Struct sự kiện ── */
typedef struct {
    lkfx_ev_type_t type;
    int            btn_id;   /* index nút (theo thứ tự add) */
    int            pin;      /* pin GPIO thực */
} lkfx_event_t;

/* ================================================================
 * API
 * ================================================================ */

/*
 * Khởi tạo 4 nút mặc định (UP=59, DOWN=58, OK=51, BACK=49, active low).
 * Gọi hàm này HOẶC gọi lkfx_input_add_btn() thủ công.
 */
void lkfx_input_init_default(void);

/*
 * Thêm 1 nút.
 * pin:        GPIO pin number
 * active_low: 1 = nhấn kéo xuống 0 (phổ biến), 0 = nhấn kéo lên 1
 * Trả về btn_id (0,1,2,...) hoặc -1 nếu đầy.
 */
int lkfx_input_add_btn(int pin, int active_low);

/*
 * Đặt thời gian giữ để phát sự kiện HOLD (mặc định 600ms).
 */
void lkfx_input_set_hold_ms(int ms);

/*
 * Non-blocking poll — trả về 1 nếu có sự kiện, 0 nếu không.
 * Gọi thường xuyên trong vòng lặp chính.
 */
int lkfx_input_poll(lkfx_event_t *ev);

/*
 * Blocking wait — chờ đến khi có sự kiện PRESS hoặc HOLD.
 * Trả về event.
 */
lkfx_event_t lkfx_input_wait(void);

/*
 * Shorthand: chờ nhấn nút, trả về btn_id.
 * Bỏ qua RELEASE và HOLD.
 */
int lkfx_input_wait_btn(void);

/*
 * Kiểm tra 1 nút đang được nhấn không (instantaneous, no debounce).
 */
int lkfx_input_is_pressed(int btn_id);

/*
 * Dọn dẹp (đóng fd). Gọi trước khi thoát.
 */
void lkfx_input_deinit(void);

#endif /* LKFX_INPUT_H */
