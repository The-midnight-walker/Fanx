// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
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
MODULE_DESCRIPTION
    ("Linux character device driver that controls and monitors a system fan");
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
	(void)inode;
	file->private_data = NULL;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long fanx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int fanx_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
		      unsigned long arg)
#endif
{
	struct fanx_prvctx_struct *ctx = filp->private_data;

	if (!ctx)
		return -EINVAL;

	if (_IOC_TYPE(cmd) != IOCTL_FANX_IOCMAGIC ||
	    _IOC_NR(cmd) > IOCTL_FANX_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
		/* Global and Configuration */
	case IOCTL_FANX_IOCRESET:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		memset(&ctx->kcfg, 0, sizeof(ctx->kcfg));
		ctx->kcfg.mode = FANX_MODE_MAN;
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCQCONFIG:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (copy_to_user
		    ((void __user *)arg, &ctx->kcfg, sizeof(ctx->kcfg))) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCSCONFIG:{
			struct fanx_ds user_cfg;

			if (copy_from_user
			    (&user_cfg, (void __user *)arg, sizeof(user_cfg)))
				return -EFAULT;

			if (user_cfg.target_rpm > FANX_RPM_MAX)
				return -EINVAL;
			if (user_cfg.mode > FANX_MODE_RTIME)
				return -EINVAL;
			if (user_cfg.timer_total_s > FANX_RTIME_MAX)
				return -EINVAL;

			if (mutex_lock_interruptible(&ctx->mtx))
				return -ERESTARTSYS;

			/* Apply only configurable fields; protect read-only metrics */
			ctx->kcfg.target_rpm = user_cfg.target_rpm;
			ctx->kcfg.mode = user_cfg.mode;
			ctx->kcfg.timer_total_s = user_cfg.timer_total_s;
			if (user_cfg.mode == FANX_MODE_RTIME)
				ctx->kcfg.timer_left_s = user_cfg.timer_total_s;
			strncpy(ctx->kcfg.profile, user_cfg.profile,
				sizeof(ctx->kcfg.profile) - 1);
			ctx->kcfg.profile[sizeof(ctx->kcfg.profile) - 1] = '\0';

			mutex_unlock(&ctx->mtx);
			return 0;
		}

		/* State and Power Control */
	case IOCTL_FANX_IOCSTART:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		ctx->kcfg.running = 1;
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCSTOP:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		ctx->kcfg.running = 0;
		ctx->kcfg.rpm = 0;
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCQSTATE:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.running, (__u8 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

		/* Speed (RPM) */
	case IOCTL_FANX_IOCQRPM:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.rpm, (__u32 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCSRPM:{
			__u32 rpm_val;

			if (get_user(rpm_val, (__u32 __user *) arg))
				return -EFAULT;
			if (rpm_val > FANX_RPM_MAX)
				return -EINVAL;
			if (mutex_lock_interruptible(&ctx->mtx))
				return -ERESTARTSYS;
			ctx->kcfg.target_rpm = rpm_val;
			mutex_unlock(&ctx->mtx);
			return 0;
		}

		/* Operating Mode */
	case IOCTL_FANX_IOCQMODE:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.mode, (__u8 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCSMODE:{
			__u8 mode_val;

			if (get_user(mode_val, (__u8 __user *) arg))
				return -EFAULT;
			if (mode_val > FANX_MODE_RTIME)
				return -EINVAL;
			if (mutex_lock_interruptible(&ctx->mtx))
				return -ERESTARTSYS;
			ctx->kcfg.mode = mode_val;
			mutex_unlock(&ctx->mtx);
			return 0;
		}

		/* Thermal Metrics */
	case IOCTL_FANX_IOCQTEMP:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.temp_mc, (__s32 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

	case IOCTL_FANX_IOCQLEVEL:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.level, (__u8 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

		/* Timer Mode */
	case IOCTL_FANX_IOCSTIMER:{
			__u32 timer_s;

			if (get_user(timer_s, (__u32 __user *) arg))
				return -EFAULT;
			if (timer_s > FANX_RTIME_MAX)
				return -EINVAL;
			if (mutex_lock_interruptible(&ctx->mtx))
				return -ERESTARTSYS;
			ctx->kcfg.timer_total_s = timer_s;
			ctx->kcfg.timer_left_s = timer_s;
			ctx->kcfg.mode = FANX_MODE_RTIME;
			ctx->kcfg.running = 1;
			mutex_unlock(&ctx->mtx);
			return 0;
		}

	case IOCTL_FANX_IOCQTIMER:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (put_user(ctx->kcfg.timer_left_s, (__u32 __user *) arg)) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

		/* Profile */
	case IOCTL_FANX_IOCSPROFILE:{
			char prof_buf[sizeof(ctx->kcfg.profile)];

			if (copy_from_user
			    (prof_buf, (void __user *)arg, sizeof(prof_buf)))
				return -EFAULT;
			prof_buf[sizeof(prof_buf) - 1] = '\0';

			if (mutex_lock_interruptible(&ctx->mtx))
				return -ERESTARTSYS;
			memcpy(ctx->kcfg.profile, prof_buf, sizeof(prof_buf));
			mutex_unlock(&ctx->mtx);
			return 0;
		}

	case IOCTL_FANX_IOCQPROFILE:
		if (mutex_lock_interruptible(&ctx->mtx))
			return -ERESTARTSYS;
		if (copy_to_user((void __user *)arg,
				 ctx->kcfg.profile,
				 sizeof(ctx->kcfg.profile))) {
			mutex_unlock(&ctx->mtx);
			return -EFAULT;
		}
		mutex_unlock(&ctx->mtx);
		return 0;

	default:
		return -ENOTTY;
	}
}

static const struct file_operations fanx_ioctl_intf_fops = {
	.owner = THIS_MODULE,
	.open = fanx_open,
	.release = fanx_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	.llseek = no_llseek,
#else
	.llseek = noop_llseek,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = fanx_ioctl,
#else
	.ioctl = fanx_ioctl,
#endif
};

static int fanx_ioctl_intf_open(struct inode *inode, struct file *filp)
{
	unsigned int minor;

	minor = iminor(inode);
	pr_debug("device node with minor # %d being used\n", minor);

	switch (minor) {
	case FANX_CDEV_BASEMINOR:
		filp->f_op = &fanx_ioctl_intf_fops;
		break;
	default:
		return -ENXIO;
	}

	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode, filp);

	return 0;
}

static const struct file_operations fanx_fops = {
	.owner = THIS_MODULE,
	.open = fanx_ioctl_intf_open,
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

	ret =
	    alloc_chrdev_region(&dev, FANX_CDEV_BASEMINOR, FANX_CDEV_MINOR_NR,
				FANX_DEVICE_NAME);
	if (ret < 0) {
		pr_alert("registering char device failed with %d\n", ret);
		goto free_mm;
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

	// TODO: implement several /dev/fanxX support
	ret = cdev_add(&fanx_prvctx->c_dev, dev, FANX_CDEV_MINOR_NR);
	if (ret < 0) {
		pr_alert("registering char device failed with %d\n", ret);
		goto destroy_class;
	}

	devp = fanx_prvctx->dev =
	    device_create(fanx_prvctx->dev_class, NULL, dev, NULL,
			  FANX_DEVICE_NAME);
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
free_mm:
	mutex_destroy(&fanx_prvctx->mtx);
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
