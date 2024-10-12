#define pr_fmt(fmt)     "[" KBUILD_MODNAME "]%s(#%d): " fmt, __func__, __LINE__

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#include "version.h"
#include "device.h"
#include "cdev.h"

#define DRV_MODULE_DESC		"QCAP env Driver"

static char version[] = DRV_MODULE_DESC " v" DRV_MODULE_VERSION;

MODULE_AUTHOR("ZzLab");
MODULE_DESCRIPTION(DRV_MODULE_DESC);
MODULE_VERSION(DRV_MODULE_VERSION);
MODULE_LICENSE("Dual BSD/GPL");

#ifdef CONFIG_OF
struct of_device_id qenv_of_match[] = {
	{ .compatible = "yuan,qenv", },
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(of, qenv_of_match);
#else // CONFIG_OF
static struct platform_device *pdev;
#endif // CONFIG_OF

static int __init qenv_mod_init(void)
{
	int err;

	pr_info("%s\n", version);

	err = qenv_device_register();
	if (err != 0) {
		pr_err("qenv_device_register() failed, err=%d\n", err);
		goto err0;
	}

#ifndef CONFIG_OF
	pdev = platform_device_alloc(QENV_DRIVER_NAME, 0);
	if (pdev == NULL) {
		pr_err("platform_device_alloc() failed\n");
		err = -ENOMEM;
		goto err1;
	}

	err = platform_device_add(pdev);
	if (err != 0) {
		pr_err("platform_device_add() failed, err=%d\n", err);
		err = -ENOMEM;
		goto err2;
	}
#endif // CONFIG_OF

	return err;

#ifndef CONFIG_OF
err2:
	platform_device_put(pdev);
err1:
	qenv_device_unregister();
#endif // CONFIG_OF
err0:
	return err;
}

static void __exit qenv_mod_exit(void)
{
	pr_info("%s\n", version);

#ifndef CONFIG_OF
	platform_device_del(pdev);
	platform_device_put(pdev);
#endif // CONFIG_OF

	qenv_device_unregister();
}

module_init(qenv_mod_init);
module_exit(qenv_mod_exit);
