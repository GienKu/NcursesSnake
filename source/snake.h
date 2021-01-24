#include<ncurses.h>
#ifndef snake_test2
#define snake_test2

typedef struct Player
{
    int x;
    int y;
}Player;
void draw_board(WINDOW * plansza,WINDOW * wynik);

void create_food(WINDOW *plansza);

void snake_length_change(Player ** snake,int length);

bool is_food(WINDOW *plansza,Player *new_head);

bool collision(int yp, int xp,WINDOW * plansza);

void tail_change(Player *snake_ptr,Player *new_head,WINDOW * plansza,int s_len);

void mark_wall(WINDOW * plansza,Player *new_head);
//movement
bool which_direction(Player **snake_ptr, char action,WINDOW *plansza,Player *new_head);

bool is_good_move(char act_mv,char next_mv);

void main_game_loop(int level);

void main_menu();

void game_over(WINDOW *plansza, WINDOW *score);
#endif
