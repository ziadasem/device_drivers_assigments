### ProcFs

a procfs (Process Filesystem)  is a filesystem in memory. where processes store their information about themselves on this virtual filesystem. ProcFS also stores other system information.

It can act as a bridge connecting the **user space** and the **kernel space**. Userspace programs can use proc files to read the information exported by the kernel. Every entry in the proc file system provides some information from the kernel.

Most proc files are read-only and only expose kernel information to user space programs.

proc files can also be used to control and modify kernel behavior on the fly. The proc files need to be writable in this case.

**Difference between procfs and sysfs:**
The Linux kernel developers decided that this was too much of a mess. So, they created sysfs , which is mounted at /sys . **sysfs , like procfs , exposes internal information from the kernel to the user**. However, for sysfs , the developers imposed a strict set of rules for how to expose this information.

**NOTE: this driver is written for Linux kernel v6.5.0**



#### 1- init the driver
```c

#include <linux/module.h>
#include <linux/init.h>

//Loading
static int __init  simpledriver_init(void){  //accessed by this file and store in init section
    printk("simple driver start\n");
    return 0 ;
}

//unloading 
static void  __exit simpledriver_exit(void){ // __exit to remove this section in static driver
    printk("simple driver exit\n");
}
//entry points
module_init(simpledriver_init);
//exit point
module_exit(simpledriver_exit);

/*LICENSE */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ziad Assem");
MODULE_DESCRIPTION("a simple driver");
```

#### 2- Creating Procfs Entry

an entry can be directory, file, symbolic link, or a device node

**Directory**

use the following function (API) to create new directory under /proc/
```c
struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent)
```

where,

**`name`**: The name of the directory that will be created under **`/proc`**.  
**`parent`**: In case the folder needs to be created in a subfolder under **`/proc`** a pointer to the same is passed else it can be left as **`NULL`**.

in the init function 
```c
//Loading
static  int  __init  simpledriver_init(void){ //accessed by this file and store in init section

//1- create the procfs directory

procfs_dir = proc_mkdir("simple_driver", NULL); //undrt proc
....
}
```
**File**

```c
struct proc_dir_entry *proc_create ( const char *name, umode_t mode, struct proc_dir_entry *parent, const struct file_operations *proc_fops )
```

for lower kernel versions
```c
create_proc_read_entry()
create_proc_entry() 
```

```c
#define  DIR_NAME  "simpledriver_dir"
#define  FILE_NAME  "simpledriver_file"
struct  proc_dir_entry* procfs_dir = NULL ;
struct  proc_dir_entry* procfs_file = NULL ;
//Loading
static  int  __init  simpledriver_init(void){ //accessed by this file and store in init section

//1- create the procfs directory and file

procfs_dir = proc_mkdir(DIR_NAME, NULL); //under proc
procfs_file = proc_create(FILE_NAME,0666,procfs_dir,&proc_fops);
printk("simple driver start\n");
return  0 ;
}
```
`&proc_fops` will be defined in the next section

**deleting**
any new entry should deleted before the end of the module to avoid memory leakage 

```c
void remove_proc_entry(const char *, struct proc_dir_entry *)
``` 

```c
//unloading 
static void  __exit simpledriver_exit(void){ // __exit to remove this section in static driver
    
    DELETING_ENTRY:
        remove_proc_entry(FILE_NAME, procfs_dir);
    DELETING_DIR:
        remove_proc_entry(DIR_NAME, NULL);
    printk("simple driver exit\n");
}
```

**Proc file operations**`proc_fops`
on reading, writing, opening, ...etc of a file/entry a custom defined function should be executed, kernel provides this interface to the developer by letting him define a `proc_fops` structure.

```c
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};
```

open and release functions are optional.
```c

//at top of the file
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
```
**the required driver is to read from user and save it in its kernel buffer and output it when user reads the driver (e.g. use `cat` command)**

read and write between `user space` and `kernel space`.

user space
----||----
*userbuffer*
----||----
kernel space

user space writes and reads to/from userbuffer and the kernel reads and writes from/to userbuffer. to access this buffer use `copy_from_user` and `copy_to_user` functions

```c
static  inline  unsigned  long  copy_from_user(void  *to, const  void  *from, unsigned  long  n)
```

copy_from_user and copy_to_user returns a status code to indicate if the operation was successful or not. hence an int value is used to catch error (less than 0 means errors).

read and write function should return how many bytes are processed (read and write) and it can be invoked many times the read/write a string which is more than kernel_buffer, so offset and some conditions are used.

```c
static ssize_t write_proc(struct file *filp, const char *userbuffer, size_t len, loff_t * off)
{

    printk(KERN_INFO "proc file write.....\t");
    if (count + *off > SIZE){
        count = SIZE - *off;
    }
    int ret ;
    ret = copy_to_user(&buffer[*off], _shared_buffer, count);
    if (ret!= 0){
        return -EFAULT;
    }
    return count;
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
    *offset = length ; //update the cursor to prevent continous reading
    return length ;  //success read bytes
}
```

the complete code is in `sample_driver_example.c` and `sample_driver_example.h`.

references [procfs-in-linux](https://embetronicx.com/tutorials/linux/device-drivers/procfs-in-linux/) and [cmu](https://www.cs.cmu.edu/afs/grand.central.org/archive/twiki/pub/Main/SumitKumar/procfs-guide.pdf)
