/*
 * Annotated reading edition of machine_runtime_freestanding.h
 *
 * This file keeps the original code intact and only adds explanatory comments.
 * The goal of this edition is to explain the role of the header, the meaning of its
 * declarations, and how it fits into the Machine compiler / runtime architecture.
 */

/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#ifndef MACHINE_RUNTIME_FREESTANDING_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_RUNTIME_FREESTANDING_H

/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_RUNTIME_API_VERSION 1011

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stddef.h>
/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stdint.h>

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct MachineListNode
{
    long long value;
    /*
     * Structure declaration.
     *
     * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
     */
    struct MachineListNode *next;
} MachineListNode;
/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct MachineList
{
    MachineListNode *head;
    MachineListNode *tail;
    long long size;
} MachineList;
/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct MachineArray
{
    long long *data;
    long long size;
    long long capacity;
} MachineArray;
/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct MachineGrid
{
    long long rows;
    long long cols;
    long long *data;
} MachineGrid;

/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
enum
{
    MACHINE_EVENT_NONE = 0,
    MACHINE_EVENT_KEY = 1,
    MACHINE_EVENT_MOUSE = 2,
    MACHINE_EVENT_QUIT = 3
};

/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
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

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_print_i64(long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_print_str(const char *s);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_print_f64(double value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_print_hp(long double value);

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_strdup(const char *s);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_concat(const char *a, const char *b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_len(const char *s);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_index_str(const char *s, long long i);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_from_text(const char *s);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_add(long double a, long double b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_sub(long double a, long double b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_mul(long double a, long double b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_div(long double a, long double b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_sqrt(long double a);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long double machine_hp_pow(long double a, long double b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_alloc_bytes(long long n);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_free_mem(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_i64(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_load_i64(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_f64(void *p, double v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
double machine_load_f64(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_str(void *p, char *v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_load_str(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_ptr_from_i64(long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_ptr_to_i64(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_ptr_offset(void *p, long long offset);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_ptr_hex(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
char *machine_ptr_bin(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_u8(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_u16(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_u32(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_store_u64(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_load_u8(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_load_u16(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_load_u32(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_load_u64(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_volatile_store_u8(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_volatile_store_u16(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_volatile_store_u32(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_volatile_store_u64(void *p, long long v);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_volatile_load_u8(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_volatile_load_u16(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_volatile_load_u32(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_volatile_load_u64(void *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall0(long long n);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall1(long long n, long long a1);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall2(long long n, long long a1, long long a2);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall3(long long n, long long a1, long long a2, long long a3);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall4(long long n, long long a1, long long a2, long long a3, long long a4);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall5(long long n, long long a1, long long a2, long long a3, long long a4, long long a5);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_syscall6(long long n, long long a1, long long a2, long long a3, long long a4, long long a5, long long a6);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_mmap_anon(long long size);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_mmap_anon_exec(long long size);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_munmap_mem(void *p, long long size);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_open_ro(const char *path);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_open_wo(const char *path);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_open_rw(const char *path);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_close(long long fd);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_read(long long fd, void *buf, long long size);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_write(long long fd, void *buf, long long size);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_fd_seek(long long fd, long long offset, long long whence);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_ioctl_i64(long long fd, long long request, long long arg);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_asm_nop(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_asm_pause(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_asm_mfence(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_asm_lfence(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_asm_sfence(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_asm_rdtsc(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_cpu_in8(long long port);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_cpu_out8(long long port, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
MachineList *machine_list_new(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_list_push_back(MachineList *list, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_list_get(MachineList *list, long long index);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_list_size(MachineList *list);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_list_free(MachineList *list);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
MachineArray *machine_array_new(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_array_push(MachineArray *a, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_array_get(MachineArray *a, long long index);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_array_set(MachineArray *a, long long index, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_array_len(MachineArray *a);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_array_free(MachineArray *a);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
MachineGrid *machine_grid_new(long long rows, long long cols);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_grid_get(MachineGrid *g, long long row, long long col);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_grid_set(MachineGrid *g, long long row, long long col, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_grid_rows(MachineGrid *g);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_grid_cols(MachineGrid *g);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_grid_fill(MachineGrid *g, long long value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_grid_free(MachineGrid *g);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_tick_ms(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_timer_reset(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_timer_elapsed_ms(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_sleep_ms(long long ms);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_enable_raw(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_disable_raw(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_term_key_available(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_read_key(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_enable_mouse(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_disable_mouse(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_last_key(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_mouse_x(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_mouse_y(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_mouse_button(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_term_poll_event(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_clear(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_flush(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_move_cursor(long long x, long long y);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_hide_cursor(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_show_cursor(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_term_draw_text(long long x, long long y, const char *text);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_win_create(const char *title, long long width, long long height);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_destroy(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_win_is_open(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_win_last_key(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_win_mouse_x(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_win_mouse_y(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_win_mouse_button(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_win_poll_event(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_set_title(const char *title);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_clear(long long r, long long g, long long b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_present(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_draw_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_fill_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_draw_line(long long x1, long long y1, long long x2, long long y2, long long r, long long g, long long b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_draw_pixel(long long x, long long y, long long r, long long g, long long b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_win_draw_text(long long x, long long y, const char *text);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_image_load(const char *path);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_image_width(void *ptr);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_image_height(void *ptr);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_image_draw(void *ptr, long long x, long long y);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_image_draw_scaled(void *ptr, long long x, long long y, long long w, long long h);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_image_free(void *ptr);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_video_play(const char *path);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_video_is_running(long long pid_value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_video_stop(long long pid_value);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_pmm_alloc_page(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void *machine_pmm_alloc_pages(long long count);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_pmm_total_bytes(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_pmm_used_bytes(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
long long machine_page_identity_map_2m(long long page_index, long long phys_base, long long flags);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_apic_supported(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int machine_apic_enable(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void machine_apic_eoi(void);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
