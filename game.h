
#define BOARD_SIZE (4)

void draw_board(struct seq_file *f, u16 board[BOARD_SIZE][BOARD_SIZE]);

bool game_ended(u16 board[BOARD_SIZE][BOARD_SIZE]);

void handle_key(u16 board[BOARD_SIZE][BOARD_SIZE], char key);

void reset_game(u16 board[BOARD_SIZE][BOARD_SIZE]);

