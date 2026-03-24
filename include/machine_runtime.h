#ifndef MACHINE_RUNTIME_H
#define MACHINE_RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

typedef struct MachineListNode
{
    long long value;
    struct MachineListNode *next;
} MachineListNode;
typedef struct MachineList
{
    MachineListNode *head;
    MachineListNode *tail;
    long long size;
} MachineList;
typedef struct MachineArray
{
    long long *data;
    long long size;
    long long capacity;
} MachineArray;
typedef struct MachineGrid
{
    long long rows;
    long long cols;
    long long *data;
} MachineGrid;

enum
{
    MACHINE_EVENT_NONE = 0,
    MACHINE_EVENT_KEY = 1,
    MACHINE_EVENT_MOUSE = 2,
    MACHINE_EVENT_QUIT = 3
};
enum
{
    MACHINE_KEY_LEFT = 1000,
    MACHINE_KEY_RIGHT = 1001,
    MACHINE_KEY_UP = 1002,
    MACHINE_KEY_DOWN = 1003,
    MACHINE_KEY_ESC = 1004,
    MACHINE_KEY_ENTER = 1005,
    MACHINE_KEY_BACKSPACE = 1006,
    MACHINE_KEY_SPACE = 1007
};

char *machine_strdup(const char *s);
char *machine_concat(const char *a, const char *b);
long long machine_len(const char *s);
char *machine_index_str(const char *s, long long i);
long double machine_hp_from_text(const char *s);
long double machine_hp_add(long double a, long double b);
long double machine_hp_sub(long double a, long double b);
long double machine_hp_mul(long double a, long double b);
long double machine_hp_div(long double a, long double b);
long double machine_hp_sqrt(long double a);
long double machine_hp_pow(long double a, long double b);
void *machine_alloc_bytes(long long n);
void machine_free_mem(void *p);
void machine_store_i64(void *p, long long v);
long long machine_load_i64(void *p);
void machine_store_f64(void *p, double v);
double machine_load_f64(void *p);
void machine_store_str(void *p, char *v);
char *machine_load_str(void *p);
MachineList *machine_list_new(void);
void machine_list_push_back(MachineList *list, long long value);
long long machine_list_get(MachineList *list, long long index);
long long machine_list_size(MachineList *list);
void machine_list_free(MachineList *list);
MachineArray *machine_array_new(void);
void machine_array_push(MachineArray *a, long long value);
long long machine_array_get(MachineArray *a, long long index);
void machine_array_set(MachineArray *a, long long index, long long value);
long long machine_array_len(MachineArray *a);
void machine_array_free(MachineArray *a);
MachineGrid *machine_grid_new(long long rows, long long cols);
long long machine_grid_get(MachineGrid *g, long long row, long long col);
void machine_grid_set(MachineGrid *g, long long row, long long col, long long value);
long long machine_grid_rows(MachineGrid *g);
long long machine_grid_cols(MachineGrid *g);
void machine_grid_fill(MachineGrid *g, long long value);
void machine_grid_free(MachineGrid *g);
long long machine_tick_ms(void);
void machine_timer_reset(void);
long long machine_timer_elapsed_ms(void);
void machine_sleep_ms(long long ms);
void machine_term_enable_raw(void);
void machine_term_disable_raw(void);
int machine_term_key_available(void);
long long machine_term_read_key(void);
void machine_term_enable_mouse(void);
void machine_term_disable_mouse(void);
long long machine_term_last_key(void);
long long machine_term_mouse_x(void);
long long machine_term_mouse_y(void);
long long machine_term_mouse_button(void);
long long machine_term_poll_event(void);
void machine_term_clear(void);
void machine_term_flush(void);
void machine_term_move_cursor(long long x, long long y);
void machine_term_hide_cursor(void);
void machine_term_show_cursor(void);
void machine_term_draw_text(long long x, long long y, const char *text);
int machine_win_create(const char *title, long long width, long long height);
void machine_win_destroy(void);
int machine_win_is_open(void);
long long machine_win_last_key(void);
long long machine_win_mouse_x(void);
long long machine_win_mouse_y(void);
long long machine_win_mouse_button(void);
long long machine_win_poll_event(void);
void machine_win_set_title(const char *title);
void machine_win_clear(long long r, long long g, long long b);
void machine_win_present(void);
void machine_win_draw_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b);
void machine_win_fill_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b);
void machine_win_draw_line(long long x1, long long y1, long long x2, long long y2, long long r, long long g, long long b);
void machine_win_draw_pixel(long long x, long long y, long long r, long long g, long long b);
void machine_win_draw_text(long long x, long long y, const char *text);
void *machine_image_load(const char *path);
long long machine_image_width(void *ptr);
long long machine_image_height(void *ptr);
void machine_image_draw(void *ptr, long long x, long long y);
void machine_image_draw_scaled(void *ptr, long long x, long long y, long long w, long long h);
void machine_image_free(void *ptr);
long long machine_video_play(const char *path);
int machine_video_is_running(long long pid_value);
void machine_video_stop(long long pid_value);

#endif
