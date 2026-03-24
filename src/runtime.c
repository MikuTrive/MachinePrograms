#include "machine_runtime.h"

static struct termios machine_saved_termios;
int machine_raw_enabled = 0;
long long machine_last_key = 0;
static long long machine_mouse_x = 0;
/* we define the global variables to track the state of the terminal input, 
   including whether raw mode is enabled, the last key pressed, 
   and the current mouse position and button state.
   these variables are used in the implementation of terminal input handling functions to 
   provide a consistent interface for reading keyboard and mouse events. */
static long long machine_mouse_y = 0;
static long long machine_mouse_button = 0;
static struct timespec machine_timer_origin = {0, 0};

void machine_panic(const char *msg)
{
    fprintf(stderr, "machine runtime error: %s\n", msg);
    exit(1);
}
/* we implement a set of utility functions for string manipulation, 
   dynamic memory allocation, list and array management, grid handling,
   timing, terminal input/output, and window management.
   these functions provide the core functionality needed for the 
   runtime environment of our programming language,
   allowing us to perform common operations such as string duplication, 
   concatenation, array resizing, grid indexing, 
   timing measurements, and terminal interactions. */
char *machine_strdup(const char *s)
{
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p)
        machine_panic("out of memory");
    memcpy(p, s, n + 1);
    return p;
}
char *machine_concat(const char *a, const char *b)
{
    size_t la = strlen(a), lb = strlen(b);
    char *p = (char *)malloc(la + lb + 1);
    if (!p)
        machine_panic("out of memory");
    memcpy(p, a, la);
    memcpy(p + la, b, lb + 1);
    return p;
}
long long machine_len(const char *s) { return (long long)strlen(s); }
char *machine_index_str(const char *s, long long i)
{
    static char buf[16][2];
    static int slot = 0;
    size_t n = strlen(s);
    if (i < 0 || (size_t)i >= n)
        machine_panic("string index out of range");
    slot = (slot + 1) % 16;
    buf[slot][0] = s[i];
    buf[slot][1] = '\0';
    return buf[slot];
}
long double machine_hp_from_text(const char *s) { return strtold(s, NULL); }
long double machine_hp_add(long double a, long double b) { return a + b; }
long double machine_hp_sub(long double a, long double b) { return a - b; }
long double machine_hp_mul(long double a, long double b) { return a * b; }
long double machine_hp_div(long double a, long double b) { return a / b; }
long double machine_hp_sqrt(long double a) { return sqrtl(a); }
long double machine_hp_pow(long double a, long double b) { return powl(a, b); }
/* we also implement a set of functions for high-precision floating-point arithmetic, 
   which allow us to perform operations such as addition, subtraction, multiplication, 
   division, square root, and power on long double values.
   these functions can be used in our programming language to 
   support high-precision calculations when needed. */
void *machine_alloc_bytes(long long n)
{
    if (n <= 0)
        machine_panic("alloc_bytes expects positive size");
    void *p = calloc(1, (size_t)n);
    if (!p)
        machine_panic("allocation failed");
    return p;
}
void machine_free_mem(void *p) { free(p); }
void machine_store_i64(void *p, long long v) { *((long long *)p) = v; }
long long machine_load_i64(void *p) { return *((long long *)p); }
void machine_store_f64(void *p, double v) { *((double *)p) = v; }
double machine_load_f64(void *p) { return *((double *)p); }
void machine_store_str(void *p, char *v) { *((char **)p) = v; }
char *machine_load_str(void *p) { return *((char **)p); }
/* we provide a set of functions for dynamic memory management, including allocating and freeing memory, 
   as well as storing and loading 64-bit integers, 
   double-precision floating-point numbers, and strings.
   these functions abstract away the details of memory management and 
   provide a simple interface for our programming language to interact with memory. */
MachineList *machine_list_new(void)
{
    MachineList *list = (MachineList *)calloc(1, sizeof(MachineList));
    if (!list)
        machine_panic("list_new failed");
    return list;
}
void machine_list_push_back(MachineList *list, long long value)
{
    MachineListNode *node = (MachineListNode *)calloc(1, sizeof(MachineListNode));
    if (!node)
        machine_panic("list push failed");
    node->value = value;
    if (!list->head)
        list->head = list->tail = node;
    else
    {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}
long long machine_list_get(MachineList *list, long long index)
{
    if (!list || index < 0 || index >= list->size)
        machine_panic("list index out of range");
    MachineListNode *cur = list->head;
    for (long long i = 0; i < index; ++i)
        cur = cur->next;
    return cur->value;
}
long long machine_list_size(MachineList *list) { return list ? list->size : 0; }
/* we implement a simple linked list data structure with functions to create a new list, 
   push values to the back of the list, get values by index, and retrieve the size of the list.
   this allows us to use linked lists in our programming language for dynamic collections of values. */
void machine_list_free(MachineList *list)
{
    if (!list)
        return;
    MachineListNode *cur = list->head;
    while (cur)
    {
        MachineListNode *next = cur->next;
        free(cur);
        cur = next;
    }
    free(list);
}
MachineArray *machine_array_new(void)
{
    MachineArray *a = (MachineArray *)calloc(1, sizeof(MachineArray));
    if (!a)
        machine_panic("array_new failed");
    return a;
}
void machine_array_reserve(MachineArray *a, long long need)
{
    if (!a)
        machine_panic("array is null");
    if (a->capacity >= need)
        return;
    long long cap = a->capacity ? a->capacity : 4;
    while (cap < need)
        cap *= 2;
    long long *p = (long long *)realloc(a->data, (size_t)cap * sizeof(long long));
    if (!p)
        machine_panic("array realloc failed");
    a->data = p;
    a->capacity = cap;
}
void machine_array_push(MachineArray *a, long long value)
{
    machine_array_reserve(a, a->size + 1);
    a->data[a->size++] = value;
}
long long machine_array_get(MachineArray *a, long long index)
{
    if (!a || index < 0 || index >= a->size)
        machine_panic("array index out of range");
    return a->data[index];
}
void machine_array_set(MachineArray *a, long long index, long long value)
{
    if (!a || index < 0 || index >= a->size)
        machine_panic("array index out of range");
    a->data[index] = value;
}
long long machine_array_len(MachineArray *a) { return a ? a->size : 0; }
/* we implement a dynamic array data structure with functions to create a new array, 
   reserve capacity, push values to the end of the array, get and set values by index, 
   and retrieve the length of the array.
   this allows us to use dynamic arrays in our programming language for 
   efficient storage and access of sequential data. */
void machine_array_free(MachineArray *a)
{
    if (!a)
        return;
    free(a->data);
    free(a);
}
MachineGrid *machine_grid_new(long long rows, long long cols)
{
    if (rows <= 0 || cols <= 0)
        machine_panic("grid_new expects positive rows and cols");
    MachineGrid *g = (MachineGrid *)calloc(1, sizeof(MachineGrid));
    if (!g)
        machine_panic("grid_new failed");
    g->rows = rows;
    g->cols = cols;
    g->data = (long long *)calloc((size_t)(rows * cols), sizeof(long long));
    if (!g->data)
        machine_panic("grid allocation failed");
    return g;
}
long long machine_grid_index(MachineGrid *g, long long row, long long col)
{
    if (!g || row < 0 || col < 0 || row >= g->rows || col >= g->cols)
        machine_panic("grid index out of range");
    return row * g->cols + col;
}
long long machine_grid_get(MachineGrid *g, long long row, long long col) { return g->data[machine_grid_index(g, row, col)]; }
void machine_grid_set(MachineGrid *g, long long row, long long col, long long value) { g->data[machine_grid_index(g, row, col)] = value; }
long long machine_grid_rows(MachineGrid *g) { return g ? g->rows : 0; }
long long machine_grid_cols(MachineGrid *g) { return g ? g->cols : 0; }
/* we implement a grid data structure, which is essentially a 2D array, 
   with functions to create a new grid, calculate the index for a given row and column, 
   get and set values at specific positions, and retrieve the number of rows and columns.
   this allows us to use grids in our programming language for applications that 
   require two-dimensional data storage and access. */
void machine_grid_fill(MachineGrid *g, long long value)
{
    if (!g)
        return;
    for (long long i = 0; i < g->rows * g->cols; ++i)
        g->data[i] = value;
}
void machine_grid_free(MachineGrid *g)
{
    if (!g)
        return;
    free(g->data);
    free(g);
}

long long machine_now_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    return (long long)ts.tv_sec * 1000LL + (long long)ts.tv_nsec / 1000000LL;
}
long long machine_tick_ms(void) { return machine_now_ms(); }
/* we implement functions to get the current time in milliseconds and 
   to measure elapsed time since a reference point.
   these functions can be used in our programming language for timing operations, 
   measuring performance, or implementing time-based features. */
void machine_timer_reset(void) { clock_gettime(CLOCK_MONOTONIC, &machine_timer_origin); }
long long machine_timer_elapsed_ms(void)
{
    struct timespec now;
    if (machine_timer_origin.tv_sec == 0 && machine_timer_origin.tv_nsec == 0)
        machine_timer_reset();
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return 0;
    return (long long)(now.tv_sec - machine_timer_origin.tv_sec) * 1000LL + (long long)(now.tv_nsec - machine_timer_origin.tv_nsec) / 1000000LL;
}
void machine_sleep_ms(long long ms)
{
    if (ms <= 0)
        return;
    struct timespec req;
    req.tv_sec = (time_t)(ms / 1000LL);
    req.tv_nsec = (long)((ms % 1000LL) * 1000000LL);
    nanosleep(&req, NULL);
}

void machine_term_restore(void)
{
    if (machine_raw_enabled)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &machine_saved_termios);
        machine_raw_enabled = 0;
    }
}
void machine_term_enable_raw(void)
{
    if (machine_raw_enabled)
        return;
    if (!isatty(STDIN_FILENO))
        return;
    if (tcgetattr(STDIN_FILENO, &machine_saved_termios) != 0)
        return;
    struct termios raw = machine_saved_termios;
    raw.c_iflag &= (tcflag_t) ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= (tcflag_t) ~(OPOST);
    raw.c_cflag |= (tcflag_t)(CS8);
    raw.c_lflag &= (tcflag_t) ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)
    {
        machine_raw_enabled = 1;
        atexit(machine_term_restore);
    }
}
void machine_term_disable_raw(void) { machine_term_restore(); }
/* we implement functions to enable and disable raw mode for terminal input, 
   which allows us to read keyboard input without waiting for a 
   newline and to handle special keys and mouse events.
   these functions use the termios API to configure the terminal settings accordingly. */
int machine_term_wait_input(long timeout_ms)
{
    fd_set set;
    struct timeval tv;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    if (timeout_ms < 0)
        return select(STDIN_FILENO + 1, &set, NULL, NULL, NULL);
    tv.tv_sec = timeout_ms / 1000L;
    tv.tv_usec = (timeout_ms % 1000L) * 1000L;
    return select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);
}
int machine_term_read_byte_with_timeout(long timeout_ms)
{
    unsigned char ch = 0;
    int ready = machine_term_wait_input(timeout_ms);
    if (ready <= 0)
        return -1;
    if (read(STDIN_FILENO, &ch, 1) != 1)
        return -1;
    return (int)ch;
}
int machine_term_key_available(void) { return machine_term_wait_input(0) > 0; }
long long machine_term_read_key(void)
{
    int ch = machine_term_read_byte_with_timeout(-1);
    if (ch < 0)
        return -1;
    if (ch == 27)
    {
        int b1 = machine_term_read_byte_with_timeout(5);
        if (b1 == '[')
        {
            int b2 = machine_term_read_byte_with_timeout(5);
            if (b2 == 'A')
                return MACHINE_KEY_UP;
            if (b2 == 'B')
                return MACHINE_KEY_DOWN;
            if (b2 == 'C')
                return MACHINE_KEY_RIGHT;
            if (b2 == 'D')
                return MACHINE_KEY_LEFT;
        }
        return MACHINE_KEY_ESC;
    }
    if (ch == '\r' || ch == '\n')
        return MACHINE_KEY_ENTER;
    if (ch == 127)
        return MACHINE_KEY_BACKSPACE;
    if (ch == ' ')
        return MACHINE_KEY_SPACE;
    return (long long)ch;
}
void machine_term_enable_mouse(void)
{
    if (isatty(STDOUT_FILENO))
        fputs("\033[?1000h\033[?1006h", stdout);
    fflush(stdout);
}
void machine_term_disable_mouse(void)
{
    if (isatty(STDOUT_FILENO))
        fputs("\033[?1000l\033[?1006l", stdout);
    fflush(stdout);
}
/* we implement functions to read keyboard input with optional timeouts, check if a key is available,
   and read special keys such as arrow keys, escape, enter, backspace, and space.
   we also implement functions to enable and disable mouse tracking in the terminal,
   which allows us to receive mouse events such as movement and button clicks. */
long long machine_term_last_key(void) { return machine_last_key; }
long long machine_term_mouse_x(void) { return machine_mouse_x; }
long long machine_term_mouse_y(void) { return machine_mouse_y; }
long long machine_term_mouse_button(void) { return machine_mouse_button; }
/* we provide functions to retrieve the last key pressed and the current mouse position and button state,
   which can be used in our programming language to respond to user input. */
long long machine_term_poll_event(void)
{
    if (!machine_term_key_available())
        return MACHINE_EVENT_NONE;
    machine_last_key = machine_term_read_key();
    return MACHINE_EVENT_KEY;
}
void machine_term_clear(void) { fputs("\033[2J\033[H", stdout); }
void machine_term_flush(void) { fflush(stdout); }
void machine_term_move_cursor(long long x, long long y) { printf("\033[%lld;%lldH", y, x); }
void machine_term_hide_cursor(void) { fputs("\033[?25l", stdout); }
void machine_term_show_cursor(void) { fputs("\033[?25h", stdout); }
void machine_term_draw_text(long long x, long long y, const char *text)
{
    machine_term_move_cursor(x, y);
    fputs(text, stdout);
}
/* we implement functions to clear the terminal screen, flush output, 
move the cursor to a specific position, hide and show the cursor, and draw text at a specific location. */

#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>) && __has_include(<SDL2/SDL_image.h>)
#define MACHINE_HAS_SDL 1
#endif
#endif
#ifndef MACHINE_HAS_SDL
#define MACHINE_HAS_SDL 0
#endif

/* if SDL2 is available, we implement functions for window management, drawing shapes and text, and loading and drawing images using SDL2.
   if SDL2 is not available, we provide stub implementations that do nothing or return default values.
   this allows our programming language to optionally use SDL2 for graphics if it's available, while still being able to run without it. */
#if MACHINE_HAS_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct MachineImage
{
    SDL_Texture *texture;
    int width;
    int height;
} MachineImage;
static SDL_Window *machine_window = NULL;
static SDL_Renderer *machine_renderer = NULL;
int machine_window_open = 0;

/* we define a structure for images that includes an SDL_Texture and its dimensions, 
   as well as static variables to track the SDL_Window, SDL_Renderer, and whether the window is open.
   these are used in the implementation of the window management and 
   drawing functions that utilize SDL2 for graphics operations. */
void machine_win_require_renderer(void)
{
    if (!machine_renderer)
        machine_panic("window is not created");
}
int machine_win_create(const char *title, long long width, long long height)
{
    if (machine_window_open)
        return 1;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
        return 0;
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    machine_window = SDL_CreateWindow(title ? title : "Machine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)width, (int)height, SDL_WINDOW_SHOWN);
    if (!machine_window)
    {
        IMG_Quit();
        SDL_Quit();
        return 0;
    }
    machine_renderer = SDL_CreateRenderer(machine_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!machine_renderer)
        machine_renderer = SDL_CreateRenderer(machine_window, -1, SDL_RENDERER_SOFTWARE);
    if (!machine_renderer)
    {
        SDL_DestroyWindow(machine_window);
        machine_window = NULL;
        IMG_Quit();
        SDL_Quit();
        return 0;
    }
    machine_window_open = 1;
    return 1;
}
void machine_win_destroy(void)
{
    if (machine_renderer)
        SDL_DestroyRenderer(machine_renderer);
    if (machine_window)
        SDL_DestroyWindow(machine_window);
    machine_renderer = NULL;
    machine_window = NULL;
    if (machine_window_open)
    {
        IMG_Quit();
        SDL_Quit();
    }
    machine_window_open = 0;
}
int machine_win_is_open(void) { return machine_window_open; }
/* we implement functions to create and destroy a window using SDL2, 
   check if the window is open, and require that a renderer is available for drawing operations.
   these functions handle the initialization and cleanup of SDL resources, 
   as well as providing an interface for managing the window state. */
long long machine_win_last_key(void) { return machine_last_key; }
long long machine_win_mouse_x(void) { return machine_mouse_x; }
long long machine_win_mouse_y(void) { return machine_mouse_y; }
/* we provide functions to retrieve the last key pressed and the current mouse position,
   which can be used in our programming language to respond to user input in the context of a graphical window. */
long long machine_win_mouse_button(void) { return machine_mouse_button; }
long long machine_win_poll_event(void)
{
    SDL_Event e;
    if (!machine_window_open)
        return MACHINE_EVENT_NONE;
    if (!SDL_PollEvent(&e))
        return MACHINE_EVENT_NONE;
    if (e.type == SDL_QUIT)
    {
        machine_window_open = 0;
        return MACHINE_EVENT_QUIT;
    }
    if (e.type == SDL_KEYDOWN)
    {
        machine_last_key = (long long)e.key.keysym.sym;
        return MACHINE_EVENT_KEY;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION)
    {
        machine_mouse_x = e.type == SDL_MOUSEMOTION ? e.motion.x : e.button.x;
        machine_mouse_y = e.type == SDL_MOUSEMOTION ? e.motion.y : e.button.y;
        machine_mouse_button = e.type == SDL_MOUSEMOTION ? 0 : e.button.button;
        return MACHINE_EVENT_MOUSE;
    }
    return MACHINE_EVENT_NONE;
}
void machine_win_set_title(const char *title)
{
    if (machine_window)
        SDL_SetWindowTitle(machine_window, title ? title : "Machine");
}
void machine_win_clear(long long r, long long g, long long b)
{
    machine_win_require_renderer();
    SDL_SetRenderDrawColor(machine_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderClear(machine_renderer);
}
void machine_win_present(void)
{
    machine_win_require_renderer();
    SDL_RenderPresent(machine_renderer);
}
/* we implement functions to poll for events from the SDL event queue, set the window title,
   clear the window with a specific color, a
   nd present the rendered content to the screen.
   these functions allow our programming language to 
   interact with the graphical window and respond to user input and rendering operations. */
void machine_win_draw_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b)
{
    SDL_Rect rect;
    machine_win_require_renderer();
    rect.x = (int)x;
    rect.y = (int)y;
    rect.w = (int)w;
    rect.h = (int)h;
    SDL_SetRenderDrawColor(machine_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderDrawRect(machine_renderer, &rect);
}
void machine_win_fill_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b)
{
    SDL_Rect rect;
    machine_win_require_renderer();
    rect.x = (int)x;
    rect.y = (int)y;
    rect.w = (int)w;
    rect.h = (int)h;
    SDL_SetRenderDrawColor(machine_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderFillRect(machine_renderer, &rect);
}
void machine_win_draw_line(long long x1, long long y1, long long x2, long long y2, long long r, long long g, long long b)
{
    machine_win_require_renderer();
    SDL_SetRenderDrawColor(machine_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderDrawLine(machine_renderer, (int)x1, (int)y1, (int)x2, (int)y2);
}
void machine_win_draw_pixel(long long x, long long y, long long r, long long g, long long b)
{
    machine_win_require_renderer();
    SDL_SetRenderDrawColor(machine_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderDrawPoint(machine_renderer, (int)x, (int)y);
}
void machine_win_draw_text(long long x, long long y, const char *text)
{
    (void)x;
    (void)y;
    (void)text;
}
/* we implement functions to draw rectangles (both outlined and filled), lines, and pixels using SDL2's rendering functions.
   the function to draw text is left as a stub, as it 
   would require additional setup for font rendering with SDL_ttf or a similar library.
   these drawing functions allow our programming language to create graphical content in the window. */
void *machine_image_load(const char *path)
{
    MachineImage *img;
    SDL_Surface *surf;
    machine_win_require_renderer();
    surf = IMG_Load(path);
    if (!surf)
        surf = SDL_LoadBMP(path);
    if (!surf)
        return NULL;
    img = (MachineImage *)calloc(1, sizeof(MachineImage));
    if (!img)
    {
        SDL_FreeSurface(surf);
        machine_panic("image allocation failed");
    }
    img->width = surf->w;
    img->height = surf->h;
    img->texture = SDL_CreateTextureFromSurface(machine_renderer, surf);
    SDL_FreeSurface(surf);
    if (!img->texture)
    {
        free(img);
        return NULL;
    }
    return img;
}
long long machine_image_width(void *ptr)
{
    MachineImage *img = (MachineImage *)ptr;
    return img ? (long long)img->width : 0;
}
long long machine_image_height(void *ptr)
{
    MachineImage *img = (MachineImage *)ptr;
    return img ? (long long)img->height : 0;
}
void machine_image_draw(void *ptr, long long x, long long y)
{
    MachineImage *img = (MachineImage *)ptr;
    SDL_Rect dst;
    machine_win_require_renderer();
    if (!img || !img->texture)
        return;
    dst.x = (int)x;
    dst.y = (int)y;
    dst.w = img->width;
    dst.h = img->height;
    SDL_RenderCopy(machine_renderer, img->texture, NULL, &dst);
}
void machine_image_draw_scaled(void *ptr, long long x, long long y, long long w, long long h)
{
    MachineImage *img = (MachineImage *)ptr;
    SDL_Rect dst;
    machine_win_require_renderer();
    if (!img || !img->texture)
        return;
    dst.x = (int)x;
    dst.y = (int)y;
    dst.w = (int)w;
    dst.h = (int)h;
    SDL_RenderCopy(machine_renderer, img->texture, NULL, &dst);
}
void machine_image_free(void *ptr)
{
    MachineImage *img = (MachineImage *)ptr;
    if (!img)
        return;
    if (img->texture)
        SDL_DestroyTexture(img->texture);
    free(img);
}
/* we implement functions to load an image from a file, retrieve its dimensions, 
   draw it at a specific position, draw it scaled to specific dimensions, and free the image resources.
   these functions allow our programming language to work with images in the graphical window using SDL2. */

#else

int machine_win_create(const char *title, long long width, long long height)
{
    (void)title;
    (void)width;
    (void)height;
    return 0;
}
void machine_win_destroy(void) {}
int machine_win_is_open(void) { return 0; }
/* if SDL2 is not available, we provide stub implementations for all the 
   window management and drawing functions that simply do nothing or return default values.
   this allows our programming language to compile and run without SDL2, 
   albeit without any graphical capabilities. */
long long machine_win_last_key(void) { return 0; }
long long machine_win_mouse_x(void) { return 0; }
long long machine_win_mouse_y(void) { return 0; }
long long machine_win_mouse_button(void) { return 0; }
long long machine_win_poll_event(void) { return MACHINE_EVENT_NONE; }
/* we provide stub implementations for the functions to retrieve the last key pressed
   and the current mouse position and button state, as well as the function to poll for events,
   which all return default values indicating no input or events. */
void machine_win_set_title(const char *title) { (void)title; }
void machine_win_clear(long long r, long long g, long long b)
{
    (void)r;
    (void)g;
    (void)b;
}
void machine_win_present(void) {}
/* we provide stub implementations for the functions to set the window title, clear the window, and present the rendered content, which do nothing in the absence of SDL2. */
void machine_win_draw_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b)
{
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)r;
    (void)g;
    (void)b;
}
void machine_win_fill_rect(long long x, long long y, long long w, long long h, long long r, long long g, long long b)
{
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)r;
    (void)g;
    (void)b;
}
void machine_win_draw_line(long long x1, long long y1, long long x2, long long y2, long long r, long long g, long long b)
{
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)r;
    (void)g;
    (void)b;
}
void machine_win_draw_pixel(long long x, long long y, long long r, long long g, long long b)
{
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
}
void machine_win_draw_text(long long x, long long y, const char *text)
{
    (void)x;
    (void)y;
    (void)text;
}
void *machine_image_load(const char *path)
{
    (void)path;
    return NULL;
}
long long machine_image_width(void *ptr)
{
    (void)ptr;
    return 0;
}
long long machine_image_height(void *ptr)
{
    (void)ptr;
    return 0;
}
void machine_image_draw(void *ptr, long long x, long long y)
{
    (void)ptr;
    (void)x;
    (void)y;
}
void machine_image_draw_scaled(void *ptr, long long x, long long y, long long w, long long h)
{
    (void)ptr;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
}
void machine_image_free(void *ptr) { (void)ptr; }
#endif
/* we provide stub implementations for all the drawing and image functions that do nothing, 
   allowing the program to compile and run without SDL2, albeit without any graphical capabilities. */

long long machine_video_play(const char *path)
{
    if (!path || access(path, R_OK) != 0)
        return -1;
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0)
    {
        int nullfd = open("/dev/null", O_WRONLY);
        if (nullfd >= 0)
        {
            dup2(nullfd, STDERR_FILENO);
            close(nullfd);
        }
        execlp("ffplay", "ffplay", "-autoexit", "-loglevel", "error", path, (char *)NULL);
        _exit(127);
    }
    return (long long)pid;
}
int machine_video_is_running(long long pid_value)
{
    pid_t pid = (pid_t)pid_value;
    int status = 0;
    pid_t r;
    if (pid <= 0)
        return 0;
    r = waitpid(pid, &status, WNOHANG);
    if (r == 0)
        return 1;
    return 0;
}
void machine_video_stop(long long pid_value)
{
    pid_t pid = (pid_t)pid_value;
    if (pid <= 0)
        return;
    kill(pid, SIGTERM);
}
