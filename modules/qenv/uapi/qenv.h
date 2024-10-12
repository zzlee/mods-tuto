/*
 * QENV Userspace API
 */
#ifndef _UAPI_LINUX_QENV_H
#define _UAPI_LINUX_QENV_H

#include <linux/ioctl.h>
#include <linux/types.h>

/**
 * DOC: QENV Userspace API
 */

/**
 * struct qenv_alloc_args - metadata passed from userspace for
 *                                      allocations
 *
 * Provided by userspace as an argument to the ioctl
 */
struct qenv_test_args {
	__u32 flags;
};


#define QENV_IOC_MAGIC		'Q'

#define QENV_IOCTL_TEST		_IOWR(QENV_IOC_MAGIC, 0x0, struct qenv_test_args)

#endif /* _UAPI_LINUX_QENV_H */
