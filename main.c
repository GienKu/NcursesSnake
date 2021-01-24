#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "snake.h"
int main()
{
    srand(time(NULL));
	initscr();
    start_color();
    cbreak();
    noecho();
    main_menu();
	getch();
	endwin();
}