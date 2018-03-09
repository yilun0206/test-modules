#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/mman.h>
#include <linux/namei.h>

struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
};

struct getdents_callback {
	struct dir_context ctx;
	struct linux_dirent * current_dir;
	struct linux_dirent * previous;
	int count;
	int error;
};

static int filldir(void * __buf, const char * name, int namlen, loff_t offset,
		   u64 ino, unsigned int d_type)
{
	struct linux_dirent * dirent; /* kernel dirent */
	struct getdents_callback * buf = (struct getdents_callback *) __buf;
	unsigned long d_ino;
	int reclen = ALIGN(offsetof(struct linux_dirent, d_name) + namlen + 2,
		sizeof(long));

	buf->error = -EINVAL;	/* only used if we fail.. */
	if (reclen > buf->count)
		return -EINVAL;
	d_ino = ino;
	if (sizeof(d_ino) < sizeof(ino) && d_ino != ino) {
		buf->error = -EOVERFLOW;
		return -EOVERFLOW;
	}
	dirent = buf->previous;
	if (dirent) {
		dirent->d_off = offset;
	}
	dirent = buf->current_dir;
	dirent->d_ino = d_ino;
	dirent->d_reclen = reclen;
	memcpy(dirent->d_name, name, namlen);
	memset(dirent->d_name + namlen, '\0', 1);
	*((char *) dirent + reclen - 1) = d_type;

	buf->previous = dirent;
	dirent = (void *)dirent + reclen;
	buf->current_dir = dirent;
	buf->count -= reclen;
	return 0;
}

static long do_getdents(const char *pathname,
		struct linux_dirent *dirent, unsigned int count)
{
	struct file * filp;
	struct linux_dirent * lastdirent;
	struct getdents_callback buf = {
		.ctx.actor = filldir,
		.count = count,
		.current_dir = dirent
	};
	int error;

	filp = filp_open(pathname, O_RDONLY | O_DIRECTORY, 0);
	if (IS_ERR_OR_NULL(filp))
		return -EBADF;

	error = iterate_dir(filp, &buf.ctx);
	if (error >= 0)
		error = buf.error;
	lastdirent = buf.previous;
	if (lastdirent) {
		lastdirent->d_off = buf.ctx.pos;
		error = count - buf.count;
	}
	filp_close(filp, NULL);
	return error;
}

static int __init temp_init_module(void)
{
	char kbuf[777];
	long ret = 0;

	ret = do_getdents("/root/yilun/", (struct linux_dirent *)kbuf, 777);
	printk("ret = %ld\n", ret);

	return ret;
}

static void __exit temp_exit_module(void)
{
	printk("Goodbye\n");
}

module_init(temp_init_module);
module_exit(temp_exit_module);
MODULE_AUTHOR("yilun");
MODULE_LICENSE("GPL");
