#ifndef __SIMPLE_DRIVER_
#define __SIMPLE_DRIVER_

#define DIR_NAME "simpledriver_dir"
#define FILE_NAME "simpledriver_file"
#define BUFFER_SIZE 255

static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode, struct file *file);
static ssize_t write_proc(struct file *filp, const char *userbuffer, size_t len, loff_t * off);
static ssize_t read_proc(struct file *filp, char __user *userbuffer, size_t length,loff_t * offset);

#endif