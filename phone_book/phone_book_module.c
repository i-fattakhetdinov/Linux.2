#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define FAIL 1
#define DEVICE_NAME "phone_book"
#define BUF_LEN 512
#define FIRST_NAME_SIZE 20
#define LAST_NAME_SIZE 20
#define AGE_SIZE 3
#define PHONE_NUMBER_SIZE 11
#define EMAIL_SIZE 30


static int major;
static int is_device_open = 0;
static char command_result[BUF_LEN] = "";
static char command[BUF_LEN];
static char *command_ptr;
static char *command_result_ptr;

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};



struct phone_book_item {
    char first_name[FIRST_NAME_SIZE];
    char last_name[LAST_NAME_SIZE];
    char age[AGE_SIZE];
    char phone_number[PHONE_NUMBER_SIZE];
    char email[EMAIL_SIZE];
    struct list_head list;
};

struct phone_book_item phone_book;

static int __init phone_book_init(void) {
    INIT_LIST_HEAD(&(phone_book.list));
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {    
        printk("Registering the character device failed with %d\n", major);
        return major;
    }
    printk("Major number %d\n", major);
    printk("Created a dev file\n");
    printk("/dev/chardev c %d 0.\n", major);
    return 0;
}

static void __exit phone_book_exit(void) {
    struct list_head *pos, *q;
    struct phone_book_item* item;

    list_for_each_safe(pos, q, &(phone_book.list))
    {
        item = list_entry(pos, struct phone_book_item, list);
        list_del(pos);
        kfree(item);
    }
    
    unregister_chrdev(major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
    if (is_device_open){
        printk("device_open failed");
        return -EBUSY;
    }
    is_device_open++;
    command_result_ptr = command_result;
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "success device_open");
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "device_release");
    is_device_open--;
    module_put(THIS_MODULE);
    return 0;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
    int bytes_read = 0;
    command_result_ptr = command_result;
    if (*command_result_ptr == 0)
        return 0;

    while (length && *command_result_ptr) {
        put_user(*(command_result_ptr++), buffer++);
        length--;
        bytes_read++;
    }
    put_user(*(command_result_ptr++), buffer++);
    memset(command_result, '\0', BUF_LEN);
    return bytes_read;
}

static int string_to_phone_book_item(char *data, struct phone_book_item *item){
    int i = 0;
    char *data_ptr = data;
    for (i=0; data_ptr[i]; data_ptr[i]==' ' ? i++ : *data_ptr++);
    if (i != 4){
        return FAIL;
    }
    strncpy(item->first_name, strsep(&data, " "), FIRST_NAME_SIZE);
    strncpy(item->last_name, strsep(&data, " "), LAST_NAME_SIZE);
    strncpy(item->age, strsep(&data, " "), AGE_SIZE);
    strncpy(item->phone_number, strsep(&data, " "), PHONE_NUMBER_SIZE);
    strncpy(item->email, strsep(&data, " "), EMAIL_SIZE);
    return SUCCESS;
}

static void add(char* data){
    struct phone_book_item *item;
    item = (struct phone_book_item*)kmalloc(sizeof(struct phone_book_item), GFP_KERNEL);
    if (item != NULL){
        if (string_to_phone_book_item(data, item) == SUCCESS){
            INIT_LIST_HEAD(&(item->list));
            list_add_tail(&(item->list), &(phone_book.list));
            sprintf(command_result, "Success add");
        }
        else {
            sprintf(command_result, "Invalid command");
        }
    }
}

static void get(char* last_name){
    last_name[strlen(last_name)] = '\0';
    struct list_head *pos, *q;
    struct phone_book_item* item;
    int num_records = 0;
    int result_length = 0;
    list_for_each_safe(pos, q, &(phone_book.list)){
        item = list_entry(pos, struct phone_book_item, list);
        if (!strcmp(item->last_name, last_name)){
            num_records++;
            if (result_length){
                result_length += sprintf(command_result + result_length, "\n");
            }
            result_length += sprintf(command_result + result_length, "%s %s %s %s %s", 
             item->first_name, item->last_name, item->age, item->phone_number, item->email);
        }
    }
    if (num_records == 0){
        sprintf(command_result, "Record not found");
    }
}

static void del(char *last_name){
    last_name[strlen(last_name)] = '\0';
    struct list_head *pos, *q;
    struct phone_book_item *item;
    int num_dels = 0;
    list_for_each_safe(pos, q, &(phone_book.list)){
        item = list_entry(pos, struct phone_book_item, list);
        if (!strcmp(item->last_name, last_name)){
            num_dels++;
            list_del(&(item->list));
            kfree(item);
            sprintf(command_result, "Success delete");
        }
    }
    if (num_dels == 0){
        sprintf(command_result, "Record not found");
    }
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{   
    command_ptr = command;
    strncpy(command_ptr, buff, BUF_LEN);
    char *command_name = strsep(&command_ptr, " ");
    char *com = strsep(&command_ptr, "\n");
    if (!strcmp("add", command_name)) {
        add(com);
    }
    else if (!strcmp("get", command_name)) {
        get(com);
    }
    else if (!strcmp("del", command_name)) {
        del(com);
    } else {
        sprintf(command_result, "Invalid command");
    }
    return 0;
}

module_init(phone_book_init);
module_exit(phone_book_exit);