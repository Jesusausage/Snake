#include <ncurses.h>
#include <chrono>
#include <iostream>

using namespace std;

struct Block;
struct Block {
    int blockPos[2];
    char entityType;
    Block *ptr_to_next;
};

void Print_Frame(WINDOW *game, WINDOW *dash,
		 Block snake[], int snakeLength, Block apple[], int score);
void Move_Snake(WINDOW *game, WINDOW *dash,
		Block snake[], int &snakeLength, int &dir, int &collisionFlag,
		Block apple[], int score, chrono::steady_clock::time_point &tend);
void Spawn_Apple(Block apple[], Block snake[], int snakeLength);
int Collision_Detect(WINDOW *game, Block snake[], int &snakeLength, Block apple[]);
void Game_Over(WINDOW *game, Block snake[], int snakeLength, Block apple[]);
WINDOW *Create_Window(int height, int width, int starty, int startx);
void Delete_Window(WINDOW *dynamicWin);


int main()
{
    srand(time(NULL));
    Block snake[900];
    int snakeLength = 10;
    Block apple[4];
    int dir=4, collisionFlag=1;
    int score=0;
    WINDOW *game, *dash;
    chrono::steady_clock::time_point tend = chrono::steady_clock::now() + chrono::milliseconds(200);

    initscr();
    noecho();
    cbreak();
    curs_set(0);
  
    start_color();
    init_color(COLOR_GREEN, 150, 350, 150);
    init_color(COLOR_YELLOW, 800, 800, 700);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
  
    game = Create_Window(32, 62, 10, 20);
    dash = Create_Window(3, 62, 42, 20);
    keypad(game, TRUE);
    wtimeout(game, 200);
    wbkgd(game, COLOR_PAIR(3));
    wbkgd(dash, COLOR_PAIR(3));

    for (int parts=0; parts<10; parts++) {
	snake[parts].blockPos[0] = 11-parts;
	snake[parts].blockPos[1] = 2;
    }
    for (int parts=10; parts<900; parts++) {
	snake[parts].blockPos[0] = 256;
	snake[parts].blockPos[1] = 256;
    }

    Spawn_Apple(apple, snake, snakeLength);

    Move_Snake(game, dash, snake, snakeLength, dir, collisionFlag, apple, score, tend);

    Game_Over(game, snake, snakeLength, apple);

    Delete_Window(game);
    endwin();
    return 0;
}


void Move_Snake(WINDOW *game, WINDOW *dash,
		Block snake[], int &snakeLength, int &dir, int &collisionFlag,
		Block apple[], int score,  chrono::steady_clock::time_point &tend)
{
    int input;
  
    while(collisionFlag) {
	Print_Frame(game, dash, snake, snakeLength, apple, score);

	input = wgetch(game);
	if (input==KEY_UP && dir!=2) dir=1;
	if (input==KEY_DOWN && dir!=1) dir=2;
	if (input==KEY_LEFT && dir!=4) dir=3;
	if (input==KEY_RIGHT && dir!=3) dir=4;
	flushinp();

	while (chrono::steady_clock::now() < tend);

	Block lastPart;
	lastPart.blockPos[0] = snake[snakeLength-1].blockPos[0];
	lastPart.blockPos[1] = snake[snakeLength-1].blockPos[1];
    
	for (int segment=snakeLength-1; segment>0; segment--) {
	    snake[segment].blockPos[0] = snake[segment-1].blockPos[0];
	    snake[segment].blockPos[1] = snake[segment-1].blockPos[1];
	}
    
	if (dir==1) {
	    if (snake[0].blockPos[1] == 1) snake[0].blockPos[1] += 29;
	    else snake[0].blockPos[1]--;
	}
	if (dir==2) {
	    if (snake[0].blockPos[1] == 30) snake[0].blockPos[1] -= 29;
	    else snake[0].blockPos[1]++;
	}
	if (dir==3) {
	    if (snake[0].blockPos[0] == 1) snake[0].blockPos[0] += 29;
	    else snake[0].blockPos[0]--;
	}
	if (dir==4) {
	    if (snake[0].blockPos[0] == 30) snake[0].blockPos[0] -= 29;
	    else snake[0].blockPos[0]++;
	}

	collisionFlag  = Collision_Detect(game, snake, snakeLength, apple);
	if (collisionFlag==2) {
      
	    snakeLength++;
	    snake[snakeLength-1].blockPos[0] = lastPart.blockPos[0];
	    snake[snakeLength-1].blockPos[1] = lastPart.blockPos[1];
	    score += 1;
      
	    Spawn_Apple(apple, snake, snakeLength);
	}
    
	tend = chrono::steady_clock::now() + chrono::milliseconds(200);

    }
}


int Collision_Detect(WINDOW *game, Block snake[], int &snakeLength, Block apple[])
{
    for (int segment=1; segment<snakeLength; segment++) {

	if (snake[0].blockPos[0]==snake[segment].blockPos[0] &&
	    snake[0].blockPos[1]==snake[segment].blockPos[1]) return 0;

	for (int applePart=0; applePart<4; applePart++) {
	    if (snake[0].blockPos[0]==apple[applePart].blockPos[0] &&
		snake[0].blockPos[1]==apple[applePart].blockPos[1]) return 2;
	}

    }
  
    return 1;
}

  
void Print_Frame(WINDOW *game, WINDOW *dash,
		 Block snake[], int snakeLength, Block apple[], int score)
{  
    wclear(game);
    wclear(dash);

    wattron(game, COLOR_PAIR(3));
    box(game, 0, 0);
    box(dash, 0, 0);
    mvwprintw(game, snake[0].blockPos[1], snake[0].blockPos[0]*2-1, "[]");
    wattron(game, COLOR_PAIR(1));
    for (int snakePart=1; snakePart<snakeLength; snakePart++)
	mvwprintw(game, snake[snakePart].blockPos[1], snake[snakePart].blockPos[0]*2-1, "[]");

    wattron(game, COLOR_PAIR(2));
    for (int applePart=0; applePart<4; applePart++)
	mvwprintw(game, apple[applePart].blockPos[1], apple[applePart].blockPos[0]*2-1, "$$");
  
    wrefresh(game);

    mvwprintw(dash, 1, 2, "Score: %d", score);
    wrefresh(dash);
}


void Game_Over(WINDOW *game, Block snake[], int snakeLength, Block apple[])
{
    wclear(game);
    wattron(game, COLOR_PAIR(3));
    box(game, 0, 0);

    wattron(game, COLOR_PAIR(2));
    for (int line=1; line<30; line+=4)
	mvwprintw(game, line, 1,
		  "    YOU LOSE   YOU LOSE   YOU LOSE   YOU LOSE   YOU LOSE");

    wattron(game, COLOR_PAIR(1));
    for (int part=snakeLength-1; part>0; part--)
	mvwprintw(game, snake[part].blockPos[1], snake[part].blockPos[0]*2-1, "[]");
    wattron(game, COLOR_PAIR(2));
    mvwprintw(game, snake[0].blockPos[1], snake[0].blockPos[0]*2-1, "XX");

    for (int applePart=0; applePart<4; applePart++)
	mvwprintw(game, apple[applePart].blockPos[1], apple[applePart].blockPos[0]*2, "$$");
  
    wrefresh(game);

    wtimeout(game, -1);
    wgetch(game);
}


void Spawn_Apple(Block apple[], Block snake[], int snakeLength)
{
  
    apple[0].blockPos[0] = rand()%29 + 1;
    apple[0].blockPos[1] = rand()%29 + 1;
    apple[1].blockPos[0] = apple[0].blockPos[0]+1;
    apple[1].blockPos[1] = apple[0].blockPos[1];
    apple[2].blockPos[0] = apple[0].blockPos[0];
    apple[2].blockPos[1] = apple[0].blockPos[1]+1;
    apple[3].blockPos[0] = apple[0].blockPos[0]+1;
    apple[3].blockPos[1] = apple[0].blockPos[1]+1;

    for (int applePart=0; applePart<4; applePart++) {
	for (int snakePart=0; snakePart<snakeLength; snakePart++) {
	    if (apple[applePart].blockPos[0] == snake[snakePart].blockPos[0] &&
		apple[applePart].blockPos[1] == snake[snakePart].blockPos[1])
		Spawn_Apple(apple, snake, snakeLength);
	}
    }
  
}
  

WINDOW *Create_Window(int height, int width, int starty, int startx)
{
    WINDOW *dynamicWin;
    dynamicWin = newwin(height, width, starty, startx);
    box(dynamicWin, 0, 0);
    wrefresh(dynamicWin);

    return dynamicWin;
}


void Delete_Window(WINDOW *dynamicWin)
{
    wborder(dynamicWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(dynamicWin);
    delwin(dynamicWin);
}
