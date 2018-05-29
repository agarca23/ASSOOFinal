#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO  */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/fs.h>           /* libfs stuff           */
//#include <asm/uaccess.h>        /* copy_to_user          */
#include <linux/buffer_head.h>  /* buffer_head           */
#include <linux/slab.h>         /* kmem_cache            */
#include "assoofs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andres Garcia Alvarez");

static struct kmem_cache *assoofs_inode_cache ;


static struct file_system_type assoofs_type ={
	.owner=THIS_MODULE,
	.name="assoofs",
	//.mount=assoofs_mount,
	.kill_sb=kill_litter_super,
};

static const struct super_operations assoofs_ops = {
	.drop_inode     = generic_delete_inode ,
};

static struct inode_operations assoofs_inode_ops = {
	.create = assoofs_create,
	.lookup = assoofs_lookup,
	.mkdir = assoofs_mkdir,
};

int assoofs_fill_super(struct super_block *sb, void *data, int silent){
	struct inode *root_inode;
	struct buffer_head *bh;
	struct assoofs_super_block_info *assoofs_sb;

	bh=sb_bread(sb,ASSOOFS_SUPERBLOCK_BLOCK_NUMBER);
	assoofs_sb=(struct assoofs_super_block_info *)bh->b_data;

	printk(KERN_INFO "The magic number obtained in disk is %llu\n", assoofs_sb->magic);
	if(assoofs_sb->magic!=ASSOOFS_MAGIC){
		printk(KERN_ERR "The filesystem that you try to mount is not of type assoofs, MagicNumber mismach");
		brelse(bh);
		return -EPERM;
	}
	if(assoofs_sb->block_size!=ASSOOFS_DEFAULT_BLOCK_SIZE){
		printk(KERN_ERR "assoofs seem to be formatted using a wrong block size.");
		brelse(bh);
		return -EPERM;
	}
	printk(KERN_INFO "assoofs filesystem of version %llu formatted with block size of %llu detected in the device.\n", assoofs_sb->version, assoofs_sb->block_size);

	/*asignar toda la informacion al superbloque*/
	sb->s_magic=ASSOOFS_MAGIC;
	sb->s_fs_info=assoofs_sb;
	sb->s_maxbytes=ASSOOFS_DEFAULT_BLOCK_SIZE;
	sb->s_op=&assoofs_ops;
	root_inode=new_inode(sb);
	inode_init_owner(root_inode,NULL,S_IFDIR);
	root_inode->i_ino=ASSOOFS_ROOTDIR_INODE_NUMBER;
	root_inode->i_sb=sb;
	root_inode->i_op = &assoofs_inode_ops;
	root_inode->i_atime=current_time(root_inode);
	root_inode->i_mtime=current_time(root_inode);
	root_inode->i_ctime=current_time(root_inode);
	root_inode->i_private = assoofs_get_inode_info(sb , ASSOOFS_ROOTDIR_INODE_NUMBER);

	sb->s_root = d_make_root(root_inode);

	return 0;

}

static struct dentry *assoofs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data){
	struct dentry *ret;
	ret=mount_bdev(fs_type,flags,dev_name, data, assoofs_fill_super);

	if(IS_ERR(ret))
		printk(KERN_ERR "Error mounting assoofs.");
	else
		printk(KERN_INFO "assoofs is sucessfully mounted on %d\n", dev_name);

	return ret;
}

static int __init assoofs_init(void){
	int ret;
	/*assoofs_inode_cache=kmem_cache_create("assoofs_inode_cache", sizeof(struct assoofs_inode_info), 0, (SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD), NULL);
	if(!assoofs_inode_cache)
		return -ENOMEM;*/
	ret=register_filesystem(&assoofs_type);
	if(ret==0)
		printk(KERN_INFO "Sucessfully registered assoofs\n");
	else
		printk(KERN_ERR "Failed to register assoofs Error %d", ret);
	return ret;
}

static void __exit assoofs_exit(void){
	int ret;
	ret=unregister_filesystem(&assoofs_type);
	kmem_cache_destroy(assoofs_inode_cache);
	if(ret==0)
		printk(KERN_INFO "Sucessfully unregistered assoofs\n");
	else
		printk(KERN_ERR "Failed to unregister assoofs, Error %d", ret);
}



module_init(assoofs_init);
module_exit(assoofs_exit);



