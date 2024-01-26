#include "Board.h"

Board::Board()
{
    //ctor
}

Board::~Board()
{
    //dtor
}

void Board::set_board(std::string FEN) {
	//cout << FEN << endl;

	wpieces.king = DELETED;

	bpieces.king = DELETED;

	for (int i = 0; i < 8; i++) {
        wpieces.queen[i] = DELETED;
		wpieces.knight[i] = DELETED;
		wpieces.rook[i] = DELETED;
		wpieces.bishop[i] = DELETED;
		wpieces.pawn[i] = DELETED;

		bpieces.queen[i] = DELETED;
		bpieces.knight[i] = DELETED;
		bpieces.rook[i] = DELETED;
		bpieces.bishop[i] = DELETED;
		bpieces.pawn[i] = DELETED;
	}


	for (int i = 0; i < 128; i++) {
		x88_representation[i] = 0;
	}

	int i_setter = 0;
	int i_Q = 0, i_R = 0, i_B = 0, i_N = 0, i_P = 0;
	int i_q = 0, i_r = 0, i_b = 0, i_n = 0, i_p = 0;

	auto set_piece = [] (int* board, int& i_setter, int weight, int& place) {
		if (i_setter < 128) {
			board[i_setter] = weight;
			place = i_setter;
			i_setter++;
		} else {
			std::cout << "FEN ERROR" << std::endl;
		}

	};

	for (int i = 0; i < FEN.length(); i++) {
		if (FEN[i] == '/') {
			i_setter += 8;
		} else if (FEN[i] == 'K') {
			set_piece(x88_representation, i_setter, weight.wking, wpieces.king);
		} else if (FEN[i] == 'Q') {
			set_piece(x88_representation, i_setter, weight.wqueen, wpieces.queen[i_Q++]);
		} else if (FEN[i] == 'R') {
			set_piece(x88_representation, i_setter, weight.wrook, wpieces.rook[i_R++]);
		} else if (FEN[i] == 'B') {
			set_piece(x88_representation, i_setter, weight.wbishop, wpieces.bishop[i_B++]);
		} else if (FEN[i] == 'N') {
			set_piece(x88_representation, i_setter, weight.wknight, wpieces.knight[i_N++]);
		} else if (FEN[i] == 'P') {
			set_piece(x88_representation, i_setter, weight.wpawn, wpieces.pawn[i_P++]);
		}

		else if (FEN[i] == 'k') {
			set_piece(x88_representation, i_setter, weight.bking, bpieces.king);
		} else if (FEN[i] == 'q') {
			set_piece(x88_representation, i_setter, weight.bqueen, bpieces.queen[i_q++]);
		} else if (FEN[i] == 'r') {
			set_piece(x88_representation, i_setter, weight.brook, bpieces.rook[i_r++]);
		} else if (FEN[i] == 'b') {
			set_piece(x88_representation, i_setter, weight.bbishop, bpieces.bishop[i_b++]);
		} else if (FEN[i] == 'n') {
			set_piece(x88_representation, i_setter, weight.bknight, bpieces.knight[i_n++]);
		} else if (FEN[i] == 'p') {
			set_piece(x88_representation, i_setter, weight.bpawn, bpieces.pawn[i_p++]);
		}

		else if (isdigit(FEN[i])) {
			int n = FEN[i] - '0';
			i_setter += n;
		}
	}
}
