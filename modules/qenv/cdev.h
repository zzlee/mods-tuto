#ifndef __QENV_CDEV_H__
#define __QENV_CDEV_H__

#include <linux/cdev.h>

struct qenv_cdev {
	dev_t cdevno;
	struct cdev cdev;
	struct file_operations* fops;
	void* private_data;
};

int qenv_cdev_register(void);
void qenv_cdev_unregister(void);

int qenv_cdev_start(struct qenv_cdev* self);
void qenv_cdev_stop(struct qenv_cdev* self);

// default file_operations
int qenv_cdev_open(struct inode *inode, struct file *filp);
int qenv_cdev_release(struct inode *inode, struct file *filep);


#endif // __QENV_CDEV_H__