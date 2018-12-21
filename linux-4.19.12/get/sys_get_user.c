#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>


#define DEVICE_NAME "/dev/phone_book"
#define BUF_LEN 512
#define FIRST_NAME_SIZE 20
#define LAST_NAME_SIZE 20
#define AGE_SIZE 3
#define PHONE_NUMBER_SIZE 11
#define EMAIL_SIZE 30

struct user_data {
    char first_name[FIRST_NAME_SIZE];
    char last_name[LAST_NAME_SIZE];
    char age[AGE_SIZE];
    char phone_number[PHONE_NUMBER_SIZE];
    char email[EMAIL_SIZE];
};

static int string_to_phone_book_item(char *data, struct phone_book_item *item) {
    int i = 0;
    char *data_ptr = data;
    for (i = 0; data_ptr[i]; data_ptr[i] == ' ' ? i++ : *data_ptr++);
    if (i != 4) {
        return FAIL;
    }
    strncpy(item->first_name, strsep(&data, " "), FIRST_NAME_SIZE);
    strncpy(item->last_name, strsep(&data, " "), LAST_NAME_SIZE);
    strncpy(item->age, strsep(&data, " "), AGE_SIZE);
    strncpy(item->phone_number, strsep(&data, " "), PHONE_NUMBER_SIZE);
    strncpy(item->email, strsep(&data, " "), EMAIL_SIZE);
    return SUCCESS;
}


asmlinkage long sys_get_user(const char* suranme, unsigned int len, struct user_data* output_data) {
    int fd;
    char command[BUF_LEN] = "get ";
    char command_result[BUF_LEN];
    strncat(command, surname, len);
    
    mm_segment_t fs = get_fs();
    set_fs(KERNEL_DS);

    fd = sys_open(DEVICE_NAME, O_RDWR, 10);
    
    sys_write(fd, command, BUF_LEN);
    sys_read(fd, command_result, BUF_LEN);
    string_to_phone_book_item(command_result, output_data);

    sys_close(fd);
    
    set_fs(fs);
    
    return 0;
}