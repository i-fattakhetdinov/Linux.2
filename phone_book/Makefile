obj-m += phone_book_module.o
all:
	gcc phone_book_writer.c -o phone_book
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
mount:
	sudo insmod phone_book_module.ko
	sudo mknod /dev/phone_book c 244 0
unmount:
	sudo rmmod phone_book_module.ko
	sudo rm /dev/phone_book
run:
	sudo ./phone_book
