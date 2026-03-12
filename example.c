/*
 * example.c — Ví dụ đầy đủ dùng thư viện lkfx
 *
 * Build:
 *   make example
 *   scp example root@192.168.137.2:/root/
 *
 * Chương trình demo:
 *   1. Vẽ các hình học cơ bản
 *   2. Demo text nhiều kích cỡ
 *   3. Mở menu chính → điều hướng đến các màn hình demo
 */

#include "lkfx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ================================================================
 * Màn hình 1: Demo vẽ hình học
 * ================================================================ */
static void screen_shapes(void) {
    lkfx_fb_fill(LKFX_BLACK);

    /* Tiêu đề */
    lkfx_fill_rect(0, 0, LKFX_W, 16, LKFX_BLUE);
    lkfx_text_center(LKFX_W/2, 4, "Shapes Demo",
                     &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLUE);

    /* Hình chữ nhật đặc */
    lkfx_fill_rect(10, 25, 50, 30, LKFX_RED);
    lkfx_text(10, 58, "fill_rect", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Hình chữ nhật viền */
    lkfx_draw_rect(75, 25, 50, 30, LKFX_GREEN);
    lkfx_text(75, 58, "draw_rect", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Bo góc */
    lkfx_fill_round_rect(140, 25, 50, 30, 8, LKFX_CYAN);
    lkfx_text(137, 58, "round_rect", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Đường thẳng */
    lkfx_draw_line(10, 80, 80, 110, LKFX_YELLOW);
    lkfx_draw_line(80, 80, 10, 110, LKFX_YELLOW);
    lkfx_text(10, 113, "lines", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Hình tròn */
    lkfx_fill_circle(120, 95, 20, LKFX_MAGENTA);
    lkfx_draw_circle(120, 95, 22, LKFX_WHITE);
    lkfx_text(100, 120, "circle", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Tam giác */
    lkfx_fill_triangle(190, 80, 170, 115, 210, 115, LKFX_RGB(255,128,0));
    lkfx_text(165, 120, "triangle", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Blend màu */
    for (int i = 0; i < 100; i++) {
        uint16_t c = lkfx_color_blend(LKFX_RED, LKFX_BLUE, i * 256 / 100);
        lkfx_draw_vline(10 + i*2, 135, 15, c);
    }
    lkfx_text(10, 152, "color blend", &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);

    /* Hint */
    lkfx_fill_rect(0, LKFX_H-12, LKFX_W, 12, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-10, "BACK:return",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);

    lkfx_fb_flush();
    /* Chờ BACK */
    while (lkfx_input_wait_btn() != LKFX_BTN_BACK);
}

/* ================================================================
 * Màn hình 2: Demo text / font
 * ================================================================ */
static void screen_fonts(void) {
    lkfx_fb_fill(LKFX_BLACK);

    lkfx_fill_rect(0, 0, LKFX_W, 16, LKFX_BLUE);
    lkfx_text_center(LKFX_W/2, 4, "Font Demo",
                     &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLUE);

    int y = 22;

    /* Scale 1 */
    lkfx_text(4, y, "Scale 1: Hello World! 0123",
              &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);
    y += 12;

    /* Scale 2 */
    lkfx_text(4, y, "Scale 2: Hello!",
              &lkfx_font_6x8, 2, LKFX_GREEN, LKFX_BLACK);
    y += 20;

    /* Scale 3 */
    lkfx_text(4, y, "Scale 3: Hi",
              &lkfx_font_6x8, 3, LKFX_YELLOW, LKFX_BLACK);
    y += 30;

    /* Scale 4 */
    lkfx_text(4, y, "Sc4: AB",
              &lkfx_font_6x8, 4, LKFX_CYAN, LKFX_BLACK);
    y += 38;

    /* Căn giữa */
    lkfx_text_center(LKFX_W/2, y, "-- Centered --",
                     &lkfx_font_6x8, 1, LKFX_MAGENTA, LKFX_BLACK);
    y += 12;

    /* Căn phải */
    lkfx_text_right(LKFX_W-4, y, "Right aligned!",
                    &lkfx_font_6x8, 1, LKFX_RGB(255,128,0), LKFX_BLACK);
    y += 14;

    /* Transparent bg */
    lkfx_fill_rect(4, y, 160, 12, LKFX_RGB(30,30,80));
    lkfx_text(4, y+2, "Transparent bg text",
              &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_TRANSPARENT);
    y += 16;

    /* Text wrap */
    lkfx_fill_rect(4, y, LKFX_W-8, 30, 0x2104);
    lkfx_text_wrap(6, y+2, LKFX_W-12, 28,
                   "Auto wrap: This is a long sentence that will wrap.",
                   &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);

    /* Hint */
    lkfx_fill_rect(0, LKFX_H-12, LKFX_W, 12, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-10, "BACK:return",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);

    lkfx_fb_flush();
    while (lkfx_input_wait_btn() != LKFX_BTN_BACK);
}

/* ================================================================
 * Màn hình 3: Demo widget
 * ================================================================ */
static void screen_widgets(void) {
    /* Sub-menu */
    const char *items[] = {
        "ProgressBar demo",
        "MessageBox (OK)",
        "MessageBox (YesNo)",
        "Toast notification",
        "InputBox",
        NULL
    };

    while (1) {
        int sel = lkfx_menu("Widget Demo", items, 0);
        if (sel < 0) break;

        if (sel == 0) {
            /* Progress bar */
            lkfx_fb_fill(LKFX_BLACK);
            lkfx_fill_rect(0, 0, LKFX_W, 16, LKFX_BLUE);
            lkfx_text_center(LKFX_W/2, 4, "ProgressBar",
                             &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLUE);

            lkfx_progressbar_t pb;
            lkfx_progressbar_init(&pb, 20, 100, 200, 10,
                                  LKFX_GREEN, LKFX_DARKGRAY);
            pb.show_text = 1;
            pb.border    = LKFX_GRAY;

            for (int v = 0; v <= 100; v += 2) {
                lkfx_progressbar_set(&pb, v);
                char buf[16]; snprintf(buf, sizeof(buf), "Loading... %d%%", v);
                lkfx_fill_rect(0, 120, LKFX_W, 10, LKFX_BLACK);
                lkfx_text_center(LKFX_W/2, 120, buf,
                                 &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);
                lkfx_fb_flush_rect(0, 95, LKFX_W, 40);
                struct timespec ts = {0, 40000000};
                nanosleep(&ts, NULL);
            }
            lkfx_toast("Done!", 1500);

        } else if (sel == 1) {
            lkfx_msgbox("Information", "This is a simple\nmessage box.", LKFX_MB_INFO);

        } else if (sel == 2) {
            int r = lkfx_msgbox("Confirm", "Are you sure?", LKFX_MB_YESNO);
            lkfx_toast(r ? "You chose YES" : "You chose NO", 1500);

        } else if (sel == 3) {
            lkfx_toast("This is a toast message!", 2000);

        } else if (sel == 4) {
            char name[32] = "";
            int r = lkfx_inputbox("Enter name:", name, sizeof(name), NULL);
            if (r > 0) {
                char msg[48];
                snprintf(msg, sizeof(msg), "Hello, %s!", name);
                lkfx_msgbox("Result", msg, LKFX_MB_OK);
            } else {
                lkfx_toast("Cancelled", 1000);
            }
        }
    }
}

/* ================================================================
 * Màn hình 4: Thông tin hệ thống
 * ================================================================ */
static void screen_sysinfo(void) {
    lkfx_fb_fill(LKFX_BLACK);
    lkfx_fill_rect(0, 0, LKFX_W, 16, LKFX_BLUE);
    lkfx_text_center(LKFX_W/2, 4, "System Info",
                     &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLUE);

    /* Đọc thông tin hệ thống */
    FILE *f;
    char buf[128];
    int y = 24;

    /* Uptime */
    f = fopen("/proc/uptime", "r");
    if (f) {
        float up; fscanf(f, "%f", &up); fclose(f);
        int h = (int)(up/3600), m = (int)(up/60)%60, s = (int)up%60;
        snprintf(buf, sizeof(buf), "Uptime: %02d:%02d:%02d", h, m, s);
        lkfx_text(4, y, buf, &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);
        y += 12;
    }

    /* Memory */
    f = fopen("/proc/meminfo", "r");
    if (f) {
        long total = 0, free_mem = 0;
        char key[64]; long val;
        while (fscanf(f, "%s %ld kB", key, &val) == 2) {
            if (strcmp(key, "MemTotal:") == 0) total = val;
            if (strcmp(key, "MemFree:")  == 0) { free_mem = val; break; }
        }
        fclose(f);
        snprintf(buf, sizeof(buf), "RAM: %ldKB / %ldKB free",
                 free_mem, total);
        lkfx_text(4, y, buf, &lkfx_font_6x8, 1, LKFX_CYAN, LKFX_BLACK);
        y += 12;

        /* Progress bar RAM usage */
        lkfx_progressbar_t pb;
        int used = total > 0 ? (int)((total - free_mem) * 100 / total) : 0;
        lkfx_progressbar_init(&pb, 4, y, LKFX_W-8, 6,
                              lkfx_color_blend(LKFX_GREEN, LKFX_RED, used*256/100),
                              LKFX_DARKGRAY);
        lkfx_progressbar_set(&pb, used);
        y += 14;
    }

    /* CPU temp (nếu có) */
    f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (f) {
        int temp; fscanf(f, "%d", &temp); fclose(f);
        snprintf(buf, sizeof(buf), "CPU temp: %d.%d C", temp/1000, (temp%1000)/100);
        lkfx_text(4, y, buf, &lkfx_font_6x8, 1, LKFX_YELLOW, LKFX_BLACK);
        y += 12;
    }

    /* Kernel version */
    f = fopen("/proc/version", "r");
    if (f) {
        char ver[80] = "";
        fgets(ver, sizeof(ver), f); fclose(f);
        ver[40] = '\0';  /* cắt ngắn */
        lkfx_text_wrap(4, y, LKFX_W-8, 24, ver,
                       &lkfx_font_6x8, 1, LKFX_GRAY, LKFX_BLACK);
        y += 26;
    }

    lkfx_fill_rect(0, LKFX_H-12, LKFX_W, 12, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-10, "BACK:return",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);
    lkfx_fb_flush();
    while (lkfx_input_wait_btn() != LKFX_BTN_BACK);
}

/* ================================================================
 * Màn hình test font — hiện toàn bộ ký tự ASCII 32-127
 * Dùng để debug: nếu thấy chữ đúng hình dạng là font OK
 * ================================================================ */
static void screen_font_test(void) {
    /* Trang 1: chữ thường + hoa, scale 1 */
    lkfx_fb_fill(LKFX_BLACK);
    lkfx_fill_rect(0, 0, LKFX_W, 14, 0x001F);
    lkfx_text_center(LKFX_W/2, 3, "Font test scale=1",
                     &lkfx_font_6x8, 1, LKFX_WHITE, 0x001F);

    /* In toàn bộ ASCII 32-127, 40 ký tự/hàng */
    int x = 0, y = 16;
    for (int c = 32; c < 128; c++) {
        char str[2] = { (char)c, 0 };
        lkfx_text(x, y, str, &lkfx_font_6x8, 1, LKFX_WHITE, LKFX_BLACK);
        x += 6;
        if (x + 6 > LKFX_W) { x = 0; y += 9; }
    }

    lkfx_fill_rect(0, LKFX_H-10, LKFX_W, 10, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-9, "OK:next  BACK:exit",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);
    lkfx_fb_flush();

    int btn = lkfx_input_wait_btn();
    if (btn == LKFX_BTN_BACK) return;

    /* Trang 2: scale 2 */
    lkfx_fb_fill(LKFX_BLACK);
    lkfx_fill_rect(0, 0, LKFX_W, 14, 0x001F);
    lkfx_text_center(LKFX_W/2, 3, "scale=2",
                     &lkfx_font_6x8, 1, LKFX_WHITE, 0x001F);

    x = 0; y = 16;
    for (int c = 32; c < 128; c++) {
        char str[2] = { (char)c, 0 };
        lkfx_text(x, y, str, &lkfx_font_6x8, 2, LKFX_CYAN, LKFX_BLACK);
        x += 12;
        if (x + 12 > LKFX_W) { x = 0; y += 17; }
        if (y + 16 > LKFX_H - 10) break;
    }

    lkfx_fill_rect(0, LKFX_H-10, LKFX_W, 10, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-9, "OK:next  BACK:exit",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);
    lkfx_fb_flush();

    btn = lkfx_input_wait_btn();
    if (btn == LKFX_BTN_BACK) return;

    /* Trang 3: scale 3, chỉ A-Z */
    lkfx_fb_fill(LKFX_BLACK);
    lkfx_fill_rect(0, 0, LKFX_W, 14, 0x001F);
    lkfx_text_center(LKFX_W/2, 3, "scale=3  A-Z",
                     &lkfx_font_6x8, 1, LKFX_WHITE, 0x001F);

    x = 0; y = 16;
    for (int c = 'A'; c <= 'Z'; c++) {
        char str[2] = { (char)c, 0 };
        lkfx_text(x, y, str, &lkfx_font_6x8, 3, LKFX_YELLOW, LKFX_BLACK);
        x += 18;
        if (x + 18 > LKFX_W) { x = 0; y += 25; }
        if (y + 24 > LKFX_H - 10) break;
    }

    lkfx_fill_rect(0, LKFX_H-10, LKFX_W, 10, 0x2104);
    lkfx_text_center(LKFX_W/2, LKFX_H-9, "BACK:exit",
                     &lkfx_font_6x8, 1, LKFX_GRAY, 0x2104);
    lkfx_fb_flush();
    while (lkfx_input_wait_btn() != LKFX_BTN_BACK);
}


int main(void) {
    /* Khởi tạo display và input */
    lkfx_display_init();
    lkfx_input_init_default();
    lkfx_theme_reset();  /* đảm bảo font pointer được set đúng lúc runtime */

    /* Menu chính */
    const char *main_items[] = {
        "Font test (debug)",
        "Shapes demo",
        "Font / Text demo",
        "Widget demo",
        "System info",
        "Exit",
        NULL
    };
    const char *main_icons[] = {
        "[F]", "[*]", "[T]", "[W]", "[I]", "[X]", NULL
    };

    while (1) {
        int sel = lkfx_menu_icons("LKFX Demo", main_items, main_icons, 0);

        if (sel == 0)      screen_font_test();
        else if (sel == 1) screen_shapes();
        else if (sel == 2) screen_fonts();
        else if (sel == 3) screen_widgets();
        else if (sel == 4) screen_sysinfo();
        else if (sel == 5 || sel < 0) {
            lkfx_msgbox("Exit", "Goodbye!", LKFX_MB_OK);
            break;
        }
    }

    lkfx_fb_fill(LKFX_BLACK);
    lkfx_fb_flush();
    lkfx_input_deinit();
    lkfx_display_deinit();
    return 0;
}
