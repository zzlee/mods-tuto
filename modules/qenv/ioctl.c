#define pr_fmt(fmt)     "[" KBUILD_MODNAME "]%s(#%d): " fmt, __func__, __LINE__

#include "ioctl.h"
#include "uapi/qenv.h"

#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

long qenv_ioctl_test(struct qenv_device* device, unsigned long arg) {
	struct qenv_test_args args;
	long ret = 0;
	struct device* dev = &device->pdev->dev;

	pr_info("\n");

	ret = copy_from_user(&args, (void __user *)arg, sizeof(args));
	if (ret != 0) {
		pr_err("copy_from_user() failed, err=%d\n", (int)ret);

		ret = -EFAULT;
		goto err;
	}

	ret = copy_to_user((void __user *)arg, &args, sizeof(args));
	if (ret != 0) {
		pr_err("copy_to_user() failed, err=%d\n", (int)ret);

		ret = -EFAULT;
		goto err;
	}

err:
	return ret;
}
