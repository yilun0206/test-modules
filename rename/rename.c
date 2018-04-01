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
#include <linux/cred.h>

/* port from linux /fs/namei.c, func: do_unlinkat
 * @ pathname: absolute path of directory to be created
 */
static long do_unlink(const char *pathname)
{
	long error;
	struct dentry *dentry;
	struct inode *dir;
	struct path path;
	struct inode *tmp;
	unsigned int lookup_flags = LOOKUP_FOLLOW;

	error = kern_path(pathname, lookup_flags, &path);
	if (error)
		return error;

	dentry = path.dentry;
	dir = dentry->d_parent->d_inode;

	mutex_lock(&dir->i_mutex);
	//error = vfs_unlink(dir, dentry, &tmp);
	error = vfs_unlink(dir, dentry);
	mutex_unlock(&dir->i_mutex);
	path_put(&path);

	return error;
}

static long do_link(const char *oldname, const char *newname, int flags)
{
	struct dentry *new_dentry;
	struct path old_path, new_path;
	int how = 0;
	long error;

	if ((flags & ~(AT_SYMLINK_FOLLOW | AT_EMPTY_PATH)) != 0)
		return -EINVAL;
	/*
	 * To use null names we require CAP_DAC_READ_SEARCH
	 * This ensures that not everyone will be able to create
	 * handlink using the passed filedescriptor.
	 */
	if (flags & AT_EMPTY_PATH) {
		if (!capable(CAP_DAC_READ_SEARCH))
			return -ENOENT;
		how = LOOKUP_EMPTY;
	}

	if (flags & AT_SYMLINK_FOLLOW)
		how |= LOOKUP_FOLLOW;
retry:
	error = kern_path(oldname, how, &old_path);
	if (error)
		return error;

	new_dentry = kern_path_create(AT_FDCWD, newname, &new_path,
					(how & LOOKUP_REVAL));
	error = PTR_ERR(new_dentry);
	if (IS_ERR(new_dentry))
		goto out;

	error = -EXDEV;
	if (old_path.mnt != new_path.mnt)
		goto out_dput;
	
	error = vfs_link(old_path.dentry, new_path.dentry->d_inode, new_dentry);
out_dput:
	done_path_create(&new_path, new_dentry);
	if (retry_estale(error, how)) {
		how |= LOOKUP_REVAL;
		goto retry;
	}
out:
	path_put(&old_path);

	return error;
}

static long do_rename(char *oldname, char *newname)
{
	long error;

	error = do_unlink(newname);
	if (error && (error != -ENOENT))
		goto out;
	error = do_link(oldname, newname, 0);
	if (error)
		goto out;
	error = do_unlink(oldname);

out:
	return error;
}

static int __init temp_init_module(void)
{
	long ret = 0;

	ret = do_rename("/home/yilun/testdir/tmpfile1", "/home/yilun/testdir/tmpfile2");
	if (ret)
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
