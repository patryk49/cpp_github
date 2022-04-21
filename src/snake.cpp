#include <curses.h>
#include <time.h>
#include <unistd.h>

#include "SPL/Utils.hpp"


constexpr size_t rows = 20;
constexpr size_t cols = 20;

uint8_t grid[rows][cols];

size_t points;


sp::Rand32 rng{clock()};

void spawn_snack() noexcept{
	for (;;){
		size_t r = 2 + rng() % (rows-4);
		size_t c = 2 + rng() % (cols-4);
		if (!grid[r][c]){
			grid[r][c] = 'F';
			return;
		}
	}
}


bool hits_hedge(size_t r, size_t c) noexcept{
	return r==(size_t)-1 || r==rows || c==(size_t)-1 || c==cols;
}


void display_init(){
	for (size_t i=1; i!=rows+3; ++i){
		mvaddch(i, 0, '#');
		mvaddch(i, cols+1, '#');
	}
	for (size_t i=1; i!=cols+1; ++i){
		mvaddch(1, i, '#');	
		mvaddch(rows+2, i, '#');
	}
	mvaddstr(0, 1, "points: ");
}

void display_draw(){
	char buff[32];
	sprintf(buff, "%4lu", points);
	mvaddstr(0, 9, buff);
	for (size_t i=0; i!=rows; ++i){
		for (size_t j=0; j!=cols; ++j){
			uint8_t c = grid[i][j];
			mvaddch(i+2, j+1, c ? c=='F' ? 'O' : 'M' : ' ');
		}
	}
}

void flush_input() noexcept{ while (getch() != ERR){} }

int main(){
	initscr();
	cbreak();
	noecho();

	size_t head_y;
   size_t head_x;
   size_t tail_y;
   size_t tail_x;

Start:
	timeout(0);
//	sp::init(grid);
	
	grid[rows/2+0][cols/2] = 'w';
	grid[rows/2+1][cols/2] = 'w';
	grid[rows/2+2][cols/2] = 'w';

	head_y = rows/2 - 1;
	head_x = cols/2;
	tail_y = rows/2 + 2;
	tail_x = cols/2;

	points = 0;
	spawn_snack();
	
	display_init();
	display_draw();

	Loop:{
		char c = getch();
		switch (c){
		case 'w':
		case 'a':
		case 'd':
			grid[rows/2-1][cols/2] = c;
			break;
		case 'q': goto Exit;
		default: goto Loop;
		}
	}
	display_draw();

	for (char c_next = '\0';;){
		char c = (c_next=='w' || c_next=='s' || c_next=='a' || c_next=='d') ? c_next : getch();
		c_next = getch();
		flush_input();
		
		if (c=='q') goto Exit;
		
		uint8_t newdir = grid[head_y][head_x];
		switch (newdir){
		case 'w':
		case 's':
			if (c=='a' || c=='d') newdir = c;
			break;
		case 'a':
		case 'd':
			if (c=='w' || c=='s') newdir = c;
			break;
		default: break;
		}

		grid[head_y][head_x] = newdir;

		head_y += (newdir=='s') - (newdir=='w');
		head_x += (newdir=='d') - (newdir=='a');

		if (hits_hedge(head_y, head_x)) goto Over;

		if (grid[head_y][head_x] == 'F'){
			grid[head_y][head_x] = '\0';
			++points;
			spawn_snack();
		} else{
			uint8_t taildir = grid[tail_y][tail_x];
			grid[tail_y][tail_x] = '\0';
			
			tail_y += (taildir=='s') - (taildir=='w');
			tail_x += (taildir=='d') - (taildir=='a');
		}

		if (grid[head_y][head_x]) goto Over;

		grid[head_y][head_x] = newdir;
	
		display_draw();
		usleep(200'000);
	}


Over:
	mvaddstr(1, cols/2-5, " GAME OVER ");
	usleep(500'000);
	flush_input();
	timeout(-1);
	if (getch() != 'q') goto Start;

Exit:
	endwin();
	return 0;
}
