#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "snake.h"

#define BG_COLOR 40
#define SECOND_COLOR 38
#define SNAKE_COLOR 22

//atrybuty,kolory,rysowanie planszy
void draw_board(WINDOW * plansza,WINDOW * wynik)
{
	start_color();
	init_pair(1,COLOR_YELLOW,BG_COLOR);   //krawedzie
    init_pair(2,SNAKE_COLOR,BG_COLOR);    //snake
    init_pair(5,154,SECOND_COLOR);                  //score     

    wbkgd(wynik,COLOR_PAIR(5)); //bg color
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
    switch ((int)action)
    {
        case 'w':
        case 'W':
        case KEY_UP:
            new_head->y = yNext-1;
            new_head->x = xNext;
            if(collision(yNext-1,xNext,plansza))return true;
            else return false;
            break;
        case 'a':
        case 'A':
        case KEY_LEFT:
            new_head->y = yNext;
            new_head->x = xNext-2;
            if (collision(yNext,xNext-2,plansza))return true;
            else return false;
            break;
        case 's':
        case 'S':
        case KEY_DOWN:
            new_head->y = yNext+1;
            new_head->x = xNext;
            if (collision(yNext+1,xNext,plansza))return true;
            else return false;
            break;
        case 'd':
        case 'D':
        case KEY_RIGHT:
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
void main_game_loop(int level)
{
    WINDOW *score = newwin(3,7,LINES/2-15,COLS/2-32);
    WINDOW *plansza = newwin(25,65,LINES/2-12,COLS/2-32);
    Player *snake_parts = malloc( 4 * sizeof(Player));
    Player new_head; 

    int yPl,xPl,snake_length = 3,scr = 0;
    bool is_alive = true;
    char actual_move = 'd',next_move;

    mvwprintw(score,1,1,"%i",scr);
    wrefresh(score);

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
    switch (level)
    {
    case 0:
        wtimeout(plansza,125);
        break;
    case 1:
        wtimeout(plansza,100);
        break;
    case 2:
        wtimeout(plansza,75);
        break;
    default:
        wtimeout(plansza,100);
    }

    while(is_alive)
    {
        if((next_move = wgetch(plansza))  == ERR)                       //Sprawdzenie czy wprowadzony został nowy klawisz
            ;
        else if(is_good_move(actual_move,next_move))                    //zmiana aktualnego kierunku jesli nowy nie jest w przeciwny do aktualnego
            actual_move = next_move;
        if(which_direction(&snake_parts,actual_move,plansza,&new_head)) //sprawdzenie kolizji
        {
            if(is_food(plansza,&new_head))                              //sprawdzenie czy dane nowe pole nie jest jedzeniem
            {
                snake_length += 1;                                      
                scr += 1;                                               
                mvwprintw(score,1,1,"%i",scr);                          
                wrefresh(score);                                        

                snake_length_change(&snake_parts,snake_length);         //dynamiczna zmiana wielkosci snake'a
                create_food(plansza);
            }
            tail_change(snake_parts,&new_head,plansza,snake_length);    //zmiana zmiana koordynatów snake'a
        }
        else 
        {
            is_alive = false;
            mark_wall(plansza,&new_head); 
            game_over(plansza,score);
        }          
    }
    main_menu();
}
void main_menu()
{
    WINDOW *MenuContainer = newwin(LINES, COLS, 0, 0);
    WINDOW *MenuPanel = subwin(MenuContainer, 10, 28, LINES/2, COLS/2-8);
    WINDOW *GameOver = subwin(MenuContainer, LINES/2,79,1,COLS/2-39);
    FILE *picture;
    char ch;
    
    init_pair(23, 89, 107);                     //font-roz, backg- zielen
    init_pair(24, 107, 89);                     //na odwrot^
    init_pair(25,COLOR_WHITE,COLOR_BLACK);      //standardowy kolor terminala
    bkgd(COLOR_PAIR(23));                       //bgcolor stdrscr
    wbkgd(GameOver,COLOR_PAIR(23));            //bgcolor GameOver
    wbkgd(MenuPanel,COLOR_PAIR(23));            //bgcolor MenuPanel

    picture = fopen("obrazek.txt","r");
    if(picture == NULL)
    {   wprintw(GameOver,"Error, nie udalo otworzyc sie pliku");
        wrefresh(GameOver);
    }
    wmove(GameOver,0,0);

    // WYSWIETLENIE ASCII ART
    wattron(GameOver,COLOR_PAIR(23) | A_BOLD);
    while((ch = fgetc(picture)) != EOF)
        waddch(GameOver,ch);
    wattroff(GameOver,COLOR_PAIR(23) | A_BOLD);

    fclose(picture);
    refresh();
    wrefresh(MenuPanel);
    wrefresh(GameOver);
    
    // OPCJE MENU
    mvwprintw(MenuPanel,0,1,"Wybierz poziom trudnosci:");
    keypad(MenuPanel,true);
    char choices[4][12] = {
        "Latwy",
        "Sredni",
        "Trudny",
        "Wyjdz z gry"
    };
    bool isPicked = false;
    int highlight = 0;
    int act;
    wmove(MenuPanel,1,1);

    // PETLA WYBORU
    while(!isPicked)
    {
        for(int i = 0; i < 4; i++)
        {
            if(i == highlight)
            {   
                wattron(MenuPanel,COLOR_PAIR(24));
                mvwaddstr(MenuPanel, i+1, 1, choices[i]);
                wattroff(MenuPanel,COLOR_PAIR(24));
            }
            else
                mvwprintw(MenuPanel, i+1, 1, "%s", choices[i]);
        }
        wrefresh(MenuPanel);
        refresh();

        act = wgetch(MenuPanel);
        switch (act)
        {
            case (int)'w':
            case (int)'W':
            case KEY_UP:
                highlight--;
                break;
            case (int)'s':
            case (int)'S':
            case KEY_DOWN:
                highlight++;
                break;
            case 10://enter
                isPicked = true;
                break;
            default:
                continue;
        }
        if(highlight > 3) highlight--;
        if(highlight < 0) highlight++;
    }
    
	wclear(MenuContainer);
	bkgd(COLOR_PAIR(25));
	delwin(MenuContainer);
	clear();
    
    switch (highlight)
    {
        case 0:
            main_game_loop(0);
            break;
        case 1:
            main_game_loop(1);
            break;
        case 2:
            main_game_loop(2);
            break;
        case 3:
            endwin();
            exit(0);
            break;
        default:
            break;
    }
}
void game_over(WINDOW *plansza, WINDOW *score)
{
    WINDOW *GameOver = newwin(6, 52, LINES/2-3, COLS/2-25);
    FILE *picture;
    char ch;
    init_pair(20, COLOR_RED, SECOND_COLOR);
    init_pair(11, COLOR_WHITE, COLOR_BLACK);
    wbkgd(GameOver,COLOR_PAIR(20));
    picture = fopen("gameover.txt","r");
    if(picture == NULL)
    {   wprintw(GameOver,"Error, nie udalo otworzyc sie pliku");
        wrefresh(GameOver);
    }
    wmove(GameOver,0,0);

    // WYSWIETLENIE ASCII ART
    wattron(GameOver, A_BOLD);
    while((ch = fgetc(picture)) != EOF)
        waddch(GameOver,ch);
    wattroff(GameOver, A_BOLD);
    fclose(picture);
    wrefresh(GameOver);
    mvprintw(LINES/2+22, COLS/2-21,"Wcisnij dowolny klawisz by wrocic do menu");
    getch();
    getch();
     
    clear();
    wclear(plansza);
    wclear(score);
    refresh();
    wrefresh(plansza);
    wrefresh(score);
    wbkgd(plansza,COLOR_PAIR(11));
    wbkgd(score,COLOR_PAIR(11));
    delwin(GameOver);

}
