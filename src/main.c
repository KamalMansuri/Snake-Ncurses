#include "backend.h"
#include "frontend.h"
#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>

char *f_choices[] = {
  "Play the game",
  "How to play",
  "High Scores",
  "Exit",
  (char *)NULL,
};

int main(int argc, char *argv[]) {
  ITEM **f_items;
  int c;
  MENU *main_menu;
  int n_choices, i;
  WINDOW *main_menu_win;

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(FALSE);


  /* Main menu */
  /* Create items */
  n_choices = ARRAY_SIZE(f_choices);
  f_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
  for(i = 0; i < n_choices; ++i)
      f_items[i] = new_item(f_choices[i], "");
  set_item_userptr(f_items[0], choiceMenu);
  set_item_userptr(f_items[1], howToPlay);
  set_item_userptr(f_items[2], highScores);
  set_item_userptr(f_items[3], exitGame);

  /* Crate menu */
  main_menu = new_menu((ITEM **)f_items);

  /* Set menu option to not show description */
  menu_opts_off(main_menu, O_SHOWDESC);

  /* Create the window to be associated with the menu */
  main_menu_win = createNewWindow(8, 20, (LINES - 8) /2, (COLS - 20) /2);
  keypad(main_menu_win, TRUE);
  
  /* Set main window and sub window */
  set_menu_win(main_menu, main_menu_win);
  set_menu_sub(main_menu, derwin(main_menu_win, 4, 18, 3, 1));

  /* Set menu mark to the string " * " */
  set_menu_mark(main_menu, " * ");

  
  while(!EXIT) {
    /* Display menu */
    post_menu(main_menu); 
    wrefresh(main_menu_win); 
    box(main_menu_win, 0, 0);
    printInMiddle(main_menu_win, 1, 0, 20, "Main Menu");
    mvwaddch(main_menu_win, 2, 0, ACS_LTEE);
    mvwhline(main_menu_win, 2, 1, ACS_HLINE, 18);
    mvwaddch(main_menu_win, 2, 19, ACS_RTEE);


    c = wgetch(main_menu_win);
    switch(c) {
      case KEY_DOWN:
        menu_driver(main_menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(main_menu, REQ_UP_ITEM);
        break;
      case 10: {  /* Enter */
        ITEM *cur;
        void (*p)(void);

        unpost_menu(main_menu);
        wrefresh(main_menu_win);

        cur = current_item(main_menu);
        p = item_userptr(cur);
        p();
        pos_menu_cursor(main_menu);
        break;
      }

    }
    wrefresh(main_menu_win);
  }

  /* Free up the memory */
  unpost_menu(main_menu); 
  free_menu(main_menu); 
  for(i = 0; i < n_choices; ++i)
    free_item(f_items[i]);
  endwin();
  return 0;
}
