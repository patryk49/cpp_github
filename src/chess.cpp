#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "SPL/Utils.hpp"

struct Chess{
	enum Piece : int8_t{
		None, Error,
		BPawn, BTower, BRook, BBishop, BQueen, BKing,
		WPawn, WTower, WRook, WBishop, WQueen, WKing,
	};

	static constexpr const char *NameTable[]{
		"  ", "xx",
		"bP", "bT", "bR", "bB", "bQ", "bK",
		"wP", "wT", "wR", "wB", "wQ", "wK"
	};
};

bool is_black(Chess::Piece x) noexcept{ return Chess::BPawn <= x && x <= Chess::BKing; }
bool is_white(Chess::Piece x) noexcept{ return Chess::WPawn <= x; }


constexpr Chess::Piece new_board[8][8] = {
	{Chess::BTower, Chess::BRook, Chess::BBishop, Chess::BKing,  Chess::BQueen, Chess::BBishop, Chess::BRook, Chess::BTower},
	{Chess::BPawn,  Chess::BPawn, Chess::BPawn,   Chess::BPawn,  Chess::BPawn,  Chess::BPawn,   Chess::BPawn, Chess::BPawn },
	{Chess::None,   Chess::None,  Chess::None,    Chess::None,   Chess::None,   Chess::None,    Chess::None,  Chess::None  },
	{Chess::None,   Chess::None,  Chess::None,    Chess::None,   Chess::None,   Chess::None,    Chess::None,  Chess::None  },
	{Chess::None,   Chess::None,  Chess::None,    Chess::None,   Chess::None,   Chess::None,    Chess::None,  Chess::None  },
	{Chess::None,   Chess::None,  Chess::None,    Chess::None,   Chess::None,   Chess::None,    Chess::None,  Chess::None  },
	{Chess::WPawn,  Chess::WPawn, Chess::WPawn,   Chess::WPawn,  Chess::WPawn,  Chess::WPawn,   Chess::WPawn, Chess::WPawn },
	{Chess::WTower, Chess::WRook, Chess::WBishop, Chess::WQueen, Chess::WKing,  Chess::WBishop, Chess::WRook, Chess::WTower}
};

Chess::Piece board[8][8];

void display_board(){
	puts("   A  B  C  D  E  F  G  H");
	for (size_t i=0; i!=8; ++i){
		puts("  |--|--|--|--|--|--|--|--|");
		putchar("87654321"[i]);
		putchar(' ');
		putchar('|');
		for (size_t j=0; j!=8; ++j){
			fputs(Chess::NameTable[board[i][j]], stdout);
			putchar('|');
		}
		putchar(' ');
		putchar("87654321"[i]);
		putchar('\n');
	} 
	puts("  |--|--|--|--|--|--|--|--|");
	puts("   A  B  C  D  E  F  G  H");
}

void display_board_as_blacks(){
	puts("   H  G  F  E  D  C  B  A");
	for (size_t i=8; --i!=(size_t)-1;){
		puts("  |--|--|--|--|--|--|--|--|");
		putchar("87654321"[i]);
		putchar(' ');
		putchar('|');
		for (size_t j=8; --j!=(size_t)-1;){
			fputs(Chess::NameTable[board[i][j]], stdout);
			putchar('|');
		}
		putchar(' ');
		putchar("87654321"[i]);
		putchar('\n');
	} 
	puts("  |--|--|--|--|--|--|--|--|");
	puts("   H  G  F  E  D  C  B  A");
}


struct Position{ int y, x; };



Chess::Piece move_piece(Position f, Position t) noexcept{
	switch (board[f.y][f.x]){
	case Chess::None:
		return Chess::Error;
	case Chess::WPawn:
		if (t.x == f.x){
			if (t.y != f.y-1 && (f.y != 6 || t.y != f.y-2)) return Chess::Error;
			if (board[t.y][t.x] != Chess::None) return Chess::Error;
			if (t.y == f.y-2 && board[f.y-1][f.x] != Chess::None) return Chess::Error;
			break;
		}
		if (t.x == f.x-1 || t.x == f.x+1){
			if (t.y != f.y-1) return Chess::Error;
			break;
		}
		return Chess::Error;
	case Chess::BPawn:
		if (t.x == f.x){
			if (t.y != f.y+1 && (f.y != 1 || t.y != f.y+2)) return Chess::Error;
			if (board[t.y][t.x] != Chess::None) return Chess::Error;
			if (t.y == f.y+2 && board[f.y+1][f.x] != Chess::None) return Chess::Error;
			break;
		}
		if (t.x == f.x-1 || t.x == f.x+1){
			if (t.y != f.y+1) return Chess::Error;
			break;
		}
		return Chess::Error;
	case Chess::WRook:
	case Chess::BRook:
		if (sp::abs(t.x-f.x) == 2 && sp::abs(t.y-f.y) != 1) return Chess::Error;
		if (sp::abs(t.y-f.y) != 2 || sp::abs(t.x-f.x) != 1) return Chess::Error;
		
		break;
	case Chess::WBishop:
	case Chess::BBishop:
		if (sp::abs(t.y-f.y) != sp::abs(t.x-f.x)) return Chess::Error;
		{
			Position step = {sp::sign(t.y-f.y), sp::sign(t.x-f.x)};
			for (int i=f.y, j=f.x; (i+=step.y)!=t.y;){
				j += step.x;
				if (board[i][j] != Chess::None) return Chess::Error;
			}
		}
		break;
	case Chess::WQueen:
	case Chess::BQueen:
		if (sp::abs(t.y-f.y) == sp::abs(t.x-f.x)){
			Position step = {sp::sign(t.y-f.y), sp::sign(t.x-f.x)};
			for (int i=f.y, j=f.x; (i+=step.y)!=t.y;){
				j += step.x;
				if (board[i][j] != Chess::None) return Chess::Error;
			}
			break;
		} [[fallthrough]];
	case Chess::WTower:
	case Chess::BTower:
		if (t.x == f.x){
			if (f.y < t.y){
				for (int i=f.y; ++i!=t.y;)
					if (board[i][f.x] != Chess::None) return Chess::Error;
			} else{
				for (int i=f.y; --i!=t.y;)
					if (board[i][f.x] != Chess::None) return Chess::Error;
			}
			break;
		}
		if (t.y == f.y){
			if (f.x < t.x){
				for (int i=f.x; ++i!=t.x;)
					if (board[f.y][i] != Chess::None) return Chess::Error;
			} else{
				for (int i=f.x; --i!=t.x;)
					if (board[f.y][i] != Chess::None) return Chess::Error;
			}
			break;
		}
		return Chess::Error;
	case Chess::WKing:
	case Chess::BKing:
		if (sp::abs(t.y-f.y) > 1 || sp::abs(t.x-f.x) > 1) return Chess::Error;
		break;
	default:
		return Chess::Error;
	}

	if (is_white(board[f.y][f.x]) ? is_white(board[t.y][t.x]) : is_black(board[t.y][t.x]))
		return Chess::Error;	
	Chess::Piece res = board[t.y][t.x];
	board[t.y][t.x] = board[f.y][f.x];
	board[f.y][f.x] = Chess::None;
	return res;
}

Position read_position(const char *str) noexcept{
	return Position{7-(str[1]-'1'), str[0]-'A'};
}

bool on_board(Position p) noexcept{
	return (7u-(size_t)p.x < 8u) && (7u-(size_t)p.y < 8u);
}

int main(){
	memcpy(board, new_board, sizeof(board));
	char buffer[8];

	bool turn = false;
	for (;;){
		write(1, "\E[H\E[2J", 7);
		turn ? display_board_as_blacks() : display_board();
	MakeMove:
		if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
		if (buffer[2] != ' ' || buffer[5] != '\n'){
			puts("Wrong input format.");
			continue;
		}
		Position f = read_position(buffer);
		Position t = read_position(buffer+3);

		if (!on_board(f) || !on_board(t)){
			puts("Illegal position.");
			goto MakeMove;
		}
		if (board[f.y][f.x] == Chess::None){
			puts("Chess piece was not selected.");
			goto MakeMove;
		}
		if (f.y == t.y && f.x == t.x){
			puts("This is not a move.");
			goto MakeMove;
		}
		if (is_white(board[f.y][f.x]) == turn){
			puts(turn ?
				"Cannot move white chess piece on blacks's turn"
				: "Cannot move black chess piece on white's turn"
			);
			goto MakeMove;
		}

		Chess::Piece killed = move_piece(f, t);
		switch (killed){
		case Chess::Error:
			puts("This move is not possible.");
			goto MakeMove;
		case Chess::WKing:
			puts("The Blacks won.");
			return 0;
		case Chess::BKing:
			puts("The Whites won.");
			return 0;
		default:
			turn = !turn;
		}
	}
}
