/*
 * lkfx_widget.c — Hiện thực các widget UI
 */

#include "lkfx_widget.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* ================================================================
 * Theme mặc định
 * ================================================================ */
lkfx_theme_t lkfx_theme = {
    .bg        = 0x0000,
    .fg        = 0xFFFF,
    .title_bg  = 0x001F,
    .title_fg  = 0xFFFF,
    .sel_bg    = 0x03EF,
    .sel_fg    = 0xFFFF,
    .border    = 0x4228,
    .hint_fg   = 0x8410,
    .hint_bg   = 0x2104,
    .font      = NULL,   /* set lúc runtime bởi _theme_font() */
    .font_scale = 1,
};

/* Trả về font hiện tại, fallback về lkfx_font_6x8 nếu chưa set */
static const lkfx_font_t *_theme_font(void) {
    if (!lkfx_theme.font)
        lkfx_theme.font = &lkfx_font_6x8;
    return lkfx_theme.font;
}

void lkfx_theme_reset(void) {
    lkfx_theme.bg         = 0x0000;
    lkfx_theme.fg         = 0xFFFF;
    lkfx_theme.title_bg   = 0x001F;
    lkfx_theme.title_fg   = 0xFFFF;
    lkfx_theme.sel_bg     = 0x03EF;
    lkfx_theme.sel_fg     = 0xFFFF;
    lkfx_theme.border     = 0x4228;
    lkfx_theme.hint_fg    = 0x8410;
    lkfx_theme.hint_bg    = 0x2104;
    lkfx_theme.font       = &lkfx_font_6x8;
    lkfx_theme.font_scale = 1;
}

/* ================================================================
 * Helper nội bộ
 * ================================================================ */
#define TITLE_H  18
#define HINT_H   12

static void _draw_title(const char *title, uint16_t bg, uint16_t fg) {
    lkfx_fill_rect(0, 0, LKFX_W, TITLE_H, bg);
    if (title)
        lkfx_text_center(LKFX_W/2, (TITLE_H - lkfx_text_height(_theme_font(), lkfx_theme.font_scale))/2,
                         title, _theme_font(), lkfx_theme.font_scale,
                         fg, bg);
}

static void _draw_hint(const char *hint) {
    lkfx_fill_rect(0, LKFX_H - HINT_H, LKFX_W, HINT_H, lkfx_theme.hint_bg);
    if (hint)
        lkfx_text_center(LKFX_W/2,
                         LKFX_H - HINT_H + (HINT_H - lkfx_text_height(_theme_font(), 1))/2,
                         hint, _theme_font(), 1,
                         lkfx_theme.hint_fg, lkfx_theme.hint_bg);
}

static void _truncate(const char *src, char *dst, int max_chars) {
    int len = strlen(src);
    if (len <= max_chars) { strcpy(dst, src); return; }
    strncpy(dst, src, max_chars - 3);
    dst[max_chars - 3] = '\0';
    strcat(dst, "...");
}

/* ================================================================
 * Menu / List
 * ================================================================ */
static int _menu_impl(const char *title, const char **items,
                      const char **icons, int initial_sel) {
    /* Đếm items */
    int count = 0;
    while (items[count]) count++;
    if (count == 0) return -1;

    const lkfx_font_t *font = _theme_font();
    int fscale = lkfx_theme.font_scale;
    int fh     = lkfx_text_height(font, fscale);
    int list_y = title ? TITLE_H : 0;
    int list_h = LKFX_H - list_y - HINT_H;
    int item_h = fh + 6;
    int visible = list_h / item_h;
    if (visible < 1) visible = 1;

    int sel    = initial_sel < count ? initial_sel : 0;
    int scroll = 0;

    /* Max chars per item */
    int icon_w   = icons ? lkfx_text_width("    ", font, fscale) : 0;
    int max_chars = (LKFX_W - icon_w - 6 - (count > visible ? 4 : 0)) /
                    (font->glyph[0].x_advance * fscale);

    while (1) {
        /* Điều chỉnh scroll */
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + visible) scroll = sel - visible + 1;

        /* Vẽ nền */
        lkfx_fill_rect(0, 0, LKFX_W, LKFX_H, lkfx_theme.bg);

        /* Title */
        if (title)
            _draw_title(title, lkfx_theme.title_bg, lkfx_theme.title_fg);

        /* Items */
        for (int i = 0; i < visible; i++) {
            int idx = scroll + i;
            if (idx >= count) break;
            int iy     = list_y + i * item_h;
            int is_sel = (idx == sel);

            uint16_t ibg = is_sel ? lkfx_theme.sel_bg : lkfx_theme.bg;
            uint16_t ifg = is_sel ? lkfx_theme.sel_fg : lkfx_theme.fg;

            lkfx_fill_rect(0, iy, LKFX_W - (count > visible ? 4 : 0),
                           item_h, ibg);

            int tx = 4;
            /* Icon */
            if (icons && icons[idx]) {
                tx = lkfx_text(tx, iy + (item_h-fh)/2,
                               icons[idx], font, fscale, ifg, ibg);
            }
            /* Tên item */
            char trunc[48]; _truncate(items[idx], trunc, max_chars);
            lkfx_text(tx, iy + (item_h-fh)/2, trunc, font, fscale, ifg, ibg);

            /* Separator */
            if (!is_sel && i < visible-1 && scroll+i+1 < count)
                lkfx_draw_hline(4, iy + item_h - 1, LKFX_W-8, lkfx_theme.border);
        }

        /* Scrollbar */
        if (count > visible) {
            int sb_x  = LKFX_W - 3;
            int sb_y  = list_y;
            int sb_h  = list_h;
            int th_h  = sb_h * visible / count;
            if (th_h < 4) th_h = 4;
            int th_y  = sb_y + sb_h * scroll / count;
            lkfx_fill_rect(sb_x, sb_y, 3, sb_h, lkfx_theme.hint_bg);
            lkfx_fill_rect(sb_x, th_y, 3, th_h, lkfx_theme.fg);
        }

        /* Hint */
        _draw_hint("OK:select  BACK:cancel  U/D:nav");
        lkfx_fb_flush();

        /* Input */
        int btn = lkfx_input_wait_btn();
        if (btn == LKFX_BTN_UP)   { if (sel > 0) sel--; }
        else if (btn == LKFX_BTN_DOWN) { if (sel < count-1) sel++; }
        else if (btn == LKFX_BTN_OK)   { return sel; }
        else if (btn == LKFX_BTN_BACK) { return -1; }
    }
}

int lkfx_menu(const char *title, const char **items, int initial_sel) {
    return _menu_impl(title, items, NULL, initial_sel);
}

int lkfx_menu_icons(const char *title,
                    const char **items, const char **icons,
                    int initial_sel) {
    return _menu_impl(title, items, icons, initial_sel);
}

/* ================================================================
 * MessageBox
 * ================================================================ */
int lkfx_msgbox(const char *title, const char *message, int type) {
    /* Box kích thước */
    int bx = 10, by = 40, bw = LKFX_W-20, bh = LKFX_H-80;

    uint16_t title_bg = lkfx_theme.title_bg;
    if (type == LKFX_MB_ERROR) title_bg = LKFX_RED;

    /* Nền mờ (dim toàn màn hình) */
    for (int y = 0; y < LKFX_H; y++)
        for (int x = 0; x < LKFX_W; x++)
            lkfx_fb_set(x, y, lkfx_color_dim(lkfx_fb_get(x,y), 100));

    /* Box */
    lkfx_fill_round_rect(bx, by, bw, bh, 6, lkfx_theme.bg);
    lkfx_draw_round_rect(bx, by, bw, bh, 6, lkfx_theme.border);

    /* Title */
    if (title) {
        lkfx_fill_rect(bx+1, by+1, bw-2, TITLE_H, title_bg);
        lkfx_text_center(bx + bw/2, by + (TITLE_H - lkfx_text_height(_theme_font(), lkfx_theme.font_scale))/2,
                         title, _theme_font(), lkfx_theme.font_scale,
                         lkfx_theme.title_fg, title_bg);
    }

    /* Message — wrap text */
    if (message) {
        int ty = by + (title ? TITLE_H + 6 : 6);
        lkfx_text_wrap(bx+6, ty, bw-12, bh - (title ? TITLE_H+12 : 12) - 24,
                        message, _theme_font(), lkfx_theme.font_scale,
                        lkfx_theme.fg, lkfx_theme.bg);
    }

    /* Nút */
    const char *hint = (type == LKFX_MB_YESNO)
                       ? "OK:Yes  BACK:No"
                       : "OK / BACK:dismiss";
    int hint_y = by + bh - HINT_H - 4;
    lkfx_fill_rect(bx+1, hint_y, bw-2, HINT_H+4, lkfx_theme.hint_bg);
    lkfx_text_center(bx + bw/2, hint_y + 2, hint,
                     _theme_font(), 1,
                     lkfx_theme.hint_fg, lkfx_theme.hint_bg);

    lkfx_fb_flush();

    int btn = lkfx_input_wait_btn();
    return (btn == LKFX_BTN_OK) ? 1 : 0;
}

void lkfx_toast(const char *message, int timeout_ms) {
    if (!message) return;
    const lkfx_font_t *font = _theme_font();
    int fscale = lkfx_theme.font_scale;
    int tw = lkfx_text_width(message, font, fscale);
    int th = lkfx_text_height(font, fscale);
    int pw = tw + 16, ph = th + 10;
    int px = (LKFX_W - pw) / 2;
    int py = LKFX_H - ph - 20;

    /* Backup vùng sẽ bị ghi đè */
    /* (đơn giản: không backup, vẽ đè rồi redraw caller) */
    lkfx_fill_round_rect(px, py, pw, ph, 4, 0x2104);
    lkfx_draw_round_rect(px, py, pw, ph, 4, lkfx_theme.border);
    lkfx_text_center(px + pw/2, py + (ph-th)/2,
                     message, font, fscale,
                     lkfx_theme.fg, 0x2104);
    lkfx_fb_flush_rect(px, py, pw, ph);

    if (timeout_ms <= 0) {
        lkfx_input_wait_btn();
    } else {
        struct timespec ts = { timeout_ms/1000, (timeout_ms%1000)*1000000L };
        nanosleep(&ts, NULL);
    }
}

/* ================================================================
 * ProgressBar
 * ================================================================ */
void lkfx_progressbar_init(lkfx_progressbar_t *pb,
                            int x, int y, int w, int h,
                            uint16_t fg, uint16_t bg) {
    pb->x = x; pb->y = y; pb->w = w; pb->h = h;
    pb->fg = fg; pb->bg = bg;
    pb->border = lkfx_theme.border;
    pb->value = 0;
    pb->show_text = 0;
}

void lkfx_progressbar_set(lkfx_progressbar_t *pb, int value) {
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    pb->value = value;

    int filled = pb->w * value / 100;

    /* Viền */
    if (pb->border)
        lkfx_draw_rect(pb->x-1, pb->y-1, pb->w+2, pb->h+2, pb->border);

    /* Track */
    lkfx_fill_rect(pb->x, pb->y, pb->w, pb->h, pb->bg);

    /* Fill */
    if (filled > 0)
        lkfx_fill_rect(pb->x, pb->y, filled, pb->h, pb->fg);

    /* Text % */
    if (pb->show_text && pb->h >= lkfx_text_height(_theme_font(), 1)) {
        char buf[8]; snprintf(buf, sizeof(buf), "%d%%", value);
        lkfx_text_center(pb->x + pb->w/2,
                         pb->y + (pb->h - lkfx_text_height(_theme_font(),1))/2,
                         buf, _theme_font(), 1,
                         LKFX_WHITE, LKFX_TRANSPARENT);
    }
}

/* ================================================================
 * InputBox
 * ================================================================ */
static const char _default_charset[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789 !@#$%&*()-_+=./";

int lkfx_inputbox(const char *title, char *buf, int buf_len,
                  const char *charset) {
    if (!buf || buf_len < 2) return -1;
    if (!charset) charset = _default_charset;
    int clen = strlen(charset);

    buf[0] = '\0';
    int len = 0;
    int cur_char = 0;  /* index trong charset */

    const lkfx_font_t *font = _theme_font();
    int fscale = 2;  /* scale 2 cho dễ đọc */
    int fh = lkfx_text_height(font, fscale);

    while (1) {
        /* Vẽ khung */
        lkfx_fill_rect(0, 0, LKFX_W, LKFX_H, lkfx_theme.bg);
        if (title)
            _draw_title(title, lkfx_theme.title_bg, lkfx_theme.title_fg);

        /* Chuỗi đã nhập */
        int ty = TITLE_H + 10;
        lkfx_fill_rect(4, ty, LKFX_W-8, fh+4, lkfx_theme.hint_bg);
        lkfx_text(6, ty+2, buf, font, fscale, lkfx_theme.fg, lkfx_theme.hint_bg);
        /* Con trỏ nhấp nháy (luôn hiện) */
        int cursor_x = 6 + lkfx_text_width(buf, font, fscale);
        lkfx_draw_vline(cursor_x, ty+2, fh, lkfx_theme.sel_bg);

        /* Ký tự đang chọn */
        int cy2 = ty + fh + 16;
        char cur_str[2] = { charset[cur_char], '\0' };
        lkfx_text_center(LKFX_W/2, cy2,
                         "^", font, fscale, lkfx_theme.hint_fg, lkfx_theme.bg);
        lkfx_text_center(LKFX_W/2, cy2 + fh + 4,
                         cur_str, font, fscale*2, lkfx_theme.sel_fg, lkfx_theme.bg);
        lkfx_text_center(LKFX_W/2, cy2 + fh*4 + 8,
                         "v", font, fscale, lkfx_theme.hint_fg, lkfx_theme.bg);

        /* Hint */
        _draw_hint("UP/DN:char  OK:add  BACK:del/exit");
        lkfx_fb_flush();

        /* Input */
        lkfx_event_t ev = lkfx_input_wait();

        if (ev.type == LKFX_EV_PRESS) {
            if (ev.btn_id == LKFX_BTN_UP) {
                cur_char = (cur_char - 1 + clen) % clen;
            } else if (ev.btn_id == LKFX_BTN_DOWN) {
                cur_char = (cur_char + 1) % clen;
            } else if (ev.btn_id == LKFX_BTN_OK) {
                if (len < buf_len - 1) {
                    buf[len++] = charset[cur_char];
                    buf[len]   = '\0';
                }
            } else if (ev.btn_id == LKFX_BTN_BACK) {
                if (len > 0) {
                    /* Xóa ký tự cuối */
                    buf[--len] = '\0';
                } else {
                    return -1;  /* Hủy */
                }
            }
        } else if (ev.type == LKFX_EV_HOLD) {
            if (ev.btn_id == LKFX_BTN_OK) {
                /* Giữ OK = xác nhận xong */
                return len;
            }
            if (ev.btn_id == LKFX_BTN_BACK) {
                return -1;
            }
        }
    }
}
