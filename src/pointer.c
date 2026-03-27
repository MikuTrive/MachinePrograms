/*
 * Annotated reading copy of pointer.c
 *
 * What this file is for:
 * - Implement pointer-formatting helper routines such as hexadecimal and binary pointer-string conversion.
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

/*
 * pointer_annotated.c
 *
 * This file is an annotated version of pointer.c.
 *
 * Important note:
 * - The original code lines are preserved exactly as they appeared.
 * - Only explanatory English comments have been added.
 *
 * High-level purpose:
 * This module provides small helper routines for formatting raw pointer values
 * into printable strings. The Machine runtime uses these helpers when the
 * language wants to display pointer addresses in hexadecimal or binary form.
 *
 * Design overview:
 * 1. A pointer is first converted to uintptr_t.
 *    - uintptr_t is an integer type guaranteed to be able to hold a pointer.
 *    - This is the safest standard C way to perform bit-level formatting on
 *      an address value.
 *
 * 2. The integer value is then formatted into a textual representation.
 *    - Hexadecimal format: 0x....
 *    - Binary format:      0b....
 *
 * 3. The functions return a pointer to a static internal buffer.
 *    - This avoids heap allocation.
 *    - This keeps the formatting fast and simple.
 *    - Multiple rotating slots are used so several formatted pointer strings
 *      can exist at once in nearby calls without being overwritten immediately.
 *
 * Important limitation:
 * Because static internal buffers are used, these returned strings are not
 * thread-safe and should be treated as temporary formatting results.
 */

/*
 * Standard header for uintptr_t.
 *
 * uintptr_t is the core integer type used here because it can safely store a
 * converted pointer value without losing address bits on the current platform.
 */
#include <stdint.h>

/*
 * machine_pointer_format_hex_impl
 *
 * Internal helper that converts a raw pointer-sized integer into a hexadecimal
 * string such as:
 *
 *   0x00007ffdeadc0de0
 *
 * Why this function takes uintptr_t instead of void *:
 * - Formatting logic works by shifting and masking bits.
 * - Bitwise operations are naturally defined on integer types.
 * - The public wrapper later converts from void * to uintptr_t before calling
 *   this helper.
 *
 * Return value:
 * - A pointer to one of the module's static formatting buffers.
 * - The caller must not free the returned string.
 * - The contents may be overwritten by later formatting calls after enough
 *   calls rotate through the available slots.
 */
static char *machine_pointer_format_hex_impl(uintptr_t value)
{
    /*
     * A ring of reusable static buffers.
     *
     * Why 16 buffers:
     * - This allows several formatting calls to coexist briefly.
     * - Example: if code prints multiple formatted pointers in one expression,
     *   a single static buffer would be overwritten too early.
     *
     * Buffer length calculation:
     * - 2 characters for the prefix: "0x"
     * - sizeof(uintptr_t) * 2 hexadecimal digits
     *   Each hex digit represents 4 bits.
     *   A byte has 8 bits, so each byte needs 2 hex digits.
     * - 1 character for the null terminator
     */
    static char buffers[16][2 + (int)(sizeof(uintptr_t) * 2) + 1];

    /*
     * Rotating slot counter used to choose which static buffer to return.
     *
     * This is incremented on every call and wrapped using modulo arithmetic.
     */
    static unsigned int slot = 0;

    /*
     * Lookup table for hexadecimal digits.
     *
     * The function uses lowercase hex output:
     *   0 1 2 3 4 5 6 7 8 9 a b c d e f
     */
    static const char digits[] = "0123456789abcdef";

    /*
     * Select the current output buffer and advance the rotating slot index.
     *
     * slot++ % 16 means:
     * - use the current slot value
     * - then increment slot
     * - wrap the chosen index into the range [0, 15]
     */
    char *buf = buffers[slot++ % 16];

    /*
     * Number of hex digits needed for a full pointer-sized value.
     *
     * Example:
     * - 64-bit platform: sizeof(uintptr_t) == 8, so nibbles == 16
     * - 32-bit platform: sizeof(uintptr_t) == 4, so nibbles == 8
     *
     * "nibble" means 4 bits, which is exactly one hexadecimal digit.
     */
    size_t nibbles = sizeof(uintptr_t) * 2;

    /* Write the standard hexadecimal prefix. */
    buf[0] = '0';
    buf[1] = 'x';

    /*
     * Emit the hex digits from most significant nibble to least significant
     * nibble, so the final string appears in normal left-to-right reading
     * order.
     */
    for (size_t i = 0; i < nibbles; ++i)
    {
        /*
         * Compute how many bits we need to shift right so that the current
         * target nibble lands in the low 4 bits.
         *
         * Example on a 64-bit platform:
         * - First iteration (i == 0): shift = 60
         * - Last iteration:           shift = 0
         */
        size_t shift = (nibbles - 1 - i) * 4;

        /*
         * Extract one nibble and map it to a printable hex digit.
         *
         * Steps:
         * 1. value >> shift  -> move the desired nibble down
         * 2. & 0xF           -> isolate only the low 4 bits
         * 3. digits[...]     -> convert numeric nibble to ASCII character
         */
        buf[2 + i] = digits[(value >> shift) & (uintptr_t)0xF];
    }

    /* Terminate the string in standard C style. */
    buf[2 + nibbles] = '\0';

    /* Return the selected formatted hexadecimal string. */
    return buf;
}

/*
 * machine_pointer_format_bin_impl
 *
 * Internal helper that converts a raw pointer-sized integer into a binary
 * string such as:
 *
 *   0b0000000000000000010101100010111011101000001010010010001010110000
 *
 * This is mainly useful for debugging, education, or low-level inspection,
 * since binary output makes the individual address bits visible.
 *
 * Return value:
 * - A pointer to one of the module's static formatting buffers.
 * - The caller must not free it.
 * - The data is temporary and may be overwritten by later calls.
 */
static char *machine_pointer_format_bin_impl(uintptr_t value)
{
    /*
     * Ring of reusable static buffers for binary output.
     *
     * Buffer length calculation:
     * - 2 characters for prefix: "0b"
     * - sizeof(uintptr_t) * 8 binary digits
     *   Each byte has 8 bits, so a full pointer value needs one character per
     *   bit.
     * - 1 character for the null terminator
     */
    static char buffers[16][2 + (int)(sizeof(uintptr_t) * 8) + 1];

    /* Rotating slot index, same design as in the hex formatter above. */
    static unsigned int slot = 0;

    /* Select the next reusable output buffer. */
    char *buf = buffers[slot++ % 16];

    /* Number of bits in a pointer-sized integer on this platform. */
    size_t bits = sizeof(uintptr_t) * 8;

    /* Write the standard binary prefix. */
    buf[0] = '0';
    buf[1] = 'b';

    /*
     * Emit bits from most significant to least significant so the final text
     * reads naturally from left to right.
     */
    for (size_t i = 0; i < bits; ++i)
    {
        /*
         * On the first character we inspect the highest bit.
         * On the last character we inspect bit 0.
         */
        size_t shift = bits - 1 - i;

        /*
         * Extract one bit and write either '1' or '0'.
         *
         * Steps:
         * 1. value >> shift  -> move target bit to the low position
         * 2. & 1             -> isolate that single bit
         * 3. ternary         -> choose ASCII '1' or '0'
         */
        buf[2 + i] = ((value >> shift) & (uintptr_t)1) ? '1' : '0';
    }

    /* Null-terminate the binary string. */
    buf[2 + bits] = '\0';

    /* Return the selected formatted binary string. */
    return buf;
}

/*
 * machine_ptr_hex
 *
 * Public wrapper used by the Machine runtime and code generation layers.
 *
 * Input:
 * - p: any pointer value to be formatted.
 *
 * Operation:
 * - Cast the pointer to uintptr_t.
 * - Forward the integer value to the internal hexadecimal formatter.
 *
 * Why a wrapper exists:
 * - Keeps the public interface easy to use from other runtime code.
 * - Separates pointer-to-integer conversion from the low-level formatting
 *   routine.
 */
char *machine_ptr_hex(void *p)
{
    return machine_pointer_format_hex_impl((uintptr_t)p);
}

/*
 * machine_ptr_bin
 *
 * Public wrapper used by the Machine runtime and code generation layers.
 *
 * Input:
 * - p: any pointer value to be formatted.
 *
 * Operation:
 * - Cast the pointer to uintptr_t.
 * - Forward the integer value to the internal binary formatter.
 *
 * This function is the binary counterpart of machine_ptr_hex.
 */
char *machine_ptr_bin(void *p)
{
    return machine_pointer_format_bin_impl((uintptr_t)p);
}
