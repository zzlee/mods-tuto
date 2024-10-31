#define pr_fmt(fmt)     "[" KBUILD_MODNAME "]%s(#%d): " fmt, __func__, __LINE__

#include "cdev.h"
#include "ioctl.h"
#include "uapi/qenv.h"

#include <linux/version.h>
#include <linux/device.h>
#include <linux/fs.h>

#define QENV_NODE_NAME		"qenv"
#define QENV_MINOR_BASE		(0)
#define QENV_MINOR_COUNT	(255)

static struct class *g_class = NULL;
static int g_major = 0;
static int g_cdevno_base = 0;

static ssize_t __sysfs_export_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t __sysfs_unexport_store(struct device *sysfs_dev, struct device_attribute *attr, const char *buf, size_t count);

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

int qenv_cdev_register(void) {
	int err;
	dev_t dev;

	pr_info("\n");

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,4,0)
	g_class = class_create(THIS_MODULE, QENV_NODE_NAME);
#else
	g_class = class_create(QENV_NODE_NAME);
#endif
	if (IS_ERR(g_class)) {
		pr_err("class_create() failed, g_class=%p\n", g_class);
		err = -EINVAL;
		goto err0;
	}

pr_info("sysfs");
	err = sysfs_create_group(g_class->dev_kobj, &attr_group);
	if (err) {
		pr_err("sysfs_create_group() failed, err=%d\n", err);

		goto err0_1;
	}

pr_info("chrdev_region");
	err = alloc_chrdev_region(&dev, QENV_MINOR_BASE, QENV_MINOR_COUNT, QENV_NODE_NAME);
	if (err) {
		pr_err("alloc_chrdev_region() failed, err=%d\n", err);
		goto err1;
	}

	g_major = MAJOR(dev);
	pr_info("g_major=%d\n", g_major);

	g_cdevno_base = 0;

	return err;

err1:
	sysfs_remove_group(g_class->dev_kobj, &attr_group);
err0_1:
	class_destroy(g_class);
err0:
	return err;
}

void qenv_cdev_unregister(void) {
	pr_info("\n");

	sysfs_remove_group(g_class->dev_kobj, &attr_group);
	unregister_chrdev_region(MKDEV(g_major, QENV_MINOR_BASE), QENV_MINOR_COUNT);
	class_destroy(g_class);
}

int qenv_cdev_start(struct qenv_cdev* self) {
	int err;
	struct device* new_device;

	pr_info("\n");

	self->cdevno = MKDEV(g_major, g_cdevno_base++);
	cdev_init(&self->cdev, self->fops);
	self->cdev.owner = THIS_MODULE;
	err = cdev_add(&self->cdev, self->cdevno, 1);
	if (err) {
		pr_err("cdev_add() failed, err=%d\n", err);
		goto err0;
	}

	new_device = device_create(g_class, NULL, self->cdevno, self,
		QENV_NODE_NAME "%d", MINOR(self->cdevno));
	if (IS_ERR(new_device)) {
		pr_err("device_create() failed, new_device=%p\n", new_device);
		goto err1;
	}

	return 0;

err1:
	cdev_del(&self->cdev);
err0:
	return err;
}

void qenv_cdev_stop(struct qenv_cdev* self) {
	pr_info("\n");

	device_destroy(g_class, self->cdevno);
	cdev_del(&self->cdev);
}

int qenv_cdev_open(struct inode *inode, struct file *filp) {
	struct qenv_cdev* self = container_of(inode->i_cdev, struct qenv_cdev, cdev);

	filp->private_data = self->private_data;
	nonseekable_open(inode, filp);

	return 0;
}

int qenv_cdev_release(struct inode *inode, struct file *filep) {
	struct qenv_cdev* self = container_of(inode->i_cdev, struct qenv_cdev, cdev);

	return 0;
}
