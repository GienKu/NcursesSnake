
#ifndef snake_
#define snake_
#include<ncurses.h>

typedef struct Coords
{
    int x;
    int y;
}Coords;
void draw_board(WINDOW *plansza,WINDOW *wynik, WINDOW *highscore, int best);

void create_food(WINDOW *plansza);

void snake_length_change(Coords **snake,int length);

bool is_food(WINDOW *plansza,Coords *new_head);

bool collision(int yp, int xp,WINDOW *plansza);

void tail_change(Coords *snake_ptr,Coords *new_head,WINDOW *plansza,int s_len);

void mark_wall(WINDOW *plansza,Coords *new_head);

bool which_direction(Coords **snake_ptr, int action,WINDOW *plansza,Coords *new_head);

bool is_good_move(int act_mv,int next_mv);

void main_game_loop(int level);

void main_menu();

void game_over(WINDOW *plansza, WINDOW *score);
#endif
