#include <stdint.h>
#include <stddef.h>
#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>
#include <orbis/UserService.h>
#include <orbis/Pad.h>

#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define SCREEN_DEPTH  4
#define FRAME_BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT * SCREEN_DEPTH)
#define NUM_BUFFERS   2

/* Standard 8x8 font bitmap (ASCII 32 to 126) */
static const uint8_t font8x8_basic[95][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*   */
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, /* ! */
    {0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, /* " */
    {0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00}, /* # */
    {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00}, /* $ */
    {0x00, 0x63, 0x66, 0x0C, 0x18, 0x33, 0x63, 0x00}, /* % */
    {0x1C, 0x36, 0x1C, 0x3B, 0x6E, 0x66, 0x3B, 0x00}, /* & */
    {0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00}, /* ' */
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, /* ( */
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, /* ) */
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, /* * */
    {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, /* + */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30}, /* , */
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, /* - */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, /* . */
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00}, /* / */
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, /* 0 */
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, /* 1 */
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00}, /* 2 */
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, /* 3 */
    {0x0C, 0x1C, 0x3C, 0x6C, 0xFE, 0x0C, 0x0C, 0x00}, /* 4 */
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, /* 5 */
    {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, /* 6 */
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00}, /* 7 */
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, /* 8 */
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00}, /* 9 */
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00}, /* : */
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30}, /* ; */
    {0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00}, /* < */
    {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00}, /* = */
    {0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00}, /* > */
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00}, /* ? */
    {0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00}, /* @ */
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, /* A */
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, /* B */
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, /* C */
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, /* D */
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, /* E */
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, /* F */
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}, /* G */
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, /* H */
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, /* I */
    {0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00}, /* J */
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, /* K */
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, /* L */
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, /* M */
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, /* N */
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, /* O */
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, /* P */
    {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00}, /* Q */
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00}, /* R */
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, /* S */
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, /* T */
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, /* U */
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, /* V */
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, /* W */
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, /* X */
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, /* Y */
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, /* Z */
    {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00}, /* [ */
    {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00}, /* \ */
    {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00}, /* ] */
    {0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00}, /* ^ */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, /* _ */
    {0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}, /* ` */
    {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00}, /* a */
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00}, /* b */
    {0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00}, /* c */
    {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00}, /* d */
    {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, /* e */
    {0x1C, 0x30, 0x78, 0x30, 0x30, 0x30, 0x30, 0x00}, /* f */
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C}, /* g */
    {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, /* h */
    {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00}, /* i */
    {0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38}, /* j */
    {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00}, /* k */
    {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, /* l */
    {0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, /* m */
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00}, /* n */
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, /* o */
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60}, /* p */
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06}, /* q */
    {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00}, /* r */
    {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00}, /* s */
    {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00}, /* t */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3B, 0x00}, /* u */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, /* v */
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x36, 0x36, 0x00}, /* w */
    {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00}, /* x */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x7C}, /* y */
    {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}, /* z */
    {0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00}, /* { */
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, /* | */
    {0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00}, /* } */
    {0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  /* ~ */
};

typedef struct {
    uint8_t r, g, b;
} ColorRGB;

static int video_handle = -1;
static void *video_mem = NULL;
static off_t direct_mem_offset = 0;
static size_t direct_mem_size = 0;
static char *buffers[NUM_BUFFERS] = {NULL, NULL};
static int active_buffer = 0;
static OrbisKernelEqueue flip_queue = 0;

static inline uint32_t make_color(ColorRGB c) {
    return 0x80000000 | ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | (uint32_t)c.b;
}

static int init_graphics(void) {
    int rc;
    OrbisVideoOutBufferAttribute attr;

    video_handle = sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN, ORBIS_VIDEO_OUT_BUS_MAIN, 0, 0);
    if (video_handle < 0) {
        return -1;
    }

    /* Allocate direct memory (aligned to 2MB) */
    size_t needed_size = NUM_BUFFERS * FRAME_BUFFER_SIZE;
    direct_mem_size = (needed_size + 0x200000 - 1) / 0x200000 * 0x200000;

    rc = sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), direct_mem_size, 0x200000, 3, &direct_mem_offset);
    if (rc < 0) {
        return -2;
    }

    rc = sceKernelMapDirectMemory(&video_mem, direct_mem_size, 0x33, 0, direct_mem_offset, 0x200000);
    if (rc < 0) {
        sceKernelReleaseDirectMemory(direct_mem_offset, direct_mem_size);
        return -3;
    }

    buffers[0] = (char *)video_mem;
    buffers[1] = (char *)video_mem + FRAME_BUFFER_SIZE;

    sceVideoOutSetBufferAttribute(&attr, 0x80000000, 1, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);
    rc = sceVideoOutRegisterBuffers(video_handle, 0, (void **)buffers, NUM_BUFFERS, &attr);
    if (rc != 0) {
        return -4;
    }

    rc = sceKernelCreateEqueue(&flip_queue, "homebrew_flip_queue");
    if (rc >= 0) {
        sceVideoOutAddFlipEvent(flip_queue, video_handle, 0);
    }
    sceVideoOutSetFlipRate(video_handle, 0);

    return 0;
}

static void draw_pixel(int x, int y, ColorRGB color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    uint32_t *fb = (uint32_t *)buffers[active_buffer];
    fb[y * SCREEN_WIDTH + x] = make_color(color);
}

static void draw_rect(int x, int y, int w, int h, ColorRGB color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    uint32_t c = make_color(color);
    uint32_t *fb = (uint32_t *)buffers[active_buffer];
    for (int r = 0; r < h; ++r) {
        uint32_t *row = &fb[(y + r) * SCREEN_WIDTH + x];
        for (int col = 0; col < w; ++col) {
            row[col] = c;
        }
    }
}

static void clear_screen(ColorRGB color) {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

static void draw_char(char ch, int x, int y, int scale, ColorRGB color) {
    if (ch < 32 || ch > 126) ch = '?';
    int idx = ch - 32;
    for (int row = 0; row < 8; ++row) {
        uint8_t row_bits = font8x8_basic[idx][row];
        for (int col = 0; col < 8; ++col) {
            if (row_bits & (1 << col)) {
                if (scale == 1) {
                    draw_pixel(x + col, y + row, color);
                } else {
                    draw_rect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

static void draw_string(const char *str, int x, int y, int scale, ColorRGB color) {
    int cur_x = x;
    int cur_y = y;
    for (const char *p = str; *p != '\0'; ++p) {
        if (*p == '\n') {
            cur_x = x;
            cur_y += 10 * scale;
            continue;
        }
        draw_char(*p, cur_x, cur_y, scale, color);
        cur_x += 9 * scale;
    }
}

static void flip_screen(int frame_id) {
    sceVideoOutSubmitFlip(video_handle, active_buffer, 1, frame_id);

    if (flip_queue > 0) {
        for (;;) {
            OrbisVideoOutFlipStatus status;
            sceVideoOutGetFlipStatus(video_handle, &status);
            if (status.flipArg == frame_id) break;

            OrbisKernelEvent evt;
            int count = 0;
            if (sceKernelWaitEqueue(flip_queue, &evt, 1, &count, 0) != 0) break;
        }
    }

    active_buffer = (active_buffer + 1) % NUM_BUFFERS;
}

static void u32_to_str(uint32_t val, char *buf) {
    char tmp[16];
    int i = 0;
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (val > 0) {
        tmp[i++] = '0' + (val % 10);
        val /= 10;
    }
    int j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

int main(void) {
    /* 1. Initialize User Service & Controller */
    int user_id = -1;
    int pad_handle = -1;

    OrbisUserServiceInitializeParams user_params;
    user_params.priority = ORBIS_KERNEL_PRIO_FIFO_LOWEST;
    sceUserServiceInitialize(&user_params);
    sceUserServiceGetInitialUser(&user_id);

    if (scePadInit() == 0 && user_id >= 0) {
        pad_handle = scePadOpen(user_id, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL);
    }

    /* Initialize Direct Framebuffer */
    if (init_graphics() != 0) {
        /* Failsafe loop if video out fails */
        for (;;) {
            sceKernelUsleep(100000);
        }
    }

    ColorRGB c_bg = {15, 23, 42};          /* Dark slate */
    ColorRGB c_card = {30, 41, 59};        /* Card container */
    ColorRGB c_card_border = {51, 65, 85};
    ColorRGB c_white = {248, 250, 252};
    ColorRGB c_cyan = {56, 189, 248};
    ColorRGB c_green = {74, 222, 128};
    ColorRGB c_yellow = {250, 204, 21};
    ColorRGB c_gray = {148, 163, 184};
    ColorRGB c_red = {248, 113, 113};

    int frame_id = 0;
    int anim_pos = 0;
    int anim_dir = 1;

    for (;;) {
        /* Read Controller Data */
        OrbisPadData pad_data;
        int pad_ok = 0;
        if (pad_handle >= 0) {
            if (scePadReadState(pad_handle, &pad_data) >= 0) {
                pad_ok = pad_data.connected != 0;
            }
        }

        /* Animate a pulse element across screen */
        anim_pos += anim_dir * 4;
        if (anim_pos > 1400) anim_dir = -1;
        if (anim_pos < 0) anim_dir = 1;

        /* Clear Screen */
        clear_screen(c_bg);

        /* Top Banner */
        draw_rect(0, 0, SCREEN_WIDTH, 120, c_card);
        draw_rect(0, 118, SCREEN_WIDTH, 4, c_cyan);
        draw_string("PS4 HOMEBREW - HELLO WORLD", 60, 35, 4, c_white);
        draw_string("OpenOrbis Direct Framebuffer (1080p @ 60 FPS)", 1050, 50, 2, c_cyan);

        /* Main Welcome Card */
        draw_rect(60, 160, 1800, 260, c_card);
        draw_rect(60, 160, 1800, 4, c_card_border);
        draw_rect(60, 416, 1800, 4, c_card_border);

        draw_string("SUCCESS! The application is running natively on your PlayStation 4.", 100, 200, 3, c_green);
        draw_string("Hardware VideoOut and SceKernel memory mapping initialized perfectly.", 100, 250, 2, c_white);
        draw_string("No external heavy runtime dependencies - 100% stable execution.", 100, 285, 2, c_gray);

        /* Moving animation bar to confirm smooth 60 FPS */
        draw_rect(100, 340, 1400, 16, c_card_border);
        draw_rect(100 + anim_pos, 336, 120, 24, c_cyan);
        draw_string("60 FPS Smooth Render Test", 100, 370, 2, c_cyan);

        /* Controller Diagnostics Card */
        draw_rect(60, 460, 1800, 560, c_card);
        draw_rect(60, 460, 1800, 4, c_card_border);
        draw_string("CONTROLLER & SYSTEM DIAGNOSTICS", 100, 500, 3, c_yellow);

        /* User & Pad Info */
        char num_buf[16];
        draw_string("Initial User ID:", 100, 560, 2, c_gray);
        u32_to_str((uint32_t)user_id, num_buf);
        draw_string(num_buf, 360, 560, 2, c_white);

        draw_string("Pad Status:", 100, 600, 2, c_gray);
        if (pad_ok) {
            draw_string("CONNECTED (DualShock 4)", 360, 600, 2, c_green);
        } else {
            draw_string("WAITING FOR INPUT / DISCONNECTED", 360, 600, 2, c_red);
        }

        draw_string("Frame Counter:", 100, 640, 2, c_gray);
        u32_to_str((uint32_t)frame_id, num_buf);
        draw_string(num_buf, 360, 640, 2, c_white);

        /* Draw Controller Buttons state */
        draw_string("Button Matrix:", 100, 710, 2, c_cyan);

        int bx = 100, by = 760;
        uint32_t btn = pad_ok ? pad_data.buttons : 0;

        /* Helper for buttons */
        #define DRAW_BTN_STATUS(name, mask, nx, ny) \
            draw_rect(nx, ny, 160, 50, (btn & (mask)) ? c_green : c_card_border); \
            draw_string(name, (nx) + 15, (ny) + 16, 2, (btn & (mask)) ? c_bg : c_white);

        DRAW_BTN_STATUS("CROSS", ORBIS_PAD_BUTTON_CROSS, bx, by);
        DRAW_BTN_STATUS("CIRCLE", ORBIS_PAD_BUTTON_CIRCLE, bx + 180, by);
        DRAW_BTN_STATUS("TRIANGLE", ORBIS_PAD_BUTTON_TRIANGLE, bx + 360, by);
        DRAW_BTN_STATUS("SQUARE", ORBIS_PAD_BUTTON_SQUARE, bx + 540, by);

        DRAW_BTN_STATUS("UP", ORBIS_PAD_BUTTON_UP, bx + 760, by);
        DRAW_BTN_STATUS("DOWN", ORBIS_PAD_BUTTON_DOWN, bx + 940, by);
        DRAW_BTN_STATUS("LEFT", ORBIS_PAD_BUTTON_LEFT, bx + 1120, by);
        DRAW_BTN_STATUS("RIGHT", ORBIS_PAD_BUTTON_RIGHT, bx + 1300, by);

        DRAW_BTN_STATUS("L1", ORBIS_PAD_BUTTON_L1, bx, by + 70);
        DRAW_BTN_STATUS("R1", ORBIS_PAD_BUTTON_R1, bx + 180, by + 70);
        DRAW_BTN_STATUS("L2", ORBIS_PAD_BUTTON_L2, bx + 360, by + 70);
        DRAW_BTN_STATUS("R2", ORBIS_PAD_BUTTON_R2, bx + 540, by + 70);
        DRAW_BTN_STATUS("L3", ORBIS_PAD_BUTTON_L3, bx + 760, by + 70);
        DRAW_BTN_STATUS("R3", ORBIS_PAD_BUTTON_R3, bx + 940, by + 70);
        DRAW_BTN_STATUS("OPTIONS", ORBIS_PAD_BUTTON_OPTIONS, bx + 1120, by + 70);
        DRAW_BTN_STATUS("TOUCHPAD", ORBIS_PAD_BUTTON_TOUCH_PAD, bx + 1300, by + 70);

        #undef DRAW_BTN_STATUS

        /* Analog Stick values */
        draw_string("Left Stick:", 100, 930, 2, c_gray);
        u32_to_str(pad_ok ? pad_data.leftStick.x : 128, num_buf);
        draw_string("X:", 270, 930, 2, c_white);
        draw_string(num_buf, 300, 930, 2, c_cyan);
        u32_to_str(pad_ok ? pad_data.leftStick.y : 128, num_buf);
        draw_string("Y:", 380, 930, 2, c_white);
        draw_string(num_buf, 410, 930, 2, c_cyan);

        draw_string("Right Stick:", 600, 930, 2, c_gray);
        u32_to_str(pad_ok ? pad_data.rightStick.x : 128, num_buf);
        draw_string("X:", 790, 930, 2, c_white);
        draw_string(num_buf, 820, 930, 2, c_cyan);
        u32_to_str(pad_ok ? pad_data.rightStick.y : 128, num_buf);
        draw_string("Y:", 900, 930, 2, c_white);
        draw_string(num_buf, 930, 930, 2, c_cyan);

        /* Flip backbuffer to display */
        flip_screen(frame_id);
        frame_id++;
    }

    return 0;
}