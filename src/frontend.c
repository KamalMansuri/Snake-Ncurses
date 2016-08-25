#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include "backend.h"
#include <panel.h>
#include <menu.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int EXIT = FALSE;

char *s_choices[] = {
  "OPEN",
  "CLOSED",
  "Back",
  (char *)NULL,
};

/* Function taken from the ncurses HOWTO page */
void printInMiddle(WINDOW *win, int start_y, int start_x, int width, char *string) {
  int length, x, y;
  float temp;

  
  if(win == NULL)
    win = stdscr;
  getyx(win, y, x);
  if(start_x != 0)
    x = start_x;
  if(start_y != 0)
    y = start_y;
  if(width == 0)
    width = 80;

  length = strlen(string);
  temp = (width - length)/ 2;
  x = start_x + (int)temp;
  mvwprintw(win, y, x, "%s", string);
  refresh();
}

void exitGame(void) {
  EXIT = TRUE;
  return;
}

void displayBoard(Board *board, WINDOW *win) {
	struct LinkedItem *head = board -> snake;


  /* Display the food */
  mvwaddch(win, board -> food -> y, board -> food -> x, board -> food -> symbol);
  if( board -> bonus_food != NULL) {
    mvwaddch(win, board -> bonus_food -> y, board -> bonus_food -> x, board -> bonus_food -> symbol);
  }

  /* Display the snake */
  while(head) {
    mvwaddch(win, head -> y, head -> x, head -> symbol);
    head = head -> next;
  }

  /* Display the border for the arena */
  box(win, 0 , 0); 
}

enum Directions getDirection(enum Directions current_direction) {
  int d = getch();

  switch(d) {
    case 'w':
      return UP;
    case KEY_UP:
      return UP;
    case 's':
      return DOWN;
    case KEY_DOWN:
      return DOWN;
    case 'a':
      return LEFT;
    case KEY_LEFT:
      return LEFT;
    case 'd':
      return RIGHT;
    case KEY_RIGHT:
      return RIGHT; 
    case 'p':
      return HOLD;
    default:
      return current_direction;   /* If no direction given then go in the same direction */
  }
}

WINDOW *createNewWindow(int height, int width, int start_y, int start_x) {
  WINDOW *local_win;

  local_win = newwin(height, width, start_y, start_x);

  return local_win;
}


void playGame(enum Option option, WINDOW *main_window) {
  int start_x, start_y, height, width;
  WINDOW *game_win;
  height = 20; width = 50;

  /* Get the middle of the sreen */ 
  start_y = (LINES - height) / 2;  
  start_x = (COLS - width) / 2;

  game_win = createNewWindow(height, width, start_y, start_x);
  Board *board = createBoard(width, height, option);
  enum Status status = GAME_ON;

  while(status == GAME_ON) {
    werase(game_win);

    /* Print info about game above the arena */
    switch(option) {
      case OPEN:
        mvwprintw(main_window ,start_y - 1, start_x, "Game mode: OPEN");
        break;
      case CLOSED:
        mvwprintw(main_window ,start_y - 1, start_x, "Game mode: CLOSED");
        break;
    }
    mvwprintw(main_window ,start_y - 2, start_x, "Your score: %ld", board -> score);
    
    wrefresh(main_window);

    if(board -> snake -> direction == RIGHT || board -> snake -> direction == LEFT) 
      timeout(100);
    else  
      timeout(125);

    displayBoard(board, game_win);
    wrefresh(game_win);
    board -> snake -> direction = getDirection(board -> snake -> direction);

    if(board -> snake -> direction == HOLD) {
      mvwprintw(main_window ,start_y - 2, start_x + 32, " Game is paused... ");
      continue;
    } else {
      mvwprintw(main_window ,start_y - 2, start_x + 32, "Press 'p' to pause ");
    }

    /* Move the snake */ 
    status = moveSnake(board);
  }
  writeScoreToFile(board);
  /* Clear the window */
  destroyBoard(board); wclear(game_win); wrefresh(game_win); 
  delwin(game_win); /* Deallocate the memory for the game window */
}

void choiceMenu(void) {
  ITEM **s_items;
  ITEM *cur;
  MENU *options_menu;
  int n_choices, i, ch, exit_con;
  WINDOW *options_menu_win;
  enum Option open, closed;
  open = OPEN; closed = CLOSED;
  int p = -1;

  exit_con = FALSE;

  /* Second menu */
  /* Create Items */
  n_choices = ARRAY_SIZE(s_choices);
  s_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
  for(i = 0; i < n_choices; ++i)
      s_items[i] = new_item(s_choices[i], "");
  set_item_userptr(s_items[0], &open);
  set_item_userptr(s_items[1], &closed);
  set_item_userptr(s_items[2], &p);

  /* Create menu */
  options_menu = new_menu((ITEM **)s_items);

  /* Set menu option to not show description */
  menu_opts_off(options_menu, O_SHOWDESC);

  /* Create the window to be associated with the menu */
  options_menu_win = createNewWindow(8, 20, (LINES - 8) /2, (COLS - 20) /2);
  keypad(options_menu_win, TRUE);
     
  /* Set main window and sub window */
  set_menu_win(options_menu, options_menu_win);
  set_menu_sub(options_menu, derwin(options_menu_win, 4, 15, 3, 4));

  /* Set menu mark to the string " * " */
  set_menu_mark(options_menu, " * ");

  while( !exit_con ) {
    /* Display the options menu */
    post_menu(options_menu);
    wrefresh(options_menu_win);
    box(options_menu_win, 0, 0);
    printInMiddle(options_menu_win, 1, 0, 20, "Pick an option:");
    mvwaddch(options_menu_win, 2, 0, ACS_LTEE);
    mvwhline(options_menu_win, 2, 1, ACS_HLINE, 18);
    mvwaddch(options_menu_win, 2, 19, ACS_RTEE);

    ch = wgetch(options_menu_win);
    switch(ch) {
      case KEY_DOWN:
        menu_driver(options_menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(options_menu, REQ_UP_ITEM);
        break;
      case 10: {  /* Enter */
        cur = current_item(options_menu);
        int *option = item_userptr(cur);
        if(*option == -1) {
          exit_con = TRUE;
          break;
        }
        
        unpost_menu(options_menu);
        wrefresh(options_menu_win);
        WINDOW *full_screen;
        int x, y;
        getmaxyx(stdscr, y, x);
        full_screen = createNewWindow(y, x, 0, 0);
        playGame(*option, full_screen);
        wclear(full_screen);
        wrefresh(full_screen);

        break;
      }
    }
  }

  /* Menu no longer used so free memory */
  unpost_menu(options_menu);
  free_menu(options_menu); 
  for(i = 0; i < n_choices; ++i)
    free_item(s_items[i]);
  wrefresh(options_menu_win);
  delwin(options_menu_win);
  return;
}

void howToPlay(void) {
  WINDOW *controls_win;  
  int ch;

  controls_win = createNewWindow(10, 30, (LINES - 9) /2, (COLS - 30) /2);
  keypad(controls_win, TRUE);

  box(controls_win, 0, 0);
  printInMiddle(controls_win, 1, 0, 30, "How to Play");
  mvwaddch(controls_win, 2, 0, ACS_LTEE);
  mvwhline(controls_win, 2, 1, ACS_HLINE, 28);
  mvwaddch(controls_win, 2, 29, ACS_RTEE);  

  mvwprintw(controls_win, 3, 1, "Move with wasd or arrows.");
  mvwprintw(controls_win, 4, 1, "Eat the food to grow.");
  mvwprintw(controls_win, 5, 1, "Food is '#' and bonus food");
  mvwprintw(controls_win, 6, 1, "is '$'.");

  mvwaddch(controls_win, 7, 0, ACS_LTEE);
  mvwhline(controls_win, 7, 1, ACS_HLINE, 28);
  mvwaddch(controls_win, 7, 29, ACS_RTEE);
  wattron(controls_win , A_STANDOUT);
  mvwprintw(controls_win, 8, 11, "* Back");
  wattroff(controls_win ,A_STANDOUT);

  wrefresh(controls_win);

  while( (ch = wgetch(controls_win)) != 10 ) {

  }

  wclear(controls_win);
  wrefresh(controls_win);
  delwin(controls_win);
  return;
}

void highScores(void) {
  FILE *fp_open, *fp_closed;
  fp_open = fopen("high-scores-open.txt", "r");
  fp_closed = fopen("high-scores-closed.txt", "r");

  char *ptr, str[70];
  int open_scores[3], closed_scores[3], i;

  /* Store high scores for each mode into an array */

  for(i = 0; i < 3; i++) {
    fgets(str, 70, fp_open);
    open_scores[i] = strtol(str, &ptr, 10);
    fgets(str, 70, fp_closed);
    closed_scores[i] = strtol(str, &ptr, 10);
  }

  /* Files not needed anymore so close */
  fclose(fp_open); fclose(fp_closed);

  WINDOW *high_scores_win;  
  int ch;

  high_scores_win = createNewWindow(11, 20, (LINES - 11) /2, (COLS - 20) /2);
  keypad(high_scores_win, TRUE);

  /* Draw the window */ 
  box(high_scores_win, 0, 0);
  printInMiddle(high_scores_win, 1, 0, 20, "High Scores");
  mvwaddch(high_scores_win, 2, 0, ACS_LTEE);
  mvwhline(high_scores_win, 2, 1, ACS_HLINE, 18);
  mvwaddch(high_scores_win, 2, 19, ACS_RTEE);

  mvwprintw(high_scores_win, 3, 1, "CLOSED:");
  mvwprintw(high_scores_win, 3, 11, "OPEN:");

  for(i = 3; i < 9; i++) {
    mvwaddch(high_scores_win, i, 9, ACS_VLINE);
    mvwaddch(high_scores_win, i, 10, ACS_VLINE);
  }

  mvwaddch(high_scores_win, 4, 0, ACS_LTEE);
  mvwhline(high_scores_win, 4, 1, ACS_HLINE, 18);
  mvwaddch(high_scores_win, 4, 19, ACS_RTEE);

  mvwaddch(high_scores_win, 2, 9, ACS_TTEE);
  mvwaddch(high_scores_win, 2, 10, ACS_TTEE);

  mvwaddch(high_scores_win, 4, 9, ACS_PLUS);
  mvwaddch(high_scores_win, 4, 10, ACS_PLUS);

  mvwaddch(high_scores_win, 8, 9, ACS_BTEE);
  mvwaddch(high_scores_win, 8, 10, ACS_BTEE);

  mvwaddch(high_scores_win, 8, 0, ACS_LTEE);
  mvwhline(high_scores_win, 8, 1, ACS_HLINE, 18);
  mvwaddch(high_scores_win, 8, 19, ACS_RTEE);

  mvwaddch(high_scores_win, 8, 9, ACS_BTEE);
  mvwaddch(high_scores_win, 8, 10, ACS_BTEE);

  wattron(high_scores_win , A_STANDOUT);
  mvwprintw(high_scores_win, 9, 7, "* Back");
  wattroff(high_scores_win ,A_STANDOUT);

  for(i = 0; i < 3; i++ ) {
    mvwprintw(high_scores_win, 5 + i, 12, "%d", open_scores[i]);
    mvwprintw(high_scores_win, 5 + i, 2, "%d", closed_scores[i]);
  }

  wrefresh(high_scores_win);

  while( (ch = wgetch(high_scores_win)) != 10 ) {

  }

  wclear(high_scores_win);
  wrefresh(high_scores_win);
  delwin(high_scores_win);
  return;
}