#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "snake.h"

#define BG_COLOR 40
#define SNAKE_COLOR 22

//atrybuty,kolory,rysowanie planszy
void draw_board(WINDOW * plansza,WINDOW * wynik)
{
	start_color();
	init_pair(1,COLOR_YELLOW,BG_COLOR);  //krawedzie
    init_pair(2,SNAKE_COLOR,BG_COLOR);   //snake
    init_pair(5,154,17);

    wbkgd(wynik,COLOR_PAIR(3)); //bg color
    //SCORE
    box(wynik,0,0);
    mvwprintw(wynik,0,1,"Score");
    //Plansza
    wbkgd(plansza,COLOR_PAIR(2)); //bg color
	wattron(plansza,COLOR_PAIR(1));
	box(plansza,ACS_VLINE,ACS_HLINE);
	printw("Test snake");
	wattroff(plansza,COLOR_PAIR(1));
    
	refresh();
	wrefresh(plansza);
    wrefresh(wynik);
}
// odzywianie snake'a
void create_food(WINDOW *plansza)
{
    init_pair(4,160,BG_COLOR);
    int xRand = rand()%64;
    int yRand = rand()%24;
    char ch;
    while(xRand == 0 || xRand%2 != 0 ||
          yRand == 0 || yRand%2 != 0 ||
         ((ch = mvwinch(plansza,yRand,xRand))) == 'o')
    {
            xRand = rand()%64;
            yRand = rand()%24;
    }

    mvwaddch(plansza,yRand,xRand,'@' | COLOR_PAIR(4));
    wrefresh(plansza);
}
void snake_length_change(Player ** snake,int length)
{
    *snake = realloc(*snake, (length+1) * sizeof(Player));
}
bool is_food(WINDOW *plansza,Player *new_head)
{
    int y = (new_head)->y;
    int x = (new_head)->x;
    char ch = mvwinch(plansza,y,x);
    if(ch == '@')return true;
    return false;
}
//sprawdzenie czy snake nie uderza w ściane
bool collision(int yp, int xp,WINDOW * plansza)
{
    char test = mvwinch(plansza,yp,xp);
    if(test == 'x' || test == 'q' || test == 'o')return false;
    return true;
}
//przesunięcie wspołrzędnych każdej części snake'a
//usnięcie ostatniego elementu ogona i dodanie nowej głowy
void tail_change(Player *snake_ptr,Player *new_head,WINDOW * plansza,int s_len)
{
    int yNew = (new_head)->y;
    int xNew = (new_head)->x;
    Player temp1,temp2 = snake_ptr[0];
    for( int i = 1; i < s_len+1; i++)
    {
        temp1 = snake_ptr[i];
        snake_ptr[i] = temp2;
        temp2 = temp1;
    }
    snake_ptr[0].y = yNew;
    snake_ptr[0].x = xNew;
    
    init_pair(10,SNAKE_COLOR,BG_COLOR);
    mvwaddch(plansza,snake_ptr[0].y, snake_ptr[0].x,'o' | COLOR_PAIR(10));
    mvwaddch(plansza,snake_ptr[s_len].y, snake_ptr[s_len].x,' ');
    wrefresh(plansza);
}
// zmiana stanu ściany na czerwony po uderzeniu 
void mark_wall(WINDOW * plansza,Player *new_head)
{
    int y = (new_head)->y;
    int x = (new_head)->x;
    init_pair(3,COLOR_RED,BG_COLOR);
    mvwaddch(plansza,y,x,'x' | COLOR_PAIR(3));
}
//sprawdzenie kierunku poruszania sie snake'a
bool which_direction(Player **snake_ptr, char action,WINDOW *plansza,Player *new_head)
{
    int yNext = (*snake_ptr)->y;
    int xNext = (*snake_ptr)->x;
    switch (action)
    {
        case 'w':
        case 'W':
        case (char)KEY_UP:
            new_head->y = yNext-1;
            new_head->x = xNext;
            if(collision(yNext-1,xNext,plansza))return true;
            else return false;
            break;
        case 'a':
        case 'A':
        case (char)KEY_LEFT:
            new_head->y = yNext;
            new_head->x = xNext-2;
            if (collision(yNext,xNext-2,plansza))return true;
            else return false;
            break;
        case 's':
        case 'S':
        case (char)KEY_DOWN:
            new_head->y = yNext+1;
            new_head->x = xNext;
            if (collision(yNext+1,xNext,plansza))return true;
            else return false;
            break;
        case 'd':
        case 'D':
        case (char)KEY_RIGHT:
            new_head->y = yNext;
            new_head->x = xNext+2;
            if (collision(yNext,xNext+2,plansza))return true;
            else return false;
            
            break;
        default:
            return true;
    }
}
bool is_good_move(char act_mv,char next_mv)
{
    switch (next_mv)
    {
        case 'w':
        case 'W':
        case (char)KEY_UP:
            if(act_mv == 's' || act_mv == 'S' || act_mv == (char)KEY_DOWN)
                return false;
            break;
        case 'a':
        case 'A':
        case (char)KEY_LEFT:
            if(act_mv == 'd' || act_mv == 'D' || act_mv == (char)KEY_RIGHT)
                return false;
            break;
        case 's':
        case 'S':
        case (char)KEY_DOWN:
            if(act_mv == 'w' || act_mv == 'W' || act_mv == (char)KEY_UP)
                return false;
            break;
        case 'd':
        case 'D':
        case (char)KEY_RIGHT:
            if(act_mv == 'a' || act_mv == 'd' || act_mv == (char)KEY_LEFT)
                return false;
            break;
    }
    return true;
}
void main_game_loop()
{
    WINDOW *score = newwin(3,7,LINES/2-15,COLS/2-32);
    WINDOW *plansza = newwin(25,65,LINES/2-12,COLS/2-32);
    Player *snake_parts = malloc( 4 * sizeof(Player));
    Player new_head; 

    int yPl,xPl,snake_length = 3,scr = 0;
    bool is_alive = true;
    char actual_move = 'd',next_move;

    getmaxyx(plansza,yPl,xPl);
    for( int i = 0; i < 3; i++)
    {
        snake_parts[i].y = yPl/2;
        snake_parts[i].x = xPl/2+8-2*i;
        mvwaddch(plansza,snake_parts[i].y,snake_parts[i].x,'o');
    }
 
    draw_board(plansza,score);
    create_food(plansza);
    getch();
    keypad(plansza,true);
    wtimeout(plansza,100);

    while(is_alive)
    {
        if((next_move = wgetch(plansza))  == ERR)
            ;
        else if(is_good_move(actual_move,next_move))
            actual_move = next_move;
        if(which_direction(&snake_parts,actual_move,plansza,&new_head))
        {
            if(is_food(plansza,&new_head))
            {
                snake_length += 1;
                scr += 1;

                mvprintw(LINES/2-14,COLS/2-31,"%i",scr);
                refresh();

                snake_length_change(&snake_parts,snake_length);
                create_food(plansza);
            }
            tail_change(snake_parts,&new_head,plansza,snake_length);
        }
        else mark_wall(plansza,&new_head);           
    }
}
