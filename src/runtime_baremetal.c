/*
 * Annotated reading copy of runtime_baremetal.c
 *
 * What this file is for:
 * - Provide the bare-metal runtime implementation for low-level Machine targets without a host operating system.
 *
 * How to read this file:
 * - First scan the includes to see which data structures and declarations this unit depends on.
 * - Then identify the major helper layers: low-level primitives, transformation helpers,
 *   public entry points, and any error-reporting or cleanup paths.
 * - Pay attention to stateful objects such as source files, token streams, AST nodes,
 *   emitted output buffers, runtime data, or target-specific configuration.
 *
 * Annotation policy:
 * - The original code body is preserved.
 * - Only explanatory comments are added.
 * - These comments are intended for learning and code-reading, not as a behavioral change.
 */

#include "machine_runtime_baremetal.h"

#include "pointer.c"

extern char _end;
extern void machine_isr_default(void);
extern void machine_isr_vector0(void);
extern void machine_isr_vector1(void);
extern void machine_isr_vector2(void);
extern void machine_isr_vector3(void);
extern void machine_isr_vector4(void);
extern void machine_isr_vector5(void);
extern void machine_isr_vector6(void);
extern void machine_isr_vector7(void);
extern void machine_isr_vector8(void);
extern void machine_isr_vector9(void);
extern void machine_isr_vector10(void);
extern void machine_isr_vector11(void);
extern void machine_isr_vector12(void);
extern void machine_isr_vector13(void);
extern void machine_isr_vector14(void);
extern void machine_isr_vector15(void);
extern void machine_isr_vector16(void);
extern void machine_isr_vector17(void);
extern void machine_isr_vector18(void);
extern void machine_isr_vector19(void);
extern void machine_isr_vector20(void);
extern void machine_isr_vector21(void);
extern void machine_isr_vector22(void);
extern void machine_isr_vector23(void);
extern void machine_isr_vector24(void);
extern void machine_isr_vector25(void);
extern void machine_isr_vector26(void);
extern void machine_isr_vector27(void);
extern void machine_isr_vector28(void);
extern void machine_isr_vector29(void);
extern void machine_isr_vector30(void);
extern void machine_isr_vector31(void);
extern void machine_isr_irq0(void);
extern void machine_isr_irq1(void);
extern unsigned long long machine_boot_pd_table0[];
extern unsigned long long machine_boot_pd_table1[];
extern unsigned long long machine_boot_pd_table2[];
extern unsigned long long machine_boot_pd_table3[];

typedef void (*machine_isr_stub_t)(void);

typedef struct
{
    unsigned short offset_low;
    unsigned short selector;
    unsigned char ist;
    unsigned char type_attr;
    unsigned short offset_mid;
    unsigned int offset_high;
    unsigned int reserved;
} __attribute__((packed)) MachineIDTEntry;

typedef struct
{
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) MachineIDTR;

static volatile unsigned short *const machine_vga = (volatile unsigned short *)0xB8000;
static MachineIDTEntry machine_idt[256];
static long long machine_cursor_row = 0;
static long long machine_cursor_col = 0;
static unsigned char machine_vga_attr = 0x0F;
static unsigned long long machine_heap_ptr = 0;
static unsigned long long machine_heap_end = 0;
static unsigned long long machine_pmm_total = 0;
static volatile unsigned long long machine_ticks_ms = 0;
static int machine_serial_ready = 0;
static int machine_debugcon_ready = 1;
static int machine_apic_enabled = 0;
static unsigned long long machine_apic_base = 0;
static unsigned long long machine_timer_base_ms = 0;
static int machine_term_raw = 0;

static long long machine_last_key = 0;
static long long machine_mouse_x = 0;
static long long machine_mouse_y = 0;
static long long machine_mouse_button = 0;
static long long machine_key_queue[64];
static unsigned int machine_key_head = 0;
static unsigned int machine_key_tail = 0;

static const machine_isr_stub_t machine_exception_stubs[32] = {
    machine_isr_vector0, machine_isr_vector1, machine_isr_vector2, machine_isr_vector3,
    machine_isr_vector4, machine_isr_vector5, machine_isr_vector6, machine_isr_vector7,
    machine_isr_vector8, machine_isr_vector9, machine_isr_vector10, machine_isr_vector11,
    machine_isr_vector12, machine_isr_vector13, machine_isr_vector14, machine_isr_vector15,
    machine_isr_vector16, machine_isr_vector17, machine_isr_vector18, machine_isr_vector19,
    machine_isr_vector20, machine_isr_vector21, machine_isr_vector22, machine_isr_vector23,
    machine_isr_vector24, machine_isr_vector25, machine_isr_vector26, machine_isr_vector27,
    machine_isr_vector28, machine_isr_vector29, machine_isr_vector30, machine_isr_vector31};

/*
 * Function overview: machine_strlen_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine strlen raw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static long long machine_strlen_raw(const char *s)
{
    long long n = 0;
    if (!s)
        return 0;
    while (s[n])
        ++n;
    return n;
}

static void *machine_memcpy_raw(void *dst, const void *src, long long n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (long long i = 0; i < n; ++i)
        d[i] = s[i];
    return dst;
}

static void *machine_memset_raw(void *dst, int value, long long n)
{
    unsigned char *d = (unsigned char *)dst;
    for (long long i = 0; i < n; ++i)
        d[i] = (unsigned char)value;
    return dst;
}

/*
 * Function overview: machine_align_up_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine align up u64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static unsigned long long machine_align_up_u64(unsigned long long value, unsigned long long align)
{
    return (value + align - 1ULL) & ~(align - 1ULL);
}

/*
 * Function overview: machine_serial_wait_tx
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine serial wait tx".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_serial_wait_tx(void)
{
    if (!machine_serial_ready)
        return;
    while ((machine_cpu_in8(0x3FD) & 0x20) == 0)
    {
    }
}

/*
 * Function overview: machine_serial_init
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine serial init".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_serial_init(void)
{
    machine_cpu_out8(0x3F9, 0x00);
    machine_cpu_out8(0x3FB, 0x80);
    machine_cpu_out8(0x3F8, 0x03);
    machine_cpu_out8(0x3F9, 0x00);
    machine_cpu_out8(0x3FB, 0x03);
    machine_cpu_out8(0x3FA, 0xC7);
    machine_cpu_out8(0x3FC, 0x0B);
    machine_serial_ready = 1;
}

/*
 * Function overview: machine_serial_putc
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine serial putc".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_serial_putc(char ch)
{
    if (!machine_serial_ready)
        return;
    machine_serial_wait_tx();
    machine_cpu_out8(0x3F8, (unsigned char)ch);
}

/*
 * Function overview: machine_debugcon_putc
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine debugcon putc".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_debugcon_putc(char ch)
{
    if (!machine_debugcon_ready)
        return;
    machine_cpu_out8(0xE9, (unsigned char)ch);
}

/*
 * Function overview: machine_cpuid_leaf1_edx
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine cpuid leaf1 edx".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static unsigned int machine_cpuid_leaf1_edx(void)
{
    unsigned int eax = 1U;
    unsigned int ebx = 0U;
    unsigned int ecx = 0U;
    unsigned int edx = 0U;
    __asm__ volatile("cpuid" : "+a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));
    (void)ebx;
    (void)ecx;
    return edx;
}

/*
 * Function overview: machine_rdmsr
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine rdmsr".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static unsigned long long machine_rdmsr(unsigned int msr)
{
    unsigned int lo = 0U;
    unsigned int hi = 0U;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((unsigned long long)hi << 32) | (unsigned long long)lo;
}

/*
 * Function overview: machine_wrmsr
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine wrmsr".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_wrmsr(unsigned int msr, unsigned long long value)
{
    unsigned int lo = (unsigned int)(value & 0xFFFFFFFFULL);
    unsigned int hi = (unsigned int)(value >> 32);
    __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}

static volatile unsigned int *machine_apic_reg(unsigned int reg)
{
    if (machine_apic_base == 0ULL)
        return (volatile unsigned int *)(uintptr_t)0;
    return (volatile unsigned int *)(uintptr_t)(machine_apic_base + (unsigned long long)reg);
}

/*
 * Function overview: machine_apic_read
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine apic read".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static unsigned int machine_apic_read(unsigned int reg)
{
    volatile unsigned int *ptr = machine_apic_reg(reg);
    return ptr ? *ptr : 0U;
}

/*
 * Function overview: machine_apic_write
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine apic write".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_apic_write(unsigned int reg, unsigned int value)
{
    volatile unsigned int *ptr = machine_apic_reg(reg);
    if (ptr)
        *ptr = value;
}

static unsigned long long *machine_pd_table_for_index(unsigned long long page_index, unsigned long long *local_index)
{
    if (page_index < 512ULL)
    {
        *local_index = page_index;
        return machine_boot_pd_table0;
    }
    if (page_index < 1024ULL)
    {
        *local_index = page_index - 512ULL;
        return machine_boot_pd_table1;
    }
    if (page_index < 1536ULL)
    {
        *local_index = page_index - 1024ULL;
        return machine_boot_pd_table2;
    }
    if (page_index < 2048ULL)
    {
        *local_index = page_index - 1536ULL;
        return machine_boot_pd_table3;
    }
    return NULL;
}

/*
 * Function overview: machine_vga_clear_screen
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine vga clear screen".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_vga_clear_screen(void)
{
    for (int i = 0; i < 80 * 25; ++i)
        machine_vga[i] = ((unsigned short)machine_vga_attr << 8) | ' ';
    machine_cursor_row = 0;
    machine_cursor_col = 0;
}

/*
 * Function overview: machine_vga_scroll
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine vga scroll".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_vga_scroll(void)
{
    for (int row = 1; row < 25; ++row)
        for (int col = 0; col < 80; ++col)
            machine_vga[(row - 1) * 80 + col] = machine_vga[row * 80 + col];
    for (int col = 0; col < 80; ++col)
        machine_vga[24 * 80 + col] = ((unsigned short)machine_vga_attr << 8) | ' ';
    machine_cursor_row = 24;
}

/*
 * Function overview: machine_putc
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine putc".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_putc(char ch)
{
    if (machine_serial_ready)
    {
        if (ch == '\n')
            machine_serial_putc('\r');
        machine_serial_putc(ch);
    }
    if (machine_debugcon_ready)
    {
        if (ch == '\n')
            machine_debugcon_putc('\r');
        machine_debugcon_putc(ch);
    }
    if (ch == '\n')
    {
        machine_cursor_col = 0;
        ++machine_cursor_row;
        if (machine_cursor_row >= 25)
            machine_vga_scroll();
        return;
    }
    if (ch == '\r')
    {
        machine_cursor_col = 0;
        return;
    }
    if (ch == '\b')
    {
        if (machine_cursor_col > 0)
            --machine_cursor_col;
        machine_vga[machine_cursor_row * 80 + machine_cursor_col] = ((unsigned short)machine_vga_attr << 8) | ' ';
        return;
    }
    machine_vga[machine_cursor_row * 80 + machine_cursor_col] = ((unsigned short)machine_vga_attr << 8) | (unsigned char)ch;
    ++machine_cursor_col;
    if (machine_cursor_col >= 80)
    {
        machine_cursor_col = 0;
        ++machine_cursor_row;
        if (machine_cursor_row >= 25)
            machine_vga_scroll();
    }
}

/*
 * Function overview: machine_print_u64_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print u64 raw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_print_u64_raw(unsigned long long value)
{
    char buf[32];
    int i = 0;
    if (value == 0)
    {
        machine_putc('0');
        return;
    }
    while (value != 0 && i < (int)sizeof(buf))
    {
        buf[i++] = (char)('0' + (value % 10ULL));
        value /= 10ULL;
    }
    while (i > 0)
        machine_putc(buf[--i]);
}

/*
 * Function overview: machine_print_exception_banner
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print exception banner".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_print_exception_banner(long long vector, long long error_code)
{
    machine_print_str("[machine baremetal exception]");
    machine_putc('v');
    machine_putc('=');
    machine_print_i64(vector);
    machine_putc('e');
    machine_putc('=');
    machine_print_i64(error_code);
}

/*
 * Function overview: machine_halt_forever
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine halt forever".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_halt_forever(void)
{
    for (;;)
        __asm__ volatile("cli; hlt");
}

/*
 * Function overview: machine_key_enqueue
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine key enqueue".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_key_enqueue(long long key)
{
    unsigned int next = (machine_key_tail + 1U) % (unsigned int)(sizeof(machine_key_queue) / sizeof(machine_key_queue[0]));
    if (next == machine_key_head)
        return;
    machine_key_queue[machine_key_tail] = key;
    machine_key_tail = next;
    machine_last_key = key;
}

/*
 * Function overview: machine_key_dequeue
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine key dequeue".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static long long machine_key_dequeue(void)
{
    if (machine_key_head == machine_key_tail)
        return 0;
    long long key = machine_key_queue[machine_key_head];
    machine_key_head = (machine_key_head + 1U) % (unsigned int)(sizeof(machine_key_queue) / sizeof(machine_key_queue[0]));
    return key;
}

/*
 * Function overview: machine_keyboard_translate
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine keyboard translate".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static long long machine_keyboard_translate(unsigned char scancode)
{
    if ((scancode & 0x80U) != 0U)
        return 0;
    switch (scancode)
    {
    case 0x01:
        return 27;
    case 0x02:
        return '1';
    case 0x03:
        return '2';
    case 0x04:
        return '3';
    case 0x05:
        return '4';
    case 0x06:
        return '5';
    case 0x07:
        return '6';
    case 0x08:
        return '7';
    case 0x09:
        return '8';
    case 0x0A:
        return '9';
    case 0x0B:
        return '0';
    case 0x0C:
        return '-';
    case 0x0D:
        return '=';
    case 0x0E:
        return '\b';
    case 0x0F:
        return '\t';
    case 0x10:
        return 'q';
    case 0x11:
        return 'w';
    case 0x12:
        return 'e';
    case 0x13:
        return 'r';
    case 0x14:
        return 't';
    case 0x15:
        return 'y';
    case 0x16:
        return 'u';
    case 0x17:
        return 'i';
    case 0x18:
        return 'o';
    case 0x19:
        return 'p';
    case 0x1A:
        return '[';
    case 0x1B:
        return ']';
    case 0x1C:
        return '\n';
    case 0x1E:
        return 'a';
    case 0x1F:
        return 's';
    case 0x20:
        return 'd';
    case 0x21:
        return 'f';
    case 0x22:
        return 'g';
    case 0x23:
        return 'h';
    case 0x24:
        return 'j';
    case 0x25:
        return 'k';
    case 0x26:
        return 'l';
    case 0x27:
        return ';';
    case 0x28:
        return '\'';
    case 0x29:
        return '`';
    case 0x2B:
        return '\\';
    case 0x2C:
        return 'z';
    case 0x2D:
        return 'x';
    case 0x2E:
        return 'c';
    case 0x2F:
        return 'v';
    case 0x30:
        return 'b';
    case 0x31:
        return 'n';
    case 0x32:
        return 'm';
    case 0x33:
        return ',';
    case 0x34:
        return '.';
    case 0x35:
        return '/';
    case 0x39:
        return ' ';
    default:
        return 0;
    }
}

/*
 * Function overview: machine_pic_send_eoi
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine pic send eoi".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_pic_send_eoi(long long vector)
{
    if (vector >= 40)
        machine_cpu_out8(0xA0, 0x20);
    if (vector >= 32 && vector < 48)
        machine_cpu_out8(0x20, 0x20);
}

/*
 * Function overview: machine_pic_remap
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine pic remap".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_pic_remap(void)
{
    unsigned char master_mask = (unsigned char)machine_cpu_in8(0x21);
    unsigned char slave_mask = (unsigned char)machine_cpu_in8(0xA1);

    machine_cpu_out8(0x20, 0x11);
    machine_cpu_out8(0xA0, 0x11);
    machine_cpu_out8(0x21, 0x20);
    machine_cpu_out8(0xA1, 0x28);
    machine_cpu_out8(0x21, 0x04);
    machine_cpu_out8(0xA1, 0x02);
    machine_cpu_out8(0x21, 0x01);
    machine_cpu_out8(0xA1, 0x01);

    (void)master_mask;
    (void)slave_mask;
    machine_cpu_out8(0x21, 0xFC);
    machine_cpu_out8(0xA1, 0xFF);
}

/*
 * Function overview: machine_pit_init
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine pit init".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_pit_init(unsigned int hz)
{
    unsigned int divisor;
    if (hz == 0U)
        hz = 1000U;
    divisor = 1193182U / hz;
    if (divisor == 0U)
        divisor = 1U;
    machine_cpu_out8(0x43, 0x36);
    machine_cpu_out8(0x40, divisor & 0xFFU);
    machine_cpu_out8(0x40, (divisor >> 8) & 0xFFU);
}

/*
 * Function overview: machine_idt_set_gate
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine idt set gate".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_idt_set_gate(int vector, machine_isr_stub_t stub)
{
    unsigned long long addr = (unsigned long long)(uintptr_t)stub;
    MachineIDTEntry *entry = &machine_idt[vector];
    entry->offset_low = (unsigned short)(addr & 0xFFFFULL);
    entry->selector = 0x08;
    entry->ist = 0;
    entry->type_attr = 0x8E;
    entry->offset_mid = (unsigned short)((addr >> 16) & 0xFFFFULL);
    entry->offset_high = (unsigned int)((addr >> 32) & 0xFFFFFFFFULL);
    entry->reserved = 0;
}

/*
 * Function overview: machine_idt_init
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine idt init".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void machine_idt_init(void)
{
    MachineIDTR idtr;
    for (int i = 0; i < 256; ++i)
        machine_idt_set_gate(i, machine_isr_default);
    for (int i = 0; i < 32; ++i)
        machine_idt_set_gate(i, machine_exception_stubs[i]);
    machine_idt_set_gate(32, machine_isr_irq0);
    machine_idt_set_gate(33, machine_isr_irq1);
    idtr.limit = (unsigned short)(sizeof(machine_idt) - 1U);
    idtr.base = (unsigned long long)(uintptr_t)&machine_idt[0];
    __asm__ volatile("lidt %0" : : "m"(idtr));
}

/*
 * Function overview: machine_interrupt_dispatch
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine interrupt dispatch".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_interrupt_dispatch(long long vector, long long error_code, void *frame)
{
    (void)frame;
    if (vector == 32)
    {
        ++machine_ticks_ms;
        machine_pic_send_eoi(vector);
        return;
    }
    if (vector == 33)
    {
        unsigned char scancode = (unsigned char)machine_cpu_in8(0x60);
        long long key = machine_keyboard_translate(scancode);
        if (key != 0)
            machine_key_enqueue(key);
        machine_pic_send_eoi(vector);
        return;
    }
    if (vector >= 32 && vector < 48)
    {
        machine_pic_send_eoi(vector);
        return;
    }
    machine_print_exception_banner(vector, error_code);
    machine_halt_forever();
}

/*
 * Function overview: machine_baremetal_init
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine baremetal init".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_baremetal_init(void)
{
    __asm__ volatile("cli");
    machine_serial_init();
    machine_vga_clear_screen();
    machine_heap_ptr = machine_align_up_u64((unsigned long long)(uintptr_t)&_end, 4096ULL);
    machine_heap_end = machine_heap_ptr + 0x04000000ULL;
    machine_pmm_total = machine_heap_end - machine_heap_ptr;
    machine_ticks_ms = 0;
    machine_timer_base_ms = 0;
    machine_last_key = 0;
    machine_mouse_x = 0;
    machine_mouse_y = 0;
    machine_mouse_button = 0;
    machine_key_head = 0;
    machine_key_tail = 0;
    machine_apic_enabled = 0;
    machine_apic_base = 0;
    machine_idt_init();
    machine_pic_remap();
    machine_pit_init(1000U);
    __asm__ volatile("sti");
}

/*
 * Function overview: machine_print_str
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print str".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_print_str(const char *s)
{
    if (!s)
        s = "";
    while (*s)
        machine_putc(*s++);
    machine_putc('\n');
}

/*
 * Function overview: machine_print_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print i64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_print_i64(long long value)
{
    unsigned long long magnitude;
    if (value < 0)
    {
        machine_putc('-');
        magnitude = (unsigned long long)(-(value + 1LL)) + 1ULL;
    }
    else
    {
        magnitude = (unsigned long long)value;
    }
    machine_print_u64_raw(magnitude);
    machine_putc('\n');
}

/*
 * Function overview: machine_print_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print f64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_print_f64(double value)
{
    (void)value;
    machine_print_str("[f64 print unsupported on baremetal]");
}

/*
 * Function overview: machine_print_hp
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine print hp".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_print_hp(long double value)
{
    (void)value;
    machine_print_str("[hp print unsupported on baremetal]");
}

char *machine_strdup(const char *s)
{
    long long n = machine_strlen_raw(s);
    char *p = (char *)machine_alloc_bytes(n + 1);
    machine_memcpy_raw(p, s, n + 1);
    return p;
}

char *machine_concat(const char *a, const char *b)
{
    long long la = machine_strlen_raw(a);
    long long lb = machine_strlen_raw(b);
    char *p = (char *)machine_alloc_bytes(la + lb + 1);
    machine_memcpy_raw(p, a, la);
    machine_memcpy_raw(p + la, b, lb + 1);
    return p;
}

/*
 * Function overview: machine_len
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine len".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_len(const char *s) { return machine_strlen_raw(s); }

char *machine_index_str(const char *s, long long i)
{
    static char slots[8][2];
    static int slot = 0;
    long long n = machine_strlen_raw(s);
    if (i < 0 || i >= n)
        return "";
    slot = (slot + 1) & 7;
    slots[slot][0] = s[i];
    slots[slot][1] = '\0';
    return slots[slot];
}

/*
 * Function overview: machine_hp_from_text
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp from text".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_from_text(const char *s)
{
    (void)s;
    return 0.0L;
}
/*
 * Function overview: machine_hp_add
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp add".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_add(long double a, long double b) { return a + b; }
/*
 * Function overview: machine_hp_sub
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp sub".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_sub(long double a, long double b) { return a - b; }
/*
 * Function overview: machine_hp_mul
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp mul".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_mul(long double a, long double b) { return a * b; }
/*
 * Function overview: machine_hp_div
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp div".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_div(long double a, long double b) { return b != 0.0L ? a / b : 0.0L; }
/*
 * Function overview: machine_hp_sqrt
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp sqrt".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_sqrt(long double a) { return a; }
/*
 * Function overview: machine_hp_pow
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine hp pow".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long double machine_hp_pow(long double a, long double b)
{
    (void)b;
    return a;
}

void *machine_alloc_bytes(long long n)
{
    if (n <= 0)
        n = 1;
    unsigned long long start = machine_align_up_u64(machine_heap_ptr, 16ULL);
    unsigned long long end = start + (unsigned long long)((n + 15LL) & ~15LL);
    if (machine_heap_end != 0ULL && end > machine_heap_end)
        return NULL;
    machine_heap_ptr = end;
    machine_memset_raw((void *)(uintptr_t)start, 0, n);
    return (void *)(uintptr_t)start;
}

/*
 * Function overview: machine_free_mem
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine free mem".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_free_mem(void *p) { (void)p; }
/*
 * Function overview: machine_store_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store i64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_i64(void *p, long long v) { *(long long *)p = v; }
/*
 * Function overview: machine_load_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load i64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_load_i64(void *p) { return *(long long *)p; }
/*
 * Function overview: machine_store_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store f64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_f64(void *p, double v) { *(double *)p = v; }
/*
 * Function overview: machine_load_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load f64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
double machine_load_f64(void *p) { return *(double *)p; }
/*
 * Function overview: machine_store_str
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store str".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_str(void *p, char *v) { *(char **)p = v; }
char *machine_load_str(void *p) { return *(char **)p; }
void *machine_ptr_from_i64(long long value) { return (void *)(uintptr_t)value; }
/*
 * Function overview: machine_ptr_to_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine ptr to i64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_ptr_to_i64(void *p) { return (long long)(uintptr_t)p; }
void *machine_ptr_offset(void *p, long long offset) { return (void *)((unsigned char *)p + offset); }
/*
 * Function overview: machine_store_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store u8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_u8(void *p, long long v) { *(unsigned char *)p = (unsigned char)v; }
/*
 * Function overview: machine_store_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store u16".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_u16(void *p, long long v) { *(unsigned short *)p = (unsigned short)v; }
/*
 * Function overview: machine_store_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store u32".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_u32(void *p, long long v) { *(unsigned int *)p = (unsigned int)v; }
/*
 * Function overview: machine_store_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine store u64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_store_u64(void *p, long long v) { *(unsigned long long *)p = (unsigned long long)v; }
/*
 * Function overview: machine_load_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load u8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_load_u8(void *p) { return *(unsigned char *)p; }
/*
 * Function overview: machine_load_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load u16".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_load_u16(void *p) { return *(unsigned short *)p; }
/*
 * Function overview: machine_load_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load u32".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_load_u32(void *p) { return *(unsigned int *)p; }
/*
 * Function overview: machine_load_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine load u64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_load_u64(void *p) { return (long long)*(unsigned long long *)p; }
/*
 * Function overview: machine_volatile_store_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile store u8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_volatile_store_u8(void *p, long long v) { *(volatile unsigned char *)p = (unsigned char)v; }
/*
 * Function overview: machine_volatile_store_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile store u16".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_volatile_store_u16(void *p, long long v) { *(volatile unsigned short *)p = (unsigned short)v; }
/*
 * Function overview: machine_volatile_store_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile store u32".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_volatile_store_u32(void *p, long long v) { *(volatile unsigned int *)p = (unsigned int)v; }
/*
 * Function overview: machine_volatile_store_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile store u64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_volatile_store_u64(void *p, long long v) { *(volatile unsigned long long *)p = (unsigned long long)v; }
/*
 * Function overview: machine_volatile_load_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile load u8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_volatile_load_u8(void *p) { return *(volatile unsigned char *)p; }
/*
 * Function overview: machine_volatile_load_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile load u16".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_volatile_load_u16(void *p) { return *(volatile unsigned short *)p; }
/*
 * Function overview: machine_volatile_load_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile load u32".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_volatile_load_u32(void *p) { return *(volatile unsigned int *)p; }
/*
 * Function overview: machine_volatile_load_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine volatile load u64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_volatile_load_u64(void *p) { return (long long)*(volatile unsigned long long *)p; }
/*
 * Function overview: machine_syscall0
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall0".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall0(long long n)
{
    (void)n;
    return -1;
}
/*
 * Function overview: machine_syscall1
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall1".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall1(long long n, long long a1)
{
    (void)n;
    (void)a1;
    return -1;
}
/*
 * Function overview: machine_syscall2
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall2".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall2(long long n, long long a1, long long a2)
{
    (void)n;
    (void)a1;
    (void)a2;
    return -1;
}
/*
 * Function overview: machine_syscall3
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall3".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall3(long long n, long long a1, long long a2, long long a3)
{
    (void)n;
    (void)a1;
    (void)a2;
    (void)a3;
    return -1;
}
/*
 * Function overview: machine_syscall4
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall4".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall4(long long n, long long a1, long long a2, long long a3, long long a4)
{
    (void)n;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    return -1;
}
/*
 * Function overview: machine_syscall5
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall5".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall5(long long n, long long a1, long long a2, long long a3, long long a4, long long a5)
{
    (void)n;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    return -1;
}
/*
 * Function overview: machine_syscall6
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine syscall6".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_syscall6(long long n, long long a1, long long a2, long long a3, long long a4, long long a5, long long a6)
{
    (void)n;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    return -1;
}
void *machine_mmap_anon(long long size) { return machine_alloc_bytes(size); }
void *machine_mmap_anon_exec(long long size) { return machine_alloc_bytes(size); }
/*
 * Function overview: machine_munmap_mem
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine munmap mem".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_munmap_mem(void *p, long long size)
{
    (void)p;
    (void)size;
    return 0;
}
/*
 * Function overview: machine_fd_open_ro
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd open ro".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_open_ro(const char *path)
{
    (void)path;
    return -1;
}
/*
 * Function overview: machine_fd_open_wo
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd open wo".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_open_wo(const char *path)
{
    (void)path;
    return -1;
}
/*
 * Function overview: machine_fd_open_rw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd open rw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_open_rw(const char *path)
{
    (void)path;
    return -1;
}
/*
 * Function overview: machine_fd_close
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd close".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_close(long long fd)
{
    (void)fd;
    return -1;
}
/*
 * Function overview: machine_fd_read
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd read".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_read(long long fd, void *buf, long long size)
{
    (void)fd;
    (void)buf;
    (void)size;
    return -1;
}
/*
 * Function overview: machine_fd_write
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd write".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_write(long long fd, void *buf, long long size)
{
    (void)fd;
    (void)buf;
    (void)size;
    return -1;
}
/*
 * Function overview: machine_fd_seek
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine fd seek".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_fd_seek(long long fd, long long offset, long long whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return -1;
}
/*
 * Function overview: machine_ioctl_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine ioctl i64".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_ioctl_i64(long long fd, long long request, long long arg)
{
    (void)fd;
    (void)request;
    (void)arg;
    return -1;
}
/*
 * Function overview: machine_asm_nop
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm nop".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_asm_nop(void) { __asm__ volatile("nop"); }
/*
 * Function overview: machine_asm_pause
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm pause".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_asm_pause(void) { __asm__ volatile("pause"); }
/*
 * Function overview: machine_asm_mfence
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm mfence".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_asm_mfence(void) { __asm__ volatile("mfence" ::: "memory"); }
/*
 * Function overview: machine_asm_lfence
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm lfence".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_asm_lfence(void) { __asm__ volatile("lfence" ::: "memory"); }
/*
 * Function overview: machine_asm_sfence
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm sfence".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_asm_sfence(void) { __asm__ volatile("sfence" ::: "memory"); }
/*
 * Function overview: machine_asm_rdtsc
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine asm rdtsc".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_asm_rdtsc(void)
{
    unsigned int lo = 0;
    unsigned int hi = 0;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (long long)(((unsigned long long)hi << 32) | (unsigned long long)lo);
}
/*
 * Function overview: machine_cpu_in8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine cpu in8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_cpu_in8(long long port)
{
    unsigned char v = 0;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"((unsigned short)port));
    return v;
}
/*
 * Function overview: machine_cpu_out8
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine cpu out8".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_cpu_out8(long long port, long long value) { __asm__ volatile("outb %0, %1" : : "a"((unsigned char)value), "Nd"((unsigned short)port)); }

void *machine_pmm_alloc_page(void)
{
    return machine_pmm_alloc_pages(1);
}

void *machine_pmm_alloc_pages(long long count)
{
    unsigned long long bytes;
    unsigned long long start;
    unsigned long long end;
    if (count <= 0)
        return NULL;
    bytes = (unsigned long long)count * 4096ULL;
    start = machine_align_up_u64(machine_heap_ptr, 4096ULL);
    end = start + bytes;
    if (machine_heap_end != 0ULL && end > machine_heap_end)
        return NULL;
    machine_heap_ptr = end;
    machine_memset_raw((void *)(uintptr_t)start, 0, (long long)bytes);
    return (void *)(uintptr_t)start;
}

/*
 * Function overview: machine_pmm_total_bytes
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine pmm total bytes".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_pmm_total_bytes(void) { return (long long)machine_pmm_total; }
/*
 * Function overview: machine_pmm_used_bytes
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine pmm used bytes".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_pmm_used_bytes(void)
{
    if (machine_heap_ptr < machine_align_up_u64((unsigned long long)(uintptr_t)&_end, 4096ULL))
        return 0;
    return (long long)(machine_heap_ptr - machine_align_up_u64((unsigned long long)(uintptr_t)&_end, 4096ULL));
}

/*
 * Function overview: machine_page_identity_map_2m
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine page identity map m".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_page_identity_map_2m(long long page_index, long long phys_base, long long flags)
{
    unsigned long long local_index = 0ULL;
    unsigned long long *table = NULL;
    unsigned long long base = ((unsigned long long)phys_base) & ~0x1FFFFFULL;
    unsigned long long entry_flags = 0x083ULL | (((unsigned long long)flags) & 0xFFFULL);
    if (page_index < 0)
        return 0;
    table = machine_pd_table_for_index((unsigned long long)page_index, &local_index);
    if (!table)
        return 0;
    table[local_index] = base | entry_flags;
    __asm__ volatile("mov %%cr3, %%rax\n\tmov %%rax, %%cr3" : : : "rax", "memory");
    return 1;
}

/*
 * Function overview: machine_apic_supported
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine apic supported".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_apic_supported(void)
{
    return (machine_cpuid_leaf1_edx() & (1U << 9)) != 0U;
}

/*
 * Function overview: machine_apic_enable
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine apic enable".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_apic_enable(void)
{
    unsigned long long apic_msr;
    unsigned long long page_index;
    unsigned long long phys_base;
    if (!machine_apic_supported())
        return 0;
    apic_msr = machine_rdmsr(0x1B);
    apic_msr |= (1ULL << 11);
    machine_wrmsr(0x1B, apic_msr);
    machine_apic_base = apic_msr & 0xFFFFF000ULL;
    phys_base = machine_apic_base & ~0x1FFFFFULL;
    page_index = phys_base >> 21;
    if (!machine_page_identity_map_2m((long long)page_index, (long long)phys_base, 0x083LL))
        return 0;
    machine_apic_write(0xF0U, machine_apic_read(0xF0U) | 0x100U);
    machine_apic_enabled = 1;
    return 1;
}

/*
 * Function overview: machine_apic_eoi
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine apic eoi".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_apic_eoi(void)
{
    if (machine_apic_enabled)
        machine_apic_write(0xB0U, 0U);
}

MachineList *machine_list_new(void) { return (MachineList *)machine_alloc_bytes((long long)sizeof(MachineList)); }
/*
 * Function overview: machine_list_push_back
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine list push back".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_list_push_back(MachineList *list, long long value)
{
    if (!list)
        return;
    MachineListNode *n = (MachineListNode *)machine_alloc_bytes((long long)sizeof(MachineListNode));
    n->value = value;
    n->next = NULL;
    if (!list->head)
        list->head = n;
    else
        list->tail->next = n;
    list->tail = n;
    ++list->size;
}
/*
 * Function overview: machine_list_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine list get".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_list_get(MachineList *list, long long index)
{
    MachineListNode *n = list ? list->head : NULL;
    for (long long i = 0; n && i < index; ++i)
        n = n->next;
    return n ? n->value : 0;
}
/*
 * Function overview: machine_list_size
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine list size".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_list_size(MachineList *list) { return list ? list->size : 0; }
/*
 * Function overview: machine_list_free
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine list free".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_list_free(MachineList *list) { (void)list; }
MachineArray *machine_array_new(void)
{
    MachineArray *a = (MachineArray *)machine_alloc_bytes((long long)sizeof(MachineArray));
    a->capacity = 8;
    a->data = (long long *)machine_alloc_bytes((long long)(sizeof(long long) * (size_t)a->capacity));
    return a;
}
/*
 * Function overview: machine_array_push
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array push".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_array_push(MachineArray *a, long long value)
{
    if (!a)
        return;
    if (a->size >= a->capacity)
    {
        long long newcap = a->capacity ? a->capacity * 2 : 8;
        long long *nd = (long long *)machine_alloc_bytes((long long)(sizeof(long long) * (size_t)newcap));
        if (a->data)
            machine_memcpy_raw(nd, a->data, (long long)(sizeof(long long) * (size_t)a->size));
        a->data = nd;
        a->capacity = newcap;
    }
    a->data[a->size++] = value;
}
/*
 * Function overview: machine_array_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array get".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_array_get(MachineArray *a, long long index) { return (!a || index < 0 || index >= a->size) ? 0 : a->data[index]; }
/*
 * Function overview: machine_array_set
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array set".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_array_set(MachineArray *a, long long index, long long value)
{
    if (a && index >= 0 && index < a->size)
        a->data[index] = value;
}
/*
 * Function overview: machine_array_len
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array len".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_array_len(MachineArray *a) { return a ? a->size : 0; }
/*
 * Function overview: machine_array_free
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array free".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_array_free(MachineArray *a) { (void)a; }
MachineGrid *machine_grid_new(long long rows, long long cols)
{
    MachineGrid *g = (MachineGrid *)machine_alloc_bytes((long long)sizeof(MachineGrid));
    g->rows = rows;
    g->cols = cols;
    g->data = (long long *)machine_alloc_bytes((long long)(sizeof(long long) * (size_t)(rows * cols)));
    return g;
}
/*
 * Function overview: machine_grid_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid get".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_grid_get(MachineGrid *g, long long row, long long col) { return (!g || row < 0 || col < 0 || row >= g->rows || col >= g->cols) ? 0 : g->data[row * g->cols + col]; }
/*
 * Function overview: machine_grid_set
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid set".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_grid_set(MachineGrid *g, long long row, long long col, long long value)
{
    if (g && row >= 0 && col >= 0 && row < g->rows && col < g->cols)
        g->data[row * g->cols + col] = value;
}
/*
 * Function overview: machine_grid_rows
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid rows".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_grid_rows(MachineGrid *g) { return g ? g->rows : 0; }
/*
 * Function overview: machine_grid_cols
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid cols".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_grid_cols(MachineGrid *g) { return g ? g->cols : 0; }
/*
 * Function overview: machine_grid_fill
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid fill".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_grid_fill(MachineGrid *g, long long value)
{
    if (!g)
        return;
    for (long long i = 0; i < g->rows * g->cols; ++i)
        g->data[i] = value;
}
/*
 * Function overview: machine_grid_free
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid free".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_grid_free(MachineGrid *g) { (void)g; }

/*
 * Function overview: machine_tick_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine tick ms".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_tick_ms(void) { return (long long)machine_ticks_ms; }
/*
 * Function overview: machine_timer_reset
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine timer reset".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_timer_reset(void) { machine_timer_base_ms = machine_ticks_ms; }
/*
 * Function overview: machine_timer_elapsed_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine timer elapsed ms".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_timer_elapsed_ms(void) { return (long long)(machine_ticks_ms - machine_timer_base_ms); }
/*
 * Function overview: machine_sleep_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine sleep ms".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_sleep_ms(long long ms)
{
    unsigned long long deadline;
    if (ms <= 0)
        return;
    deadline = machine_ticks_ms + (unsigned long long)ms;
    while (machine_ticks_ms < deadline)
        __asm__ volatile("hlt");
}
/*
 * Function overview: machine_term_enable_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term enable raw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_enable_raw(void) { machine_term_raw = 1; }
/*
 * Function overview: machine_term_disable_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term disable raw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_disable_raw(void) { machine_term_raw = 0; }
/*
 * Function overview: machine_term_key_available
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term key available".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_term_key_available(void) { return machine_key_head != machine_key_tail; }
/*
 * Function overview: machine_term_read_key
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term read key".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_read_key(void) { return machine_key_dequeue(); }
/*
 * Function overview: machine_term_enable_mouse
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term enable mouse".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_enable_mouse(void) {}
/*
 * Function overview: machine_term_disable_mouse
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term disable mouse".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_disable_mouse(void) {}
/*
 * Function overview: machine_term_last_key
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term last key".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_last_key(void) { return machine_last_key; }
/*
 * Function overview: machine_term_mouse_x
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term mouse x".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_mouse_x(void) { return machine_mouse_x; }
/*
 * Function overview: machine_term_mouse_y
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term mouse y".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_mouse_y(void) { return machine_mouse_y; }
/*
 * Function overview: machine_term_mouse_button
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term mouse button".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_mouse_button(void) { return machine_mouse_button; }
/*
 * Function overview: machine_term_poll_event
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term poll event".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_term_poll_event(void)
{
    (void)machine_term_raw;
    return machine_term_key_available() ? 1 : 0;
}
/*
 * Function overview: machine_term_clear
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term clear".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_clear(void) { machine_vga_clear_screen(); }
/*
 * Function overview: machine_term_flush
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term flush".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_flush(void) {}
/*
 * Function overview: machine_term_move_cursor
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term move cursor".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_move_cursor(long long x, long long y)
{
    if (x >= 0 && x < 80)
        machine_cursor_col = x;
    if (y >= 0 && y < 25)
        machine_cursor_row = y;
}
/*
 * Function overview: machine_term_hide_cursor
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term hide cursor".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_hide_cursor(void) {}
/*
 * Function overview: machine_term_show_cursor
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term show cursor".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_show_cursor(void) {}
/*
 * Function overview: machine_term_draw_text
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine term draw text".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_term_draw_text(long long x, long long y, const char *text)
{
    machine_term_move_cursor(x, y);
    while (text && *text)
        machine_putc(*text++);
}
/*
 * Function overview: machine_win_create
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win create".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_win_create(const char *title, long long width, long long height)
{
    (void)title;
    (void)width;
    (void)height;
    return 0;
}
/*
 * Function overview: machine_win_destroy
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win destroy".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_destroy(void) {}
/*
 * Function overview: machine_win_is_open
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win is open".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_win_is_open(void) { return 0; }
/*
 * Function overview: machine_win_last_key
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win last key".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_win_last_key(void) { return machine_last_key; }
/*
 * Function overview: machine_win_mouse_x
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win mouse x".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_win_mouse_x(void) { return machine_mouse_x; }
/*
 * Function overview: machine_win_mouse_y
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win mouse y".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_win_mouse_y(void) { return machine_mouse_y; }
/*
 * Function overview: machine_win_mouse_button
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win mouse button".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_win_mouse_button(void) { return machine_mouse_button; }
/*
 * Function overview: machine_win_poll_event
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win poll event".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_win_poll_event(void) { return machine_term_poll_event(); }
/*
 * Function overview: machine_win_set_title
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win set title".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_set_title(const char *title) { (void)title; }
/*
 * Function overview: machine_win_clear
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win clear".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_clear(long long r, long long g, long long b)
{
    (void)r;
    (void)g;
    (void)b;
}
/*
 * Function overview: machine_win_present
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win present".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_present(void) {}
/*
 * Function overview: machine_win_draw_rect
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win draw rect".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
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
/*
 * Function overview: machine_win_fill_rect
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win fill rect".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
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
/*
 * Function overview: machine_win_draw_line
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win draw line".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
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
/*
 * Function overview: machine_win_draw_pixel
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win draw pixel".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_draw_pixel(long long x, long long y, long long r, long long g, long long b)
{
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
}
/*
 * Function overview: machine_win_draw_text
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine win draw text".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_win_draw_text(long long x, long long y, const char *text) { machine_term_draw_text(x, y, text); }
void *machine_image_load(const char *path)
{
    (void)path;
    return NULL;
}
/*
 * Function overview: machine_image_width
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine image width".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_image_width(void *ptr)
{
    (void)ptr;
    return 0;
}
/*
 * Function overview: machine_image_height
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine image height".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_image_height(void *ptr)
{
    (void)ptr;
    return 0;
}
/*
 * Function overview: machine_image_draw
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine image draw".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_image_draw(void *ptr, long long x, long long y)
{
    (void)ptr;
    (void)x;
    (void)y;
}
/*
 * Function overview: machine_image_draw_scaled
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine image draw scaled".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_image_draw_scaled(void *ptr, long long x, long long y, long long w, long long h)
{
    (void)ptr;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
}
/*
 * Function overview: machine_image_free
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine image free".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_image_free(void *ptr) { (void)ptr; }
/*
 * Function overview: machine_video_play
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine video play".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
long long machine_video_play(const char *path)
{
    (void)path;
    return -1;
}
/*
 * Function overview: machine_video_is_running
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine video is running".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
int machine_video_is_running(long long pid_value)
{
    (void)pid_value;
    return 0;
}
/*
 * Function overview: machine_video_stop
 *
 * High-level purpose:
 * - This routine belongs to runtime_baremetal.c.
 * - It exists to provide the bare-metal runtime implementation for low-level machine targets without a host operating system.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine video stop".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void machine_video_stop(long long pid_value) { (void)pid_value; }
