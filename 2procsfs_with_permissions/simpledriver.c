
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "simpledriver.h"


char kernel_buffer[BUFFER_SIZE] = "";

struct proc_dir_entry* procfs_dir = NULL ; 
struct proc_dir_entry* procfs_file = NULL ; 
 

static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};

//Loading
static int __init  simpledriver_init(void){ 

    procfs_dir = proc_mkdir(DIR_NAME, NULL); //under proc
    procfs_file = proc_create(FILE_NAME,0444, procfs_dir,&proc_fops);
    printk("simple driver start\n");
    return 0 ;
}

//unloading 
static void  __exit simpledriver_exit(void){ 
    
    DELETING_ENTRY:
        remove_proc_entry(FILE_NAME, procfs_dir);
    DELETING_DIR:
        remove_proc_entry(DIR_NAME, NULL);
    printk("simple driver exit\n");
}


static int open_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc file opend.....\t");
    return 0;
}

static int release_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc file released.....\n");
    return 0;
}

static ssize_t write_proc(struct file *filp, const char *userbuffer, size_t len, loff_t * off)
{

    printk(KERN_INFO "proc file write.....\t");
    if (len + *off > BUFFER_SIZE){
        len = BUFFER_SIZE - *off;
    }
    int ret ;
    ret = copy_from_user(&kernel_buffer[*off], userbuffer, len);
    if (ret!= 0){
        return -EFAULT;
    }
    return len;
}

static ssize_t read_proc(struct file *filp, char __user *userbuffer, size_t length,loff_t * offset)
{

    if (length  + *offset >  BUFFER_SIZE) {
        length = BUFFER_SIZE - *offset;
    }
    
    int ret ;
    ret = copy_to_user(userbuffer, &kernel_buffer[*offset], length);
    if (ret!= 0){
        return -EFAULT;
    }
    return length ;  //success read bytes
}


//entry points
module_init(simpledriver_init);

//exit point
module_exit(simpledriver_exit);

/*LICENSE */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ziad Assem");
MODULE_DESCRIPTION("a simple driver");

