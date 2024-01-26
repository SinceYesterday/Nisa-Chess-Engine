#include "Engine.h"

Engine::Engine()
{
    moves_generator = new MovesGenerator();
    board = new Board();
    search = new Search();
    std::fill_n(board->x88_representation, 128, 0);
}

Engine::calculate() {
    search->best_from = -1;
    search->best_to = -1;
    search->start_search(board->x88_representation, &board->wpieces, &board->bpieces);
}

Engine::generate_moves(int* from, int* to, int shift, int& num_moves) {
    num_moves = 0;
    Board::Pieces_position* pieces = &board->wpieces;

    //moves_generator->generate_moves(board, from, to, shift, pieces, num_moves);
    for (int i  = 0; i < 8; i++) {
        moves_generator->knight_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, pieces->knight[i], pieces->king);

        moves_generator->straight_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, moves_generator->dir.vertical, pieces->rook[i], pieces->king);

        moves_generator->straight_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, moves_generator->dir.diagonal, pieces->bishop[i], pieces->king);

        moves_generator->straight_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, moves_generator->dir.vertical, pieces->queen[i], pieces->king);
        moves_generator->straight_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, moves_generator->dir.diagonal, pieces->queen[i], pieces->king);
    }

    moves_generator->king_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, pieces->king);

    for (int i = 0; i < 8; i++) {
        moves_generator->pawn_moves(board->x88_representation, from, to, num_moves, from, to, num_moves, pieces->pawn[i], pieces->king);
    }
}

Engine::~Engine() {
}

Engine::set_fen(std::string FEN) {
    board->set_board(FEN);
}

Engine::set_side(int cur_side) {
    search->side = cur_side;
}

Engine::set_castle_rights(int* castle_rights) {
    search->moves_generator->castle_rights[0] = castle_rights[0];
    search->moves_generator->castle_rights[1] = castle_rights[1];
    search->moves_generator->castle_rights[2] = castle_rights[2];
    search->moves_generator->castle_rights[3] = castle_rights[3];
}

Engine::set_enpassant(int enpassant) {
    search->moves_generator->enpassant = enpassant;
}

char Engine::weight_to_char(int weight)
{
    if (weight == board->weight.wking) return 'K';
    else if (weight == board->weight.wqueen) return 'Q';
    else if (weight == board->weight.wrook) return 'R';
    else if (weight == board->weight.wbishop) return 'B';
    else if (weight == board->weight.wknight) return 'N';
    else if (weight == board->weight.wpawn) return 'P';

    else if (weight == board->weight.bking) return 'k';
    else if (weight == board->weight.bqueen) return 'q';
    else if (weight == board->weight.brook) return 'r';
    else if (weight == board->weight.bbishop) return 'b';
    else if (weight == board->weight.bknight) return 'n';
    else if (weight == board->weight.bpawn) return 'p';
}

Engine::print_board()
{
    std::cout << std::endl;
    for ( int i = 1; i <= 128; i++) {
        if ( ( (i - 1) & 0x88) == 0 ) {
            if (board->x88_representation[i - 1] == 0) {
                std::cout << " - ";
            } else {
                std::cout << " "  << weight_to_char(board->x88_representation[i - 1]) << " ";
            }
        }
        if ( i % 8 == 0)
            std::cout << "   ";

        if ( i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << std::endl;
    for ( int i = 1; i <= 128; i++) {
        if ( ( (i - 1) & 0x88) == 0 ) {
            std::cout << std::setw(5) << (i - 1);
        }
        if ( i % 8 == 0)
            std::cout << "   ";

        if ( i % 16 == 0)
            std::cout << std::endl;
    }
}
