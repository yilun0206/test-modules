obj-m := timer.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf .tmp_versions *~ *.o *ko *.mod.c .*.cmd *.markers *.order *sysmvers *.symvers .*.swp
