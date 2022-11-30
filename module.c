#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define MODULE_VERS "0.1"
#define MODULE_NAME "2048"
#define PROCFS_NAME "play_2048"

#include "game.h"

ssize_t eleven_proc_read(struct file *file, const char __user *buff, size_t count, loff_t *ppos);
ssize_t eleven_proc_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos);
static int eleven_proc_open(struct inode *inode, struct file *file);
static int eleven_proc_show(struct seq_file *m, void *v);

static struct proc_dir_entry *eleven_proc_file;
u16 board[BOARD_SIZE][BOARD_SIZE];
struct eleven_mutex {
	atomic_t		count;
	spinlock_t		wait_lock;
	struct list_head	wait_list;
};

DEFINE_MUTEX(eleven_mutex);

static const struct proc_ops proc_fops = {
	.proc_open = eleven_proc_open,
	.proc_write = eleven_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};


static int eleven_proc_show(struct seq_file *m, void *v) {
	mutex_lock(&eleven_mutex);
	draw_board(m, board);
	if (game_ended(board)) {
		seq_printf(m, "         GAME OVER          \n");
		printk(KERN_ALERT "%s game ended!\n", MODULE_NAME);
	}
	mutex_unlock(&eleven_mutex);
	return 0;
}

static int eleven_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, eleven_proc_show, NULL);
}

ssize_t eleven_proc_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos)
{
	char key;
	if (!count) {
      return 0;
    }

	if (copy_from_user(&key, buff, sizeof(key)))
		return -EFAULT;
	mutex_lock(&eleven_mutex);
	handle_key(board, key);
	mutex_unlock(&eleven_mutex);
	return count;
}

static int __init eleven_init(void)
{
	reset_game(board);

	eleven_proc_file = proc_create(PROCFS_NAME, 0, NULL, &proc_fops);
	if (eleven_proc_file == NULL) {
#ifdef DEBUG
		printk(KERN_ALERT "%s error: could not create temporary /proc entry\n",
				MODULE_NAME);
#endif
		return -1;
	}
	return 0;
}

static void __exit eleven_cleanup(void)
{
	if (eleven_proc_file)
		proc_remove(eleven_proc_file);
}

module_init(eleven_init);
module_exit(eleven_cleanup);

MODULE_AUTHOR("Jörg Thalheim <joerg@higgsboson.tk");
MODULE_DESCRIPTION("play 2048 in linux kernel");
MODULE_LICENSE("GPL");
MODULE_VERSION(MODULE_VERS);

/* ex: set tabstop=4 shiftwidth=4 noexpandtab: */
