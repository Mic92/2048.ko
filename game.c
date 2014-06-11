/*
 * The original game was ported from the console version of Maurits van der Schee:
 * - source: https://github.com/mevdschee/2048.c
 * - license: https://github.com/mevdschee/2048.c/blob/master/LICENSE
 */
#include <linux/kernel.h>
#include <asm-generic/uaccess.h>
#include <linux/random.h>

#include "game.h"

static u32 score = 0;

static void get_color(u16 value, char *color, int length);
static s8 find_target(u16 array[BOARD_SIZE], s8 x, s8 stop);
static bool slide_array(u16 array[BOARD_SIZE]);
static void rotate_board(u16 board[BOARD_SIZE][BOARD_SIZE]);
static bool move_up(u16 board[BOARD_SIZE][BOARD_SIZE]);
static bool move_left(u16 board[BOARD_SIZE][BOARD_SIZE]);
static bool move_down(u16 board[BOARD_SIZE][BOARD_SIZE]);
static bool move_right(u16 board[BOARD_SIZE][BOARD_SIZE]);
static bool find_pair_down(u16 board[BOARD_SIZE][BOARD_SIZE]);
static s16 count_empty(u16 board[BOARD_SIZE][BOARD_SIZE]);

void draw_board(struct seq_file *f, u16 board[BOARD_SIZE][BOARD_SIZE])
{
	u8 x,y,t;
	char color[40], reset[] = "\033[m";
	seq_printf(f, "\033[H");

	seq_printf(f, "2048.ko %17d pts\n\n",score);

	for (y = 0; y < BOARD_SIZE; y++) {
		for (x = 0; x < BOARD_SIZE; x++) {
			get_color(board[x][y], color, 40);
			seq_printf(f, "%s", color);
			seq_printf(f, "       ");
			seq_printf(f, "%s", reset);
		}
		seq_printf(f, "\n");
		for (x = 0; x < BOARD_SIZE; x++) {
			get_color(board[x][y], color, 40);
			seq_printf(f, "%s",color);
			if (board[x][y]!=0) {
				char s[8];
				snprintf(s, 8, "%u", board[x][y]);
				t = 7 - strlen_user(s);
				seq_printf(f, "%*s%s%*s", t - t / 2, "" , s , t/2 , "");
			} else {
				seq_printf(f, "   ·   ");
			}
			seq_printf(f, "%s", reset);
		}
		seq_printf(f, "\n");
		for (x = 0; x < BOARD_SIZE; x++) {
			get_color(board[x][y], color, 40);
			seq_printf(f, "%s", color);
			seq_printf(f, "       ");
			seq_printf(f, "%s", reset);
		}
		seq_printf(f, "\n");
	}
	seq_printf(f, "\n");
	seq_printf(f, "        ←,↑,→,↓ or q        \n");
	seq_printf(f, "\033[A");
}

void add_random(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	s8 x, y;
	s16 r, len= 0;
	u16 n, list[BOARD_SIZE * BOARD_SIZE][2];

	for (x = 0; x < BOARD_SIZE; x++) {
		for (y = 0; y < BOARD_SIZE; y++) {
			if (board[x][y] == 0) {
				list[len][0] = x;
				list[len][1] = y;
				len++;
			}
		}
	}

	if (len > 0) {
		r = get_random_int() % len;
		x = list[r][0];
		y = list[r][1];
		n = ((get_random_int() % 10) / 9 + 1) * 2;
		board[x][y] = n;
	}
}

bool game_ended(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	bool ended = true;
	if (count_empty(board) > 0)
		return false;
	if (find_pair_down(board))
		return false;
	rotate_board(board);
	if (find_pair_down(board))
		ended = false;
	rotate_board(board);
	rotate_board(board);
	rotate_board(board);
	return ended;
}

void handle_key(u16 board[BOARD_SIZE][BOARD_SIZE], char key)
{
	bool success = false;

	if (key == 'r' || key == 'q') {
		reset_game(board);
		return;
	}
	if (game_ended(board)) {
		return;
	}

	switch(key) {
	case 97:	/* 'a' */
	case 104:	/* 'h' */
	case 68:	/* left arrow */
		success = move_left(board);  break;
	case 100:	/* 'd' */
	case 108:	/* 'l' */
	case 67:	/* right arrow */
		success = move_right(board); break;
	case 119:	/* 'w' */
	case 107:	/* 'k' */
	case 65:	/* up arrow */
		success = move_up(board);    break;
	case 115:	/* 's' */
	case 106:	/* 'j' */
	case 66:	/* down arrow */
		success = move_down(board);  break;
	}
	if (success)
		add_random(board);
}

void reset_game(u16 board[BOARD_SIZE][BOARD_SIZE]) {
	score = 0;
	memset(board, 0, sizeof(u16) * BOARD_SIZE * BOARD_SIZE);
	add_random(board);
	add_random(board);
}

static void get_color(u16 value, char *color, int length)
{
	u8 original[] = {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0};
	/*u8 blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0}*/
	/*u8 bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};*/
	u8 *scheme = original;
	u8 *background = scheme + 0;
	u8 *foreground = scheme + 1;
	if (value > 0) while (value >>= 1) {
		if (background + 2 < scheme + sizeof(original)) {
			background += 2;
			foreground += 2;
		}
	}
	snprintf(color,length, "\033[38;5;%d;48;5;%dm", *foreground, *background);
}

static s8 find_target(u16 array[BOARD_SIZE], s8 x, s8 stop)
{
	s8 t;
	/* if the position is already on the first, don't evaluate */
	if (x == 0)
		return x;

	for(t = x - 1; t >= 0; t--) {
		if (array[t] != 0) {
			if (array[t] != array[x])
				/* merge is not possible, take next position */
				return t+1;

			return t;
		} else {
			/* we should not slide further, return this one */
			if (t == stop)
				return t;
		}
	}
	// we did not find a
	return x;
}

static bool slide_array(u16 array[BOARD_SIZE])
{
	bool success = false;
	u8 x, t, stop = 0;

	for (x = 0; x < BOARD_SIZE; x++) {
		if (array[x] != 0) {
			t = find_target(array, x, stop);
			/* if target is not original position, then move or merge */
			if (t != x) {
				/* if target is not zero, set stop to avoid double merge */
				if (array[t] != 0) {
					score += array[t] + array[x];
					stop = t + 1;
				}
				array[t] += array[x];
				array[x] = 0;
				success = true;
			}
		}
	}
	return success;
}

static void rotate_board(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	s8 i, j, n = BOARD_SIZE;
	s16 tmp;
	for (i = 0; i < n / 2; i++) {
		for (j = i; j < n - i - 1; j++){
			tmp = board[i][j];
			board[i][j] = board[j][n - i - 1];
			board[j][n - i - 1] = board[n - i - 1][n - j - 1];
			board[n - i - 1][n - j - 1] = board[n - j - 1][i];
			board[n - j - 1][i] = tmp;
		}
	}
}

static bool move_up(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	bool success = false;
	s8 x;
	for (x = 0; x < BOARD_SIZE; x++)
		success |= slide_array(board[x]);
	return success;
}

static bool move_left(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	bool success;
	rotate_board(board);
	success = move_up(board);
	rotate_board(board);
	rotate_board(board);
	rotate_board(board);
	return success;
}

static bool move_down(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	bool success;
	rotate_board(board);
	rotate_board(board);
	success = move_up(board);
	rotate_board(board);
	rotate_board(board);
	return success;
}

static bool move_right(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	bool success;
	rotate_board(board);
	rotate_board(board);
	rotate_board(board);
	success = move_up(board);
	rotate_board(board);
	return success;
}

static bool find_pair_down(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	s8 x, y;
	for (x = 0; x < BOARD_SIZE; x++)
		for (y = 0; y < BOARD_SIZE - 1; y++)
			if (board[x][y]==board[x][y + 1])
				return true;
	return false;
}

static s16 count_empty(u16 board[BOARD_SIZE][BOARD_SIZE])
{
	s8 x, y;
	s16 count = 0;
	for (x = 0; x < BOARD_SIZE; x++)
		for (y = 0; y < BOARD_SIZE; y++)
			if (board[x][y] == 0)
				count++;
	return count;
}

/* ex: set tabstop=4 shiftwidth=4 noexpandtab: */
