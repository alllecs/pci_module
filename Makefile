obj-m += test_pci_module.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
run:
	sudo insmod test_pci_module.ko size=1024 iter=3
	sudo rmmod test_pci_module.ko
	dmesg
