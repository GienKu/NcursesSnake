#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "snake.h"

//KOLORY
#define FONT_SCORE_COLOR 154
#define BG_SCORE_COLOR 38
#define BG_COLOR 40
#define SNAKE_COLOR 22
#define FOOD_COLOR 160
#define MAIN_MENU_FONT 89
#define MAIN_MENU_BG 107

// ID PAR KOLOROW
#define ID_EDGES 1
#define ID_SNAKE 2
#define ID_MENU 3
#define ID_FOOD 4
#define ID_SCORE 5
#define ID_MENU_REVERSED 6
#define ID_STANDARD 7
#define ID_GAMEOVER 8
#define ID_MARK_WALL 9

//atrybuty,kolory,rysowanie planszy
void draw_board(WINDOW *plansza,WINDOW *wynik, WINDOW *highscore,int best)
{
	start_color();
    init_pair(ID_SCORE, FONT_SCORE_COLOR, BG_SCORE_COLOR);          //score & highscore
    //------WYNIK-------//
    wbkgd(wynik,COLOR_PAIR(ID_SCORE));                              // wynik bg color
    box(wynik,0,0);
    mvwprintw(wynik,0,1,"SCORE");

    //-------PLANSZA--------//
    init_pair(1,COLOR_YELLOW, BG_COLOR);                //krawedzie
    init_pair(2,SNAKE_COLOR, BG_COLOR);  
        //snake
    wbkgd(plansza, COLOR_PAIR(ID_SNAKE));               //bg color
	wattron(plansza, COLOR_PAIR(ID_EDGES));
	box(plansza, ACS_VLINE, ACS_HLINE);
	wattroff(plansza, COLOR_PAIR(ID_EDGES));

    //-------HIGHSCORE-------//
    wbkgd(highscore,COLOR_PAIR(ID_SCORE));              // wynik bg color
    box(highscore,0,0);
    mvwprintw(highscore,0,1,"BEST");
    mvwprintw(highscore,1,1,"%i",best);
    
	refresh();
    wrefresh(highscore);
	wrefresh(plansza);
    wrefresh(wynik);
}
// utworzenie jedzenia dla snake'a
void create_food(WINDOW *plansza)
{
    init_pair(ID_FOOD, FOOD_COLOR, BG_COLOR);
    int xRand = rand()%64;
    int yRand = rand()%24;
    char ch = mvwinch(plansza,yRand,xRand);
    while(xRand == 0 || xRand%2 != 0 ||
          yRand == 0 || yRand%2 != 0 ||
          ch == 'o')
    {
        xRand = rand()%64;
        yRand = rand()%24;
        ch = mvwinch(plansza,yRand,xRand);
    }

    mvwaddch(plansza,yRand,xRand,'@' | COLOR_PAIR(ID_FOOD));
    wrefresh(plansza);
}
//zmiana długości snake'a
void snake_length_change(Coords **snake,int length)
{
    *snake = realloc(*snake, (length+1) * sizeof(Coords));
}
bool is_food(WINDOW *plansza,Coords *new_head)
{
    int y = (new_head)->y;
    int x = (new_head)->x;
    char ch = mvwinch(plansza,y,x);
    return (ch == '@');
}
//sprawdzenie czy snake nie uderza w ściane
bool collision(int yp, int xp,WINDOW *plansza)
{
    char test = mvwinch(plansza,yp,xp);
    if(test == 'x' || test == 'q' || test == 'o')return false;
    return true;
}
//przesunięcie wspołrzędnych każdej części snake'a
//usnięcie ostatniego elementu ogona i dodanie nowej głowy
void tail_change(Coords *snake_ptr, Coords *new_head, WINDOW *plansza, int s_len)
{
    int yNew = (new_head)->y;
    int xNew = (new_head)->x;
    Coords temp1,temp2 = snake_ptr[0];
    for( int i = 1; i < s_len+1; i++)
    {
        temp1 = snake_ptr[i];
        snake_ptr[i] = temp2;
        temp2 = temp1;
    }
    snake_ptr[0].y = yNew;
    snake_ptr[0].x = xNew;
    
    init_pair(ID_SNAKE, SNAKE_COLOR, BG_COLOR);
    mvwaddch(plansza,snake_ptr[0].y, snake_ptr[0].x,'o' | COLOR_PAIR(ID_SNAKE));
    mvwaddch(plansza,snake_ptr[s_len].y, snake_ptr[s_len].x,' ');
    wrefresh(plansza);
}
// zmiana stanu ściany lub ogona na czerwony po uderzeniu 
void mark_wall(WINDOW *plansza, Coords *new_head)
{
    int y = new_head->y;
    int x = new_head->x;
    init_pair(ID_MARK_WALL, COLOR_RED, BG_COLOR);
    mvwaddch(plansza, y, x, 'x' | COLOR_PAIR(ID_MARK_WALL));
    wrefresh(plansza);
}
//sprawdzenie kierunku poruszania sie snake'a i kolizji
bool which_direction(Coords **snake_ptr, int action, WINDOW *plansza, Coords *new_head)
{
    int yNext = (*snake_ptr)->y;
    int xNext = (*snake_ptr)->x;
    switch (action)
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
// Sprawdzenie czy wprowadzony truch nie jest przeciwny do aktualnego
bool is_good_move(int act_mv,int next_mv)
{
    switch (next_mv)
    {
        case 'w':
        case 'W':
        case KEY_UP:
            if(act_mv == 's' || act_mv == 'S' || act_mv == KEY_DOWN)
                return false;
            break;
        case 'a':
        case 'A':
        case KEY_LEFT:
            if(act_mv == 'd' || act_mv == 'D' || act_mv == KEY_RIGHT)
                return false;
            break;
        case 's':
        case 'S':
        case KEY_DOWN:
            if(act_mv == 'w' || act_mv == 'W' || act_mv == KEY_UP)
                return false;
            break;
        case 'd':
        case 'D':
        case KEY_RIGHT:
            if(act_mv == 'a' || act_mv == 'd' || act_mv == KEY_LEFT)
                return false;
            break;
    }
    return true;
}
void main_game_loop(int level)
{
    WINDOW *highscore = newwin(3,7,LINES/2-15,COLS/2+26);
    WINDOW *score = newwin(3,7,LINES/2-15,COLS/2-32);
    WINDOW *plansza = newwin(25,65,LINES/2-12,COLS/2-32);
    Coords *snake_parts = malloc( 4 * sizeof(Coords));
    Coords new_head; 
    FILE *bscores;
    char scrs[3][3] = {
                        {"000"},
                        {"000"},
                        {"000"}
    };
    int yPl,xPl,snake_length = 3,scr = 0;
    bool is_alive = true;
    int actual_move = KEY_RIGHT,next_move;

    mvwprintw(score,1,1,"%i",scr);
    wrefresh(score);

    keypad(plansza,true);
    getmaxyx(plansza,yPl,xPl);
    for( int i = 0; i < 3; i++)
    {
        snake_parts[i].y = yPl/2;
        snake_parts[i].x = xPl/2+8-2*i;
        mvwaddch(plansza,snake_parts[i].y,snake_parts[i].x,'o');
    }
 
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
    // ---------WYPISANIE HIGHSCORE-----------
    bscores = fopen("data/bestscore.txt","r");
    int i = 0,sc = 0;
    while((fscanf(bscores,"%s",scrs[i])) != -1){
        i++;
    }
    sc = strtol(scrs[level],NULL,10);
    fclose(bscores);
    //----------------------------------------

    draw_board(plansza,score,highscore,sc);
    create_food(plansza);
    wmove(plansza,0,0);
    
    getch();

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
            tail_change(snake_parts,&new_head,plansza,snake_length);    //zmiana koordynatów snake'a
        }
        else 
        {
            is_alive = false;
            mark_wall(plansza,&new_head); 
            game_over(plansza,score);
        }          
    }
    //--------SPRAWDZENIE HIGHSCORE----------
    if(scr > sc)
    {
        sprintf(scrs[level],"%d", scr);
        bscores = fopen("data/bestscore.txt","w");
        for(int j = 0; j < 3; j++)
            fprintf(bscores,"%s\n",scrs[j]);
        fclose(bscores);
    }
    //----------------------------------------
    main_menu();
}
void main_menu()
{
    WINDOW *MenuContainer = newwin(LINES, COLS, 0, 0);
    WINDOW *MenuPanel = subwin(MenuContainer, 10, 28, LINES/2, COLS/2-8);
    WINDOW *MenuAscii = subwin(MenuContainer, LINES/2,79,1,COLS/2-39);
    FILE *picture;
    char ch;
    
    init_pair(ID_MENU, MAIN_MENU_FONT, MAIN_MENU_BG);               //font-roz, backg- zielen
    init_pair(ID_MENU_REVERSED, MAIN_MENU_BG, MAIN_MENU_FONT);      //na odwrot^
    init_pair(ID_STANDARD, COLOR_WHITE, COLOR_BLACK);               //standardowy kolor terminala
    bkgd(COLOR_PAIR(ID_MENU));                                      //bgcolor stdrscr
    wbkgd(MenuAscii,COLOR_PAIR(ID_MENU));                           //bgcolor MenuAscii
    wbkgd(MenuPanel,COLOR_PAIR(ID_MENU));                           //bgcolor MenuPanel

    picture = fopen("data/obrazek.txt","r");
    if(picture == NULL)
    {   wprintw(MenuAscii,"Error, nie udalo otworzyc sie pliku");
        wrefresh(MenuAscii);
    }
    wmove(MenuAscii,0,0);

    // WYSWIETLENIE ASCII ART
    wattron(MenuAscii,COLOR_PAIR(ID_MENU) | A_BOLD);
    while((ch = fgetc(picture)) != EOF)
        waddch(MenuAscii,ch);
    wattroff(MenuAscii,COLOR_PAIR(ID_MENU) | A_BOLD);

    fclose(picture);
    refresh();
    wrefresh(MenuPanel);
    wrefresh(MenuAscii);
    
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
                wattron(MenuPanel,COLOR_PAIR(ID_MENU_REVERSED));
                mvwaddstr(MenuPanel, i+1, 1, choices[i]);
                wattroff(MenuPanel,COLOR_PAIR(ID_MENU_REVERSED));
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
    init_pair(ID_GAMEOVER, COLOR_RED, BG_SCORE_COLOR);
    wbkgd(GameOver,COLOR_PAIR(ID_GAMEOVER));
    picture = fopen("data/gameover.txt","r");
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
    attron(COLOR_PAIR(ID_STANDARD));
    mvprintw(LINES/2+22, COLS/2-21,"Wcisnij dowolny klawisz by wrocic do menu");
    attroff(COLOR_PAIR(ID_STANDARD));
    getch();
    getch();

    clear();
    wclear(plansza);
    wclear(score);
    refresh();
    wrefresh(plansza);
    wrefresh(score);
    wbkgd(plansza,COLOR_PAIR(ID_STANDARD));
    wbkgd(score,COLOR_PAIR(ID_STANDARD));
    delwin(GameOver);

}
