#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "snake.h"
int main(int argc, char ** argv)
{
    srand(time(NULL));
    initscr();
    curs_set(false);
    start_color();
    cbreak();
    noecho();
    main_menu();
    getch();
	endwin();
}