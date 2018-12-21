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

asmlinkage long sys_del_user(const char* surname, unsigned int len) {
    int fd;
    char command[BUF_LEN] = "del ";
    strncat(command, surname, len);
    
    mm_segment_t fs = get_fs();
    set_fs(KERNEL_DS);

    fd = sys_open(DEVICE_NAME, O_RDWR, 10);

    sys_write(fd, command, BUF_LEN);
    sys_close(fd);

    set_fs(fs);

    return 0;
}