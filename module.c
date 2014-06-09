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
static u16 board[BOARD_SIZE][BOARD_SIZE];

static const struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.open = eleven_proc_open,
	.write = eleven_proc_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int eleven_proc_show(struct seq_file *m, void *v) {
	draw_board(m, board);
	if (game_ended(board))
		seq_printf(m, "         GAME OVER          \n");
	return 0;
}

static int eleven_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, eleven_proc_show, NULL);
}

ssize_t eleven_proc_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos)
{
	if (count >= 1) {
		handle_key(board, buff[0]);
	}
	return count;
}

static int __init eleven_init(void)
{
	memset(board, 0, sizeof(board));
	add_random(board);
	add_random(board);

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
