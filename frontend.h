#include <ncurses.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

extern int EXIT;

extern char *s_choices[];

void choiceMenu(void);
void howToPlay(void);
void highScores(void);
void exitGame(void);
void playGame(enum Option option, WINDOW *main_window);
void printInMiddle(WINDOW *win, int start_y, int start_x, int width, char *string);
void displayBoard(Board *board, WINDOW *win);
enum Directions getDirection(enum Directions current_direction);
WINDOW *createNewWindow(int height, int width, int start_y, int start_x);
