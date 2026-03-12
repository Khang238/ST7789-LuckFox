/*
 * lkfx_widget.h — Widget UI sẵn: List, Menu, MessageBox, ProgressBar
 *
 * Tất cả widget đều:
 *  - Vẽ vào framebuffer (không tự flush)
 *  - Trả về kết quả sau khi người dùng tương tác
 *  - Tự xử lý input bên trong (blocking)
 *
 * ── Ví dụ nhanh ─────────────────────────────────────────────────
 *
 *   // Menu đơn giản
 *   const char *items[] = { "Play video", "Settings", "About", NULL };
 *   int choice = lkfx_menu("Main Menu", items, 0);
 *   // choice = 0,1,2 hoặc -1 nếu BACK
 *
 *   // Hộp xác nhận
 *   int ok = lkfx_msgbox("Delete file?", "This cannot be undone.",
 *                         LKFX_MB_YESNO);
 *
 *   // Progress bar (không blocking — tự cập nhật)
 *   lkfx_progressbar_t pb;
 *   lkfx_progressbar_init(&pb, 20, 200, 200, 8, LKFX_GREEN, LKFX_DARKGRAY);
 *   lkfx_progressbar_set(&pb, 50);   // 50%
 *   lkfx_fb_flush_rect(pb.x, pb.y, pb.w+2, pb.h+2);
 */

#ifndef LKFX_WIDGET_H
#define LKFX_WIDGET_H

#include <stdint.h>
#include "lkfx_display.h"
#include "lkfx_font.h"
#include "lkfx_gfx.h"
#include "lkfx_input.h"

/* ── Theme toàn cục (có thể override trước khi dùng widget) ── */
typedef struct {
    uint16_t bg;           /* nền chính */
    uint16_t fg;           /* chữ chính */
    uint16_t title_bg;     /* nền title bar */
    uint16_t title_fg;     /* chữ title bar */
    uint16_t sel_bg;       /* nền item được chọn */
    uint16_t sel_fg;       /* chữ item được chọn */
    uint16_t border;       /* viền */
    uint16_t hint_fg;      /* chữ hint bar dưới */
    uint16_t hint_bg;      /* nền hint bar */
    const lkfx_font_t *font;
    int font_scale;
} lkfx_theme_t;

/* Theme mặc định — có thể sửa trực tiếp */
extern lkfx_theme_t lkfx_theme;

/* Khôi phục theme gốc */
void lkfx_theme_reset(void);

/* ================================================================
 * Widget: List / Menu
 * ================================================================ */

/*
 * Hiển thị danh sách có thể cuộn, người dùng chọn 1 item.
 *
 * title:       chuỗi tiêu đề (NULL = không có title bar)
 * items:       mảng chuỗi, kết thúc bằng NULL
 * initial_sel: item được chọn ban đầu (0-based)
 *
 * Trả về: index item được chọn (0..n-1)
 *         -1 nếu người dùng nhấn BACK
 *
 * Điều hướng: UP/DOWN di chuyển, OK xác nhận, BACK hủy.
 */
int lkfx_menu(const char *title, const char **items, int initial_sel);

/*
 * Phiên bản có icon prefix cho mỗi item.
 * icons: mảng chuỗi icon (ví dụ "[F]", "[D]", ">"), cùng size với items.
 */
int lkfx_menu_icons(const char *title,
                    const char **items, const char **icons,
                    int initial_sel);

/* ================================================================
 * Widget: MessageBox
 * ================================================================ */

/* Loại messagebox */
#define LKFX_MB_OK        0   /* chỉ nút OK */
#define LKFX_MB_YESNO     1   /* Yes / No */
#define LKFX_MB_INFO      2   /* OK, icon info */
#define LKFX_MB_ERROR     3   /* OK, title đỏ */

/*
 * Hiển thị hộp thông báo.
 *
 * title:   tiêu đề
 * message: nội dung (hỗ trợ \n)
 * type:    LKFX_MB_OK, LKFX_MB_YESNO, LKFX_MB_INFO, LKFX_MB_ERROR
 *
 * Trả về: 1 nếu OK/Yes, 0 nếu No/BACK
 */
int lkfx_msgbox(const char *title, const char *message, int type);

/*
 * Thông báo ngắn tự tắt sau timeout_ms.
 * timeout_ms = 0 → đợi nhấn nút.
 */
void lkfx_toast(const char *message, int timeout_ms);

/* ================================================================
 * Widget: ProgressBar (non-blocking)
 * ================================================================ */
typedef struct {
    int      x, y, w, h;
    uint16_t fg;         /* màu thanh đã fill */
    uint16_t bg;         /* màu track chưa fill */
    uint16_t border;     /* màu viền (0 = không viền) */
    int      value;      /* 0..100 */
    int      show_text;  /* hiện "xx%" */
} lkfx_progressbar_t;

/* Khởi tạo progressbar */
void lkfx_progressbar_init(lkfx_progressbar_t *pb,
                            int x, int y, int w, int h,
                            uint16_t fg, uint16_t bg);

/* Cập nhật giá trị (0..100) và vẽ lại vào framebuffer */
void lkfx_progressbar_set(lkfx_progressbar_t *pb, int value);

/* ================================================================
 * Widget: InputBox (nhập chuỗi bằng nút)
 * ================================================================ */
/*
 * Cho phép người dùng nhập chuỗi ký tự bằng UP/DOWN để chọn ký tự,
 * OK để xác nhận ký tự, BACK để xóa.
 *
 * title:   tiêu đề
 * buf:     buffer để lưu kết quả
 * buf_len: kích thước buffer
 * charset: tập ký tự (NULL = dùng mặc định: a-z A-Z 0-9 space)
 *
 * Trả về: độ dài chuỗi nhập, -1 nếu hủy (giữ BACK)
 */
int lkfx_inputbox(const char *title, char *buf, int buf_len,
                  const char *charset);

#endif /* LKFX_WIDGET_H */
