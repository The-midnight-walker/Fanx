// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/*
 * diagnostics.h - Kernel & Userspace Diagnostic and Tracing Utilities
 *
 * (C) 2026 Author: Midnight Walker
 *
 * Based on and inspired by 'convenient.h' from the book:
 *   "Linux Kernel Programming - Part 2"
 *   (c) Kaiwan N Billimoria, Packt Publishing
 *   Repository:
 * https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2
 *
 * Modified, refactored, and adapted for ISO C compliance and project standards.
 */

#ifndef _DIAGNOSTICS_H
#define _DIAGNOSTICS_H

#include <asm/param.h>

#ifdef __KERNEL__
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/ratelimit.h>
#include <linux/sched.h>
#include <linux/version.h>
#else
#include <asm/bitsperlong.h>
#include <stdio.h>
#include <unistd.h>
#ifndef BITS_PER_LONG
#define BITS_PER_LONG __BITS_PER_LONG
#endif
#endif

#ifdef __KERNEL__

#ifndef prfx_fmt
#define prfx_fmt ""
#endif

#ifdef DEBUG
/* Debug builds: prefix + function name + line number, then the caller's format.
 */
#define pr_fmt(fmt) prfx_fmt ": %s:%d: " fmt, __func__, __LINE__
#else
/* Release builds: prefix only. */
#define pr_fmt(fmt) prfx_fmt ": " fmt
#endif

/* =============
 * DEBUG TRACING
 * =============
 * 1. Preferred approach: Use Kernel Dynamic Debug (pr_debug) when possible.
 * 2. High-performance trace: Enable Ftrace via Makefile (ccflags-y +=
 * -DUSE_FTRACE_BUFFER) and view logs at /sys/kernel/debug/tracing/trace.
 * 3. Default fallback: Uses rate-limited printk() to prevent dmesg flooding.
 */

/*
 * Debug output selection:
 * Define USE_FTRACE_BUFFER in your Makefile (ccflags-y += -DUSE_FTRACE_BUFFER)
 * to divert DBGPRINT() logs to the Ftrace ring buffer
 * (/sys/kernel/debug/tracing/trace). Otherwise, default rate-limited printk()
 * will be used (dmesg).
 */
#ifndef USE_FTRACE_BUFFER
#undef USE_FTRACE_BUFFER
#endif

#ifdef USE_FTRACE_BUFFER
#define DBGPRINT(string, args...) trace_printk(string, ##args)
#else
#define DBGPRINT(string, args...)                                              \
    do {                                                                       \
        int USE_RATELIMITING = 1;                                              \
        if (USE_RATELIMITING) {                                                \
            pr_info_ratelimited(string, ##args);                               \
        } else                                                                 \
            pr_info(string, ##args);                                           \
    } while (0)
#endif

#endif				/* __KERNEL__ */

/* =========================
 * TRACING MESSAGES PRINTING
 * =========================
 * QPRINT   : Quick Print
 * QPRINTDS : Quick Print Dump Stack
 */
#ifdef DEBUG
#ifdef __KERNEL__
#define PRINT(string, args...)                                                 \
    DBGPRINT("%s:%d : " string, __func__, __LINE__, ##args)
#else
#define PRINT(string, args...)                                                 \
    fprintf(stderr, "%s:%d : " string, __func__, __LINE__, ##args)
#endif

#ifdef __KERNEL__
#define PRINT_SHORT(string, args...) DBGPRINT(string, ##args)
#else
#define PRINT_SHORT(string, args...) fprintf(stderr, string, ##args)
#endif

#define QPRINT PRINT("\n")

#ifdef __KERNEL__
#ifndef USE_FTRACE_BUFFER
#define QPRINTDS                                                               \
    do {                                                                       \
        PRINT("\n");                                                           \
        dump_stack();                                                          \
    } while (0)
#else
#define QPRINTDS                                                               \
    do {                                                                       \
        PRINT("\n");                                                           \
        trace_dump_stack();                                                    \
    } while (0)
#endif
#endif

#ifdef __KERNEL__
#define HexDump(from_addr, len)                                                \
    print_hex_dump_bytes(" ", DUMP_PREFIX_ADDRESS, from_addr, len)
#endif
#else				/* #ifdef DEBUG */
#define PRINT(string, args...)
#define PRINT_SHORT(string, args...)
#define QPRINT
#define QPRINTDS
#define HexDump(from_addr, len)
#endif

/*
 * SHOW_DELTA_* Macros:
 * Formats a low value, high value, and their difference (delta = hi - low)
 * for printk/printf parameter expansion.
 *
 * Suffixes specify the unit scaling applied to the delta:
 *   _b  : Bytes (raw difference)
 *   _K  : KiB (shift >> 10)
 *   _M  : MiB (shift >> 20)
 *   _G  : GiB (shift >> 30)
 *   _MG / _MGT : Multi-unit display (MiB, GiB, TiB depending on arch)
 *
 * Inspired by Raspberry Pi kernel source (arch/arm/mm/init.c).
 */
#define SHOW_DELTA_b(low, hi) (low), (hi), ((hi) - (low))
#define SHOW_DELTA_K(low, hi) (low), (hi), (((hi) - (low)) >> 10)
#define SHOW_DELTA_M(low, hi) (low), (hi), (((hi) - (low)) >> 20)
#define SHOW_DELTA_G(low, hi) (low), (hi), (((hi) - (low)) >> 30)
#define SHOW_DELTA_MG(low, hi)                                                 \
    (low), (hi), (((hi) - (low)) >> 20), (((hi) - (low)) >> 30)

#if (BITS_PER_LONG == 64)
#define SHOW_DELTA_MGT(low, hi)                                                \
    (low), (hi), (((hi) - (low)) >> 20), (((hi) - (low)) >> 30),               \
        (((hi) - (low)) >> 40)
#else				// 32-bit
#define SHOW_DELTA_MGT(low, hi)                                                \
    (low), (hi), (((hi) - (low)) >> 20), (((hi) - (low)) >> 30)
#endif

#ifdef __KERNEL__

/*
 * PRINT_CTX() - Prints process context and kernel state (Ftrace style).
 *
 * It displays 4 columns of kernel state flags:
 *                       _-----=> irqs-off          [d]
 *                      / _----=> need-resched      [N]
 *                     | / _---=> hardirq/softirq   [H|h|s] [1]
 *                     || / _--=> preempt-depth     [#]
 *                     ||| /
 * CPU  TASK/PID       ||||  DURATION                  FUNCTION CALLS
 * |     |    |        ||||   |   |                     |   |   |   |
 *
 * [1] 'h' = hard IRQ running ; 'H' = hard IRQ inside a soft IRQ
 *
 * (Note: The 'DURATION' and 'FUNCTION CALLS' columns are not displayed
 * because this macro prints a snapshot, not a graph trace.)
 *
 * Sample output:
 *  CPU)  task_name:PID  | irqs,need-resched,hard/softirq,preempt-depth  \*
 * func_name() *\
 *  001)  rdwr_drv_secret-4857   |  ...0   \* read_miscdrv_rdwr() *\
 *
 * Usage:
 * Uses pr_debug(). Enable #define DEBUG or use Kernel Dynamic Debug to view
 * output.
 */

/*
 * Compatibility wrapper for hard IRQ checks.
 * Uses in_hardirq() on Linux 5.11+ and falls back to in_irq() on older kernels.
 * This keeps macro expansion clean without inline #ifdef directives.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
#define IN_HARDIRQ_CHECK() in_hardirq()
#else
#define IN_HARDIRQ_CHECK() in_irq()
#endif

#define PRINT_CTX()                                                            \
    do {                                                                       \
        int PRINTCTX_SHOWHDR = 0;                                              \
        char intr = '.';                                                       \
        if (!in_task()) {                                                      \
            if (IN_HARDIRQ_CHECK() && in_serving_softirq())                    \
                intr = 'H'; /* Hard IRQ occurred inside a soft IRQ */          \
            else if (IN_HARDIRQ_CHECK())                                       \
                intr = 'h'; /* Hard IRQ is running */                          \
            else if (in_serving_softirq())                                     \
                intr = 's'; /* Soft IRQ is running */                          \
        } else                                                                 \
            intr = '.';                                                        \
                                                                               \
        if (PRINTCTX_SHOWHDR == 1)                                             \
            pr_debug("CPU)  task_name:PID  | "                                 \
                     "irqs,need-resched,hard/softirq,preempt-depth  /* "       \
                     "func_name() */\n");                                      \
        pr_debug(                                                              \
            "%03d) %c%s%c:%d   |  "                                            \
            "%c%c%c%u   "                                                      \
            "/* %s() */\n",                                                    \
            raw_smp_processor_id(),                                            \
            (!current->mm ? '[' : ' '),                                        \
            current->comm,                                                     \
            (!current->mm ? ']' : ' '),                                        \
            current->pid,                                                      \
            (irqs_disabled() ? 'd' : '.'),                                     \
            (need_resched() ? 'N' : '.'),                                      \
            intr,                                                              \
            (preempt_count() & 0xff),                                          \
            __func__);                                                         \
    } while (0)

/*
 * Why raw_smp_processor_id() is used:
 * Standard smp_processor_id() triggers preemption warnings on kernels
 * configured with CONFIG_DEBUG_PREEMPT. Using raw_smp_processor_id() avoids
 * false alarms, matching Ftrace behavior.
 */

/* ==========
 * ASSERTIONS
 * ==========
 * Quoting Kaiwan N. Billimoria (Linux Kernel Programming):
 *
 * "Hey, careful!
 * Using assertions is great *but* be aware of traps & pitfalls:
 * http://blog.regehr.org/archives/1096
 *
 * The closest equivalent perhaps, to assert() in the kernel are the BUG(),
 * BUG_ON() and WARN(), WARN_ON() macros. Using BUG*() is _only_ for those
 * cases where recovery is impossible. WARN*() is usually considered a better
 * option. Pl see <asm-generic/bug.h> for details."
 */

// emit a noisy printk to "warn" the dev/user when the assertion failed
#define assert(expr)                                                           \
    do {                                                                       \
        if (!(expr)) {                                                         \
            pr_warn(                                                           \
                "********** Assertion [%s] failed! : %s:%s:%d **********\n",   \
                #expr,                                                         \
                __FILE__,                                                      \
                __func__,                                                      \
                __LINE__);                                                     \
        }                                                                      \
    } while (0)

#endif				/* __KERNEL__ */

/* ==========
 * DELAY LOOP
 * ==========
 */
#ifndef __KERNEL__
/* No cpu_relax() in userspace: plain compiler barrier */
#define cpu_relax() __asm__ __volatile__("" ::: "memory")
#endif

/*
 * beep() - Helper function to print a single character without newlines.
 * Uses pr_cont() in kernel space, write() to STDOUT in user space.
 */
static inline void beep(int what)
{
#ifdef __KERNEL__
	pr_cont("%c", (char)what);
#else
	char buf = (char)what;
	(void)write(STDOUT_FILENO, &buf, 1);
#endif
}

/*
 * DELAY_LOOP() - Emulates busy work by looping and printing progress.
 * @val        : ASCII character to print via beep()
 * @loop_count : Outer loop iteration count
 */
#define DELAY_LOOP(val, loop_count)                                            \
    do {                                                                       \
        int c = 0, m;                                                          \
        unsigned int for_index, inner_index;                                   \
        volatile unsigned int x;                                               \
                                                                               \
        for (for_index = 0; for_index < (loop_count); for_index++) {           \
            beep((val));                                                       \
            c++;                                                               \
            for (inner_index = 0; inner_index < HZ; inner_index++) {           \
                for (m = 0; m < 50; m++)                                       \
                    cpu_relax();                                               \
                x = inner_index / 2;                                           \
            }                                                                  \
        }                                                                      \
    } while (0)

/*------------------------------------------------------------------------*/

#ifdef __KERNEL__
/*
 * SHOW_DELTA() - Calculates and displays time elapsed between two timestamps.
 * @later   : Ending timestamp (ktime_t)
 * @earlier : Starting timestamp (ktime_t)
 *
 * Expected usage pattern:
 *   ktime_t t1 = ktime_get_real();
 *   ... work ...
 *   ktime_t t2 = ktime_get_real();
 *   SHOW_DELTA(t2, t1);
 *
 * Note on 32-bit architectures (e.g., ARM32 / BeagleBone Black):
 * 64-bit division on 32-bit targets causes compiler modpost errors
 * due to missing __aeabi_uldivmod symbol. On 32-bit platforms, this macro
 * emits a warning fallback instead of triggering build errors.
 */
#if (BITS_PER_LONG == 64)

#define SHOW_DELTA(later, earlier)                                             \
    do {                                                                       \
        if (time_after((unsigned long)(later), (unsigned long)(earlier))) {    \
            s64 delta_ns = ktime_to_ns(ktime_sub((later), (earlier)));         \
            pr_info("delta: %lld ns", delta_ns);                               \
            if (delta_ns / 1000 >= 1)                                          \
                pr_cont(" (~ %lld us", delta_ns / 1000);                       \
            if (delta_ns / 1000000 >= 1)                                       \
                pr_cont(" ~ %lld ms", delta_ns / 1000000);                     \
            if (delta_ns / 1000 >= 1)                                          \
                pr_cont(")\n");                                                \
        } else {                                                               \
            pr_warn("SHOW_DELTA(): invalid parameters (earlier > later?)\n");  \
        }                                                                      \
    } while (0)
#else				/* 32-bit architecture fallback */
#define SHOW_DELTA(later, earlier)                                             \
    pr_warn("SHOW_DELTA(): ktime_sub() not supported on 32-bit\n")
#endif

#endif				/* __KERNEL__ */

#endif				/* DIAGNOSTICS_H_ */
