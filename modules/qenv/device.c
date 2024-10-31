#define pr_fmt(fmt)     "[" KBUILD_MODNAME "]%s(#%d): " fmt, __func__, __LINE__

#include "device.h"
#include "cdev.h"
#include "ioctl.h"
#include "uapi/qenv.h"

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/fs.h>

#ifdef CONFIG_COMPAT
#include <asm/compat.h>
#endif

static int __probe(struct platform_device *pdev);
static int __remove(struct platform_device *pdev);

static struct qenv_device* __device_new(void);
static struct qenv_device* __device_get(struct qenv_device* self);
static void __device_put(struct qenv_device* self);

static int __device_start(struct qenv_device* self);
static void __device_stop(struct qenv_device* self);

static long __file_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_COMPAT
static long __file_ioctl_compat(struct file *filep, unsigned int cmd, unsigned long arg);
#endif

static ssize_t __sysfs_export_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t __sysfs_unexport_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count);

#ifdef CONFIG_OF
extern struct of_device_id qenv_of_match[];
#endif // CONFIG_OF

static struct platform_driver __driver = {
	.driver = {
		.name = QENV_DRIVER_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table	= qenv_of_match,
#endif // CONFIG_OF
	},
	.probe  = __probe,
	.remove = __remove,
};

static struct file_operations __fops = {
	.open = qenv_cdev_open,
	.release = qenv_cdev_release,
	.unlocked_ioctl = __file_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = __file_ioctl_compat,
#endif
};

static ssize_t __sysfs_export_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct qenv_device *self = (struct qenv_device *)dev_get_drvdata(sysfs_dev);

	pr_err("EPERM");

	return 0;
}

static ssize_t __sysfs_unexport_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct qenv_device *self = (struct qenv_device *)dev_get_drvdata(sysfs_dev);

	pr_err("EPERM");

	return 0;
}

static DEVICE_ATTR(export, 0664, NULL, __sysfs_export_store);
static DEVICE_ATTR(unexport, 0664, NULL, __sysfs_unexport_store);

static struct attribute *attrs[] = {
	&dev_attr_export.attr,
	&dev_attr_unexport.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __probe(struct platform_device *pdev) {
	int err = 0;
	struct qenv_device* self;

	self = __device_new();
	if(! self) {
		pr_err("__device_new() failed\n");
		err = -ENOMEM;
		goto err0;
	}

	// pr_info("self=%p\n", self);

	self->pdev = pdev;
	platform_set_drvdata(pdev, self);

	err = __device_start(self);
	if(err) {
		pr_err("__device_start() failed, err=%d\n", err);
		goto err1;
	}

	return err;

err1:
	__device_put(self);
err0:
	return err;
}

static int __remove(struct platform_device *pdev) {
	int err = 0;
	struct qenv_device* self = platform_get_drvdata(pdev);

	// pr_info("self=%p\n", self);

	__device_stop(self);
	__device_put(self);
	platform_set_drvdata(pdev, NULL);

	return err;
}

int qenv_device_register(void) {
	int err;

	// pr_info("\n");

	err = qenv_cdev_register();
	if (err != 0) {
		pr_err("qenv_cdev_register() failed, err=%d\n", err);
		goto err0;
	}

	err = platform_driver_register(&__driver);
	if(err) {
		pr_err("platform_driver_register() failed\n");
		goto err1;
	}

	return err;

err1:
	qenv_cdev_unregister();
err0:
	return err;
}

void qenv_device_unregister(void) {
	// pr_info("\n");

	platform_driver_unregister(&__driver);
	qenv_cdev_unregister();
}

static struct qenv_device* __device_new(void) {
	struct qenv_device* self = kzalloc(sizeof(struct qenv_device), GFP_KERNEL);

	kref_init(&self->ref);

	return self;
}

static struct qenv_device* __device_get(struct qenv_device* self) {
	if (self)
		kref_get(&self->ref);

	return self;
}

static void __device_free(struct kref *ref)
{
	struct qenv_device* self = container_of(ref, struct qenv_device, ref);

	// pr_info("\n");

	kfree(self);
}

static void __device_put(struct qenv_device* self) {
	if (self)
		kref_put(&self->ref, __device_free);
}

static int __device_start(struct qenv_device* self) {
	int err;
	struct device* dev = &self->pdev->dev;

	pr_info("\n");

	self->cdev.private_data = self;
	self->cdev.fops = &__fops;

	err = qenv_cdev_start(&self->cdev);
	if(err) {
		pr_err("qenv_cdev_start() failed, err=%d\n", err);
		goto err0;
	}

	err = sysfs_create_group(&dev->kobj, &attr_group);
	if (err) {
		dev_err(dev, "sysfs group creation (%d) failed \n", err);

		goto err1;
	}

	return 0;

err1:
	qenv_cdev_stop(&self->cdev);
err0:
	return err;
}

static void __device_stop(struct qenv_device* self) {
	struct device* dev = &self->pdev->dev;

	pr_info("\n");

	sysfs_remove_group(&dev->kobj, &attr_group);
	qenv_cdev_stop(&self->cdev);
}

static long __file_ioctl(struct file * filp, unsigned int cmd, unsigned long arg) {
	long ret;
	struct qenv_device* device = filp->private_data;

	switch(cmd) {
	case QENV_IOCTL_TEST:
		ret = qenv_ioctl_test(device, arg);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long __file_ioctl_compat(struct file *filep, unsigned int cmd, unsigned long arg) {
	return __file_ioctl(filep, cmd, (unsigned long)compat_ptr(arg));
}
#endif
