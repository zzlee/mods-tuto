#ifndef __QENV_DEVICE_H__
#define __QENV_DEVICE_H__

#include "cdev.h"

#define QENV_DRIVER_NAME "qenv"

struct qenv_device {
	struct kref ref;
	struct platform_device* pdev;

	// cdev
	struct qenv_cdev cdev;
};

int qenv_device_register(void);
void qenv_device_unregister(void);

#endif // __QENV_DEVICE_H__