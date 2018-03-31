#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/mman.h>
#include <linux/namei.h>
#include <linux/mutex.h>
#include <linux/mount.h>


static long do_rename(const char *oldname, const char *newname)
{
	struct dentry *old_dir, *new_dir;
	struct dentry *old_dentry, *new_dentry;
	struct dentry *trap;
	struct path oldparent, newparent, old, new;
	unsigned int lookup_flags = 0;
	bool should_retry = false;
	long error;
retry:
	error = kern_path(oldname, lookup_flags | LOOKUP_PARENT, &oldparent);
	if (error)
		goto exit;

	error = kern_path(newname, lookup_flags | LOOKUP_PARENT, &newparent);
	if (error)
		goto exit1;

	error = -EXDEV;
	if (oldparent.mnt != newparent.mnt)
		goto exit2;

	old_dir = oldparent.dentry;
	new_dir = newparent.dentry;

	error = mnt_want_write(oldparent.mnt);
	if (error)
		goto exit2;

	trap = lock_rename(new_dir, old_dir);

	lookup_flags |= LOOKUP_FOLLOW;

	error = kern_path(oldname, lookup_flags, &old);
	if (error)
		goto exit3;
	old_dentry = old.dentry;
	
	/* source should not be ancestor of target */
	error = -EINVAL;
	if (old_dentry == trap)
		goto exit4;

	new_dentry = kern_path_create(AT_FDCWD, newname,
				&new, lookup_flags | LOOKUP_EMPTY);
	if (IS_ERR(new_dentry)) {
		error = PTR_ERR(new_dentry);
		goto exit4;
	}
	
	/* target should not be an ancestor of source */
	error = -ENOTEMPTY;
	if (new_dentry == trap)
		goto exit5;

	error = vfs_rename(old_dir->d_inode, old_dentry,
				   new_dir->d_inode, new_dentry);

exit5:
	path_put(&new);
exit4:
	path_put(&old);
exit3:
	unlock_rename(new_dir, old_dir);
	mnt_drop_write(oldparent.mnt);
exit2:
	if (retry_estale(error, lookup_flags))
		should_retry = true;
	path_put(&newparent);
exit1:
	path_put(&oldparent);
	if (should_retry) {
		should_retry = false;
		lookup_flags |= LOOKUP_REVAL;
		goto retry;
	}
exit:
	return error;
}

static int __init temp_init_module(void)
{
	long ret = 0;

	ret = do_rename("/home/yilun/testfile", "/home/yilun/testfile2");
	
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
