obj-m:=dht11.o
PWD:=$(shell pwd)
KDIR:=$(KERNEL_PATH)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) 
clean:
	rm -rf *.ko *.order *.symvers *.cmd *.o *.mod.c *.tmp_versions .*.cmd .tmp_versions
