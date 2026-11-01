// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#define prfx_fmt FANX_DRIVER_NAME

#include <linux/fs.h>
#include <linux/version.h>

#include "diagnostics.h"
#include "kfanx.h"

MODULE_AUTHOR("The-midnight-walker");
MODULE_DESCRIPTION(
    "Linux character device driver that controls and monitors a system fan");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("DRIVER_VERSION");

static struct fanx_prvctx_struct *fanx_prvctx;

static int fanx_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int fanx_release(struct inode *inode, struct file *file)
{
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long fanx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int
fanx_ioctl(struct *ino, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    return 0;
}

static const struct file_operations fanx_fops = {
    .owner = THIS_MODULE,
    .open = fanx_open,
    .release = fanx_release,
    .llseek = noop_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
    .unlocked_ioctl = fanx_ioctl,
#else
    .ioctl = fanx_ioctl,
#endif
};

static int __init fanx_init(void)
{
    int ret;
    dev_t dev;
    struct device *devp;

    fanx_prvctx = kzalloc(sizeof(*fanx_prvctx), GFP_KERNEL);
    if (!fanx_prvctx)
        return -ENOMEM;

    spin_lock_init(&fanx_prvctx->splock);
    mutex_init(&fanx_prvctx->mtx);

    ret = alloc_chrdev_region(
        &dev, FANX_CDEV_BASEMINOR, FANX_CDEV_MINOR_NR, FANX_DEVICE_NAME);
    if (ret < 0) {
        pr_alert("registering char device failed with %d\n", ret);
        goto free_mm;
    }

    cdev_init(&fanx_prvctx->c_dev, &fanx_fops);
    fanx_prvctx->c_dev.owner = THIS_MODULE;

    ret = cdev_add(&fanx_prvctx->c_dev, dev, FANX_CDEV_MINOR_NR);
    if (ret < 0) {
        pr_alert("registering char device failed with %d\n", ret);
        goto unreg_region;
    }

    pr_info("registering char device and assigned major=%d", MAJOR(dev));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    fanx_prvctx->dev_class = class_create(DEVICE_NAME);
#else
    fanx_prvctx->dev_class = class_create(THIS_MODULE, FANX_DEVICE_NAME);
#endif
    if (IS_ERR(fanx_prvctx->dev_class)) {
        ret = PTR_ERR(fanx_prvctx->dev_class);
        goto del_cdev;
    }

    devp = fanx_prvctx->dev = device_create(
        fanx_prvctx->dev_class, NULL, dev, NULL, FANX_DEVICE_NAME);
    if (IS_ERR(devp)) {
        ret = PTR_ERR(devp);
        goto destroy_class;
    }

    pr_info("device created on /dev/%s\n", FANX_DEVICE_NAME);
    return 0;

destroy_class:
    class_destroy(fanx_prvctx->dev_class);
del_cdev:
    cdev_del(&fanx_prvctx->c_dev);
unreg_region:
    unregister_chrdev_region(dev, FANX_CDEV_MINOR_NR);
free_mm:
    kfree(fanx_prvctx);
    fanx_prvctx = NULL;
    return ret;
}

static void __exit fanx_exit(void)
{
    dev_t dev = fanx_prvctx->c_dev.dev;

    device_destroy(fanx_prvctx->dev_class, dev);
    class_destroy(fanx_prvctx->dev_class);
    cdev_del(&fanx_prvctx->c_dev);
    unregister_chrdev_region(dev, FANX_CDEV_MINOR_NR);
    kfree(fanx_prvctx);
    pr_info("unloaded\n");
}

module_init(fanx_init);
module_exit(fanx_exit);
