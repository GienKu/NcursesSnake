#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "snake.h"
int main()
{
    srand(time(NULL));
	initscr();
    cbreak();
    noecho();
    main_game_loop();
	getch();
	endwin();
}