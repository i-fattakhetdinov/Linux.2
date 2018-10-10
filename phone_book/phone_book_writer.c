#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUF_LEN 512
#define DEVICE_FILE_NAME "/dev/phone_book"


int main(int argc, char* argv[]){
    
    char command[BUF_LEN];
    char result[BUF_LEN];

    int fd = open(DEVICE_FILE_NAME, O_RDWR);
    
    if (fd < 0) {
        printf ("Open device failed");
        return 1;
    }

    while(1){
        printf("dev/phone_book$  ");
        fgets(command, BUF_LEN, stdin);
        
        if (command[0]=='\0' || command[0]=='\n')
         continue;
        if (!strncmp(command, "exit", 4)){
            close(fd);
            return 0;
        }
        write(fd, command, BUF_LEN);
        read(fd, result, BUF_LEN);
        printf("%s\n",result);
    }
    close(fd);
    return 0;
}