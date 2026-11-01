// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef _KERNEL_KFANX_H
#define _KERNEL_KFANX_H

#include <linux/cdev.h>

#include "fanx.h"

/* Dynamically allocated our major number by the kernel
so don't need to define the major number*/
#define FANX_CDEV_BASEMINOR 0
#define FANX_CDEV_MINOR_NR 2

struct fanx_prvctx_struct {
    /* Cacheline 0 */
    spinlock_t splock;
    struct timer_list tmr;
    ktime_t ktime_init;

    /* Cacheline 1 */
    struct mutex mtx ____cacheline_aligned;

    /* Cacheline 2 and 3 */
    struct fanx_ds kcfg;

    /* Cacheline 4 */
    struct class *dev_class ____cacheline_aligned;
    struct cdev c_dev;
    struct device *dev;
};

#endif /* _KERNEL_KFANX_H */
