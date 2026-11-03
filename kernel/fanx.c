// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#include <linux/fs.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "kfanx.h"

#define prfx_fmt FANX_DRIVER_NAME
#include "diagnostics.h"

MODULE_AUTHOR("The-midnight-walker");
MODULE_DESCRIPTION(
    "Linux character device driver that controls and monitors a system fan");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(FANX_DRIVER_VERSION);

static struct fanx_prvctx_struct *fanx_prvctx;

static int fanx_open(struct inode *inode, struct file *file)
{
    struct fanx_prvctx_struct *ctx =
        container_of(inode->i_cdev, struct fanx_prvctx_struct, c_dev);

    file->private_data = ctx;
    return 0;
}

static int fanx_release(struct inode *inode, struct file *file)
{
    file->private_data = NULL;
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long fanx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int
fanx_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    struct fanx_prvctx_struct *ctx = filp->private_data;

    if (!ctx)
        return -EINVAL;

    if (_IOC_TYPE(cmd) != FANX_IOC_MAGIC || _IOC_NR(cmd) > FANX_IOC_MAXNR)
        return -ENOTTY;

    switch (cmd) {
    case FANX_IOC_RESET:
        if (mutex_lock_interruptible(&ctx->mtx))
            return -ERESTARTSYS;
        memset(&ctx->kcfg, 0, sizeof(ctx->kcfg));
        mutex_unlock(&ctx->mtx);
        return 0;

    case FANX_IOC_GET_CONFIG:
        if (mutex_lock_interruptible(&ctx->mtx))
            return -ERESTARTSYS;
        if (copy_to_user((void __user *)arg, &ctx->kcfg, sizeof(ctx->kcfg))) {
            mutex_unlock(&ctx->mtx);
            return -EFAULT;
        }
        mutex_unlock(&ctx->mtx);
        return 0;

    case FANX_IOC_SET_CONFIG:
        if (mutex_lock_interruptible(&ctx->mtx))
            return -ERESTARTSYS;
        if (copy_from_user(&ctx->kcfg, (void __user *)arg, sizeof(ctx->kcfg))) {
            mutex_unlock(&ctx->mtx);
            return -EFAULT;
        }
        mutex_unlock(&ctx->mtx);
        return 0;

    default:
        return -ENOTTY;
    }
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
        goto destroy_mutex;
    }

    pr_info("registering char device and assigned major=%d\n", MAJOR(dev));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    fanx_prvctx->dev_class = class_create(FANX_DEVICE_NAME);
#else
    fanx_prvctx->dev_class = class_create(THIS_MODULE, FANX_DEVICE_NAME);
#endif
    if (IS_ERR(fanx_prvctx->dev_class)) {
        ret = PTR_ERR(fanx_prvctx->dev_class);
        goto unreg_region;
    }

    cdev_init(&fanx_prvctx->c_dev, &fanx_fops);
    fanx_prvctx->c_dev.owner = THIS_MODULE;

    ret = cdev_add(&fanx_prvctx->c_dev, dev, FANX_CDEV_MINOR_NR);
    if (ret < 0) {
        pr_alert("registering char device failed with %d\n", ret);
        goto destroy_class;
    }

    devp = fanx_prvctx->dev = device_create(
        fanx_prvctx->dev_class, NULL, dev, NULL, FANX_DEVICE_NAME);
    if (IS_ERR(devp)) {
        ret = PTR_ERR(devp);
        goto del_cdev;
    }

    pr_info("device created on /dev/%s\n", FANX_DEVICE_NAME);
    return 0;

del_cdev:
    cdev_del(&fanx_prvctx->c_dev);
destroy_class:
    class_destroy(fanx_prvctx->dev_class);
unreg_region:
    unregister_chrdev_region(dev, FANX_CDEV_MINOR_NR);
destroy_mutex:
    mutex_destroy(&fanx_prvctx->mtx);
free_mm:
    kfree(fanx_prvctx);
    fanx_prvctx = NULL;
    return ret;
}

static void __exit fanx_exit(void)
{
    dev_t dev;

    if (!fanx_prvctx)
        return;

    dev = fanx_prvctx->c_dev.dev;

    device_destroy(fanx_prvctx->dev_class, dev);
    class_destroy(fanx_prvctx->dev_class);
    cdev_del(&fanx_prvctx->c_dev);
    unregister_chrdev_region(dev, FANX_CDEV_MINOR_NR);
    mutex_destroy(&fanx_prvctx->mtx);
    kfree(fanx_prvctx);
    fanx_prvctx = NULL;
    pr_info("unloaded\n");
}

module_init(fanx_init);
module_exit(fanx_exit);
