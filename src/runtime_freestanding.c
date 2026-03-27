/*
 * Annotated reading copy of runtime_freestanding.c
 *
 * What this file is for:
 * - Provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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

#include "machine_runtime_freestanding.h"

#include "pointer.c"

#define MACHINE_SYS_READ 0
#define MACHINE_SYS_WRITE 1
#define MACHINE_SYS_OPEN 2
#define MACHINE_SYS_CLOSE 3
#define MACHINE_SYS_LSEEK 8
#define MACHINE_SYS_MMAP 9
#define MACHINE_SYS_MUNMAP 11
#define MACHINE_SYS_IOCTL 16
#define MACHINE_SYS_NANOSLEEP 35
#define MACHINE_SYS_EXIT 60
#define MACHINE_SYS_CLOCK_GETTIME 228

#define MACHINE_PROT_READ 0x1
#define MACHINE_PROT_WRITE 0x2
#define MACHINE_PROT_EXEC 0x4
#define MACHINE_MAP_PRIVATE 0x02
#define MACHINE_MAP_ANONYMOUS 0x20

#define MACHINE_O_RDONLY 0
#define MACHINE_O_WRONLY 1
#define MACHINE_O_RDWR 2
#define MACHINE_O_CREAT 0100
#define MACHINE_O_TRUNC 01000

struct MachineTimespec
{
    long tv_sec;
    long tv_nsec;
};

typedef struct MachineAllocHeader
{
    long long mapped_size;
} MachineAllocHeader;

static long long machine_last_key = 0;
static long long machine_mouse_x = 0;
static long long machine_mouse_y = 0;
static long long machine_mouse_button = 0;
static struct MachineTimespec machine_timer_origin = {0, 0};

/*
 * Function overview: machine_internal_syscall0
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall0".
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
static long long machine_internal_syscall0(long long n)
{
    long long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall1
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall1".
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
static long long machine_internal_syscall1(long long n, long long a1)
{
    long long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall2
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall2".
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
static long long machine_internal_syscall2(long long n, long long a1, long long a2)
{
    long long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall3
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall3".
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
static long long machine_internal_syscall3(long long n, long long a1, long long a2, long long a3)
{
    long long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall4
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall4".
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
static long long machine_internal_syscall4(long long n, long long a1, long long a2, long long a3, long long a4)
{
    long long ret;
    register long long r10 __asm__("r10") = a4;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall5
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall5".
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
static long long machine_internal_syscall5(long long n, long long a1, long long a2, long long a3, long long a4, long long a5)
{
    long long ret;
    register long long r10 __asm__("r10") = a4;
    register long long r8 __asm__("r8") = a5;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
}
/*
 * Function overview: machine_internal_syscall6
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine internal syscall6".
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
static long long machine_internal_syscall6(long long n, long long a1, long long a2, long long a3, long long a4, long long a5, long long a6)
{
    long long ret;
    register long long r10 __asm__("r10") = a4;
    register long long r8 __asm__("r8") = a5;
    register long long r9 __asm__("r9") = a6;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

/*
 * Function overview: machine_syscall0
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall0(long long n) { return machine_internal_syscall0(n); }
/*
 * Function overview: machine_syscall1
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall1(long long n, long long a1) { return machine_internal_syscall1(n, a1); }
/*
 * Function overview: machine_syscall2
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall2(long long n, long long a1, long long a2) { return machine_internal_syscall2(n, a1, a2); }
/*
 * Function overview: machine_syscall3
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall3(long long n, long long a1, long long a2, long long a3) { return machine_internal_syscall3(n, a1, a2, a3); }
/*
 * Function overview: machine_syscall4
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall4(long long n, long long a1, long long a2, long long a3, long long a4) { return machine_internal_syscall4(n, a1, a2, a3, a4); }
/*
 * Function overview: machine_syscall5
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall5(long long n, long long a1, long long a2, long long a3, long long a4, long long a5) { return machine_internal_syscall5(n, a1, a2, a3, a4, a5); }
/*
 * Function overview: machine_syscall6
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_syscall6(long long n, long long a1, long long a2, long long a3, long long a4, long long a5, long long a6) { return machine_internal_syscall6(n, a1, a2, a3, a4, a5, a6); }

/*
 * Function overview: machine_strlen_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * Function overview: machine_write_all
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine write all".
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
static void machine_write_all(const char *s, long long n)
{
    while (n > 0)
    {
        long long wrote = machine_internal_syscall3(MACHINE_SYS_WRITE, 1, (long long)(intptr_t)s, n);
        if (wrote <= 0)
            return;
        s += wrote;
        n -= wrote;
    }
}
/*
 * Function overview: machine_exit_now
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine exit now".
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
static void machine_exit_now(long long code)
{
    machine_internal_syscall1(MACHINE_SYS_EXIT, code);
    for (;;)
    {
    }
}
/*
 * Function overview: machine_panic
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine panic".
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
static void machine_panic(const char *msg)
{
    static const char prefix[] = "machine freestanding runtime error: ";
    machine_write_all(prefix, (long long)(sizeof(prefix) - 1));
    machine_write_all(msg ? msg : "(null)", machine_strlen_raw(msg ? msg : "(null)"));
    machine_write_all("\n", 1);
    machine_exit_now(1);
}

/*
 * Function overview: machine_print_str
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    machine_write_all(s, machine_strlen_raw(s));
    machine_write_all("\n", 1);
}
/*
 * Function overview: machine_print_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    char buf[64];
    long long n = value;
    int neg = 0;
    int i = 0;
    if (n == 0)
    {
        machine_write_all("0\n", 2);
        return;
    }
    if (n < 0)
    {
        neg = 1;
        n = -n;
    }
    while (n > 0 && i < (int)sizeof(buf) - 1)
    {
        buf[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    if (neg)
        buf[i++] = '-';
    for (int j = 0; j < i / 2; ++j)
    {
        char t = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = t;
    }
    buf[i++] = '\n';
    machine_write_all(buf, i);
}
/*
 * Function overview: machine_print_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    machine_print_str("[f64 print unsupported in freestanding target]");
}
/*
 * Function overview: machine_print_hp
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    machine_print_str("[hp print unsupported in freestanding target]");
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    static char slots[16][2];
    static int slot = 0;
    long long n = machine_strlen_raw(s);
    if (i < 0 || i >= n)
        machine_panic("string index out of range");
    slot = (slot + 1) & 15;
    slots[slot][0] = s[i];
    slots[slot][1] = '\0';
    return slots[slot];
}
/*
 * Function overview: machine_hp_from_text
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    long double sign = 1.0L;
    long double value = 0.0L;
    long double frac = 0.1L;
    if (!s)
        return 0.0L;
    if (*s == '-')
    {
        sign = -1.0L;
        ++s;
    }
    while (*s >= '0' && *s <= '9')
    {
        value = value * 10.0L + (long double)(*s - '0');
        ++s;
    }
    if (*s == '.')
    {
        ++s;
        while (*s >= '0' && *s <= '9')
        {
            value += (long double)(*s - '0') * frac;
            frac *= 0.1L;
            ++s;
        }
    }
    return sign * value;
}
/*
 * Function overview: machine_hp_add
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long double machine_hp_div(long double a, long double b) { return b == 0.0L ? 0.0L : a / b; }
/*
 * Function overview: machine_hp_sqrt
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long double machine_hp_sqrt(long double a)
{
    if (a <= 0.0L)
        return 0.0L;
    long double x = a > 1.0L ? a : 1.0L;
    for (int i = 0; i < 32; ++i)
        x = (x + a / x) * 0.5L;
    return x;
}
/*
 * Function overview: machine_hp_pow
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    long long n = (long long)b;
    long double r = 1.0L;
    if ((long double)n != b)
        return 0.0L;
    if (n < 0)
    {
        if (a == 0.0L)
            return 0.0L;
        a = 1.0L / a;
        n = -n;
    }
    while (n-- > 0)
        r *= a;
    return r;
}

void *machine_ptr_from_i64(long long value) { return (void *)(intptr_t)value; }
/*
 * Function overview: machine_ptr_to_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_ptr_to_i64(void *p) { return (long long)(intptr_t)p; }
void *machine_ptr_offset(void *p, long long offset) { return (void *)((unsigned char *)p + offset); }
/*
 * Function overview: machine_store_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_i64(void *p, long long v) { *((long long *)p) = v; }
/*
 * Function overview: machine_load_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_load_i64(void *p) { return *((long long *)p); }
/*
 * Function overview: machine_store_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_f64(void *p, double v) { *((double *)p) = v; }
/*
 * Function overview: machine_load_f64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
double machine_load_f64(void *p) { return *((double *)p); }
/*
 * Function overview: machine_store_str
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_str(void *p, char *v) { *((char **)p) = v; }
char *machine_load_str(void *p) { return *((char **)p); }
/*
 * Function overview: machine_store_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_u8(void *p, long long v) { *((uint8_t *)p) = (uint8_t)v; }
/*
 * Function overview: machine_store_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_u16(void *p, long long v) { *((uint16_t *)p) = (uint16_t)v; }
/*
 * Function overview: machine_store_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_u32(void *p, long long v) { *((uint32_t *)p) = (uint32_t)v; }
/*
 * Function overview: machine_store_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_store_u64(void *p, long long v) { *((uint64_t *)p) = (uint64_t)v; }
/*
 * Function overview: machine_load_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_load_u8(void *p) { return *((uint8_t *)p); }
/*
 * Function overview: machine_load_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_load_u16(void *p) { return *((uint16_t *)p); }
/*
 * Function overview: machine_load_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_load_u32(void *p) { return *((uint32_t *)p); }
/*
 * Function overview: machine_load_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_load_u64(void *p) { return (long long)(*((uint64_t *)p)); }
/*
 * Function overview: machine_volatile_store_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_volatile_store_u8(void *p, long long v) { *((volatile uint8_t *)p) = (uint8_t)v; }
/*
 * Function overview: machine_volatile_store_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_volatile_store_u16(void *p, long long v) { *((volatile uint16_t *)p) = (uint16_t)v; }
/*
 * Function overview: machine_volatile_store_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_volatile_store_u32(void *p, long long v) { *((volatile uint32_t *)p) = (uint32_t)v; }
/*
 * Function overview: machine_volatile_store_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_volatile_store_u64(void *p, long long v) { *((volatile uint64_t *)p) = (uint64_t)v; }
/*
 * Function overview: machine_volatile_load_u8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_volatile_load_u8(void *p) { return *((volatile uint8_t *)p); }
/*
 * Function overview: machine_volatile_load_u16
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_volatile_load_u16(void *p) { return *((volatile uint16_t *)p); }
/*
 * Function overview: machine_volatile_load_u32
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_volatile_load_u32(void *p) { return *((volatile uint32_t *)p); }
/*
 * Function overview: machine_volatile_load_u64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_volatile_load_u64(void *p) { return (long long)(*((volatile uint64_t *)p)); }

static void *machine_mmap_flags(long long size, long long prot)
{
    if (size <= 0)
        return NULL;
    long long result = machine_internal_syscall6(MACHINE_SYS_MMAP, 0, size, prot, MACHINE_MAP_PRIVATE | MACHINE_MAP_ANONYMOUS, -1, 0);
    if (result < 0)
        return NULL;
    return (void *)(intptr_t)result;
}
void *machine_mmap_anon(long long size) { return machine_mmap_flags(size, MACHINE_PROT_READ | MACHINE_PROT_WRITE); }
void *machine_mmap_anon_exec(long long size) { return machine_mmap_flags(size, MACHINE_PROT_READ | MACHINE_PROT_WRITE | MACHINE_PROT_EXEC); }
/*
 * Function overview: machine_munmap_mem
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_munmap_mem(void *p, long long size) { return machine_internal_syscall2(MACHINE_SYS_MUNMAP, (long long)(intptr_t)p, size); }

void *machine_alloc_bytes(long long n)
{
    long long total = (long long)sizeof(MachineAllocHeader) + n;
    MachineAllocHeader *h = (MachineAllocHeader *)machine_mmap_anon(total);
    if (!h)
        machine_panic("mmap alloc failed");
    h->mapped_size = total;
    return (void *)(h + 1);
}
/*
 * Function overview: machine_free_mem
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_free_mem(void *p)
{
    if (!p)
        return;
    MachineAllocHeader *h = ((MachineAllocHeader *)p) - 1;
    machine_munmap_mem(h, h->mapped_size);
}

/*
 * Function overview: machine_fd_open_ro
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_open_ro(const char *path) { return machine_internal_syscall3(MACHINE_SYS_OPEN, (long long)(intptr_t)path, MACHINE_O_RDONLY, 0); }
/*
 * Function overview: machine_fd_open_wo
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_open_wo(const char *path) { return machine_internal_syscall3(MACHINE_SYS_OPEN, (long long)(intptr_t)path, MACHINE_O_WRONLY | MACHINE_O_CREAT | MACHINE_O_TRUNC, 0644); }
/*
 * Function overview: machine_fd_open_rw
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_open_rw(const char *path) { return machine_internal_syscall3(MACHINE_SYS_OPEN, (long long)(intptr_t)path, MACHINE_O_RDWR | MACHINE_O_CREAT, 0644); }
/*
 * Function overview: machine_fd_close
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_close(long long fd) { return machine_internal_syscall1(MACHINE_SYS_CLOSE, fd); }
/*
 * Function overview: machine_fd_read
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_read(long long fd, void *buf, long long size) { return machine_internal_syscall3(MACHINE_SYS_READ, fd, (long long)(intptr_t)buf, size); }
/*
 * Function overview: machine_fd_write
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_write(long long fd, void *buf, long long size) { return machine_internal_syscall3(MACHINE_SYS_WRITE, fd, (long long)(intptr_t)buf, size); }
/*
 * Function overview: machine_fd_seek
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_fd_seek(long long fd, long long offset, long long whence) { return machine_internal_syscall3(MACHINE_SYS_LSEEK, fd, offset, whence); }
/*
 * Function overview: machine_ioctl_i64
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_ioctl_i64(long long fd, long long request, long long arg) { return machine_internal_syscall3(MACHINE_SYS_IOCTL, fd, request, arg); }

/*
 * Function overview: machine_asm_nop
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    unsigned int lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((long long)hi << 32) | (long long)lo;
}
/*
 * Function overview: machine_cpu_in8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"((unsigned short)port));
    return (long long)value;
}
/*
 * Function overview: machine_cpu_out8
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_cpu_out8(long long port, long long value)
{
    __asm__ volatile("outb %0, %1" : : "a"((unsigned char)value), "Nd"((unsigned short)port));
}

MachineList *machine_list_new(void)
{
    MachineList *list = (MachineList *)machine_alloc_bytes((long long)sizeof(MachineList));
    machine_memset_raw(list, 0, (long long)sizeof(MachineList));
    return list;
}
/*
 * Function overview: machine_list_push_back
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    MachineListNode *node;
    if (!list)
        machine_panic("list is null");
    node = (MachineListNode *)machine_alloc_bytes((long long)sizeof(MachineListNode));
    node->value = value;
    node->next = NULL;
    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
    list->size += 1;
}
/*
 * Function overview: machine_list_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    MachineListNode *node;
    if (!list || index < 0 || index >= list->size)
        machine_panic("list index out of range");
    node = list->head;
    while (index-- > 0)
        node = node->next;
    return node->value;
}
/*
 * Function overview: machine_list_size
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_list_free(MachineList *list)
{
    if (!list)
        return;
    MachineListNode *node = list->head;
    while (node)
    {
        MachineListNode *next = node->next;
        machine_free_mem(node);
        node = next;
    }
    machine_free_mem(list);
}

MachineArray *machine_array_new(void)
{
    MachineArray *a = (MachineArray *)machine_alloc_bytes((long long)sizeof(MachineArray));
    machine_memset_raw(a, 0, (long long)sizeof(MachineArray));
    return a;
}
/*
 * Function overview: machine_array_reserve
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine array reserve".
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
static void machine_array_reserve(MachineArray *a, long long need)
{
    long long cap = a->capacity ? a->capacity : 4;
    while (cap < need)
        cap *= 2;
    if (cap == a->capacity)
        return;
    long long *new_data = (long long *)machine_alloc_bytes(cap * (long long)sizeof(long long));
    if (a->data)
        machine_memcpy_raw(new_data, a->data, a->size * (long long)sizeof(long long));
    if (a->data)
        machine_free_mem(a->data);
    a->data = new_data;
    a->capacity = cap;
}
/*
 * Function overview: machine_array_push
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
        machine_panic("array is null");
    machine_array_reserve(a, a->size + 1);
    a->data[a->size++] = value;
}
/*
 * Function overview: machine_array_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_array_get(MachineArray *a, long long index)
{
    if (!a || index < 0 || index >= a->size)
        machine_panic("array index out of range");
    return a->data[index];
}
/*
 * Function overview: machine_array_set
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    if (!a || index < 0 || index >= a->size)
        machine_panic("array index out of range");
    a->data[index] = value;
}
/*
 * Function overview: machine_array_len
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_array_free(MachineArray *a)
{
    if (!a)
        return;
    if (a->data)
        machine_free_mem(a->data);
    machine_free_mem(a);
}

MachineGrid *machine_grid_new(long long rows, long long cols)
{
    MachineGrid *g;
    if (rows <= 0 || cols <= 0)
        machine_panic("grid dimensions must be positive");
    g = (MachineGrid *)machine_alloc_bytes((long long)sizeof(MachineGrid));
    g->rows = rows;
    g->cols = cols;
    g->data = (long long *)machine_alloc_bytes(rows * cols * (long long)sizeof(long long));
    machine_memset_raw(g->data, 0, rows * cols * (long long)sizeof(long long));
    return g;
}
/*
 * Function overview: machine_grid_index
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "machine grid index".
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
static long long machine_grid_index(MachineGrid *g, long long row, long long col)
{
    if (!g || row < 0 || col < 0 || row >= g->rows || col >= g->cols)
        machine_panic("grid index out of range");
    return row * g->cols + col;
}
/*
 * Function overview: machine_grid_get
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_grid_get(MachineGrid *g, long long row, long long col) { return g->data[machine_grid_index(g, row, col)]; }
/*
 * Function overview: machine_grid_set
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_grid_set(MachineGrid *g, long long row, long long col, long long value) { g->data[machine_grid_index(g, row, col)] = value; }
/*
 * Function overview: machine_grid_rows
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    long long n;
    if (!g)
        return;
    n = g->rows * g->cols;
    for (long long i = 0; i < n; ++i)
        g->data[i] = value;
}
/*
 * Function overview: machine_grid_free
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_grid_free(MachineGrid *g)
{
    if (!g)
        return;
    machine_free_mem(g->data);
    machine_free_mem(g);
}

/*
 * Function overview: machine_tick_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_tick_ms(void)
{
    struct MachineTimespec now;
    if (machine_internal_syscall2(MACHINE_SYS_CLOCK_GETTIME, 1, (long long)(intptr_t)&now) < 0)
        return 0;
    return now.tv_sec * 1000LL + now.tv_nsec / 1000000LL;
}
/*
 * Function overview: machine_timer_reset
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_timer_reset(void)
{
    if (machine_internal_syscall2(MACHINE_SYS_CLOCK_GETTIME, 1, (long long)(intptr_t)&machine_timer_origin) < 0)
    {
        machine_timer_origin.tv_sec = 0;
        machine_timer_origin.tv_nsec = 0;
    }
}
/*
 * Function overview: machine_timer_elapsed_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_timer_elapsed_ms(void)
{
    struct MachineTimespec now;
    if (machine_internal_syscall2(MACHINE_SYS_CLOCK_GETTIME, 1, (long long)(intptr_t)&now) < 0)
        return 0;
    return (now.tv_sec - machine_timer_origin.tv_sec) * 1000LL + (now.tv_nsec - machine_timer_origin.tv_nsec) / 1000000LL;
}
/*
 * Function overview: machine_sleep_ms
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    struct MachineTimespec req;
    if (ms <= 0)
        return;
    req.tv_sec = ms / 1000LL;
    req.tv_nsec = (ms % 1000LL) * 1000000LL;
    machine_internal_syscall2(MACHINE_SYS_NANOSLEEP, (long long)(intptr_t)&req, 0);
}

/*
 * Function overview: machine_term_enable_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_term_enable_raw(void) {}
/*
 * Function overview: machine_term_disable_raw
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_term_disable_raw(void) {}
/*
 * Function overview: machine_term_key_available
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
int machine_term_key_available(void) { return 0; }
/*
 * Function overview: machine_term_read_key
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_term_read_key(void)
{
    unsigned char ch = 0;
    long long r = machine_internal_syscall3(MACHINE_SYS_READ, 0, (long long)(intptr_t)&ch, 1);
    if (r == 1)
    {
        machine_last_key = (long long)ch;
        return machine_last_key;
    }
    return -1;
}
/*
 * Function overview: machine_term_enable_mouse
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_term_poll_event(void) { return MACHINE_EVENT_NONE; }
/*
 * Function overview: machine_term_clear
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_term_clear(void) { machine_write_all("\033[2J\033[H", 7); }
/*
 * Function overview: machine_term_flush
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    (void)x;
    (void)y;
}
/*
 * Function overview: machine_term_hide_cursor
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    (void)x;
    (void)y;
    machine_write_all(text ? text : "", machine_strlen_raw(text ? text : ""));
}

/*
 * Function overview: machine_win_create
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_win_last_key(void) { return 0; }
/*
 * Function overview: machine_win_mouse_x
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_win_mouse_x(void) { return 0; }
/*
 * Function overview: machine_win_mouse_y
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_win_mouse_y(void) { return 0; }
/*
 * Function overview: machine_win_mouse_button
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_win_mouse_button(void) { return 0; }
/*
 * Function overview: machine_win_poll_event
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_win_poll_event(void) { return MACHINE_EVENT_NONE; }
/*
 * Function overview: machine_win_set_title
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_win_draw_text(long long x, long long y, const char *text)
{
    (void)x;
    (void)y;
    (void)text;
}
void *machine_image_load(const char *path)
{
    (void)path;
    return 0;
}
/*
 * Function overview: machine_image_width
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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

void *machine_pmm_alloc_page(void) { return machine_alloc_bytes(4096); }
void *machine_pmm_alloc_pages(long long count)
{
    if (count <= 0)
        return NULL;
    return machine_alloc_bytes(count * 4096);
}
/*
 * Function overview: machine_pmm_total_bytes
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_pmm_total_bytes(void) { return 0; }
/*
 * Function overview: machine_pmm_used_bytes
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
long long machine_pmm_used_bytes(void) { return 0; }
/*
 * Function overview: machine_page_identity_map_2m
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
    (void)page_index;
    (void)phys_base;
    (void)flags;
    return 0;
}
/*
 * Function overview: machine_apic_supported
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
int machine_apic_supported(void) { return 0; }
/*
 * Function overview: machine_apic_enable
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
int machine_apic_enable(void) { return 0; }
/*
 * Function overview: machine_apic_eoi
 *
 * High-level purpose:
 * - This routine belongs to runtime_freestanding.c.
 * - It exists to provide the freestanding runtime implementation for syscall-oriented targets without libc startup.
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
void machine_apic_eoi(void) {}
