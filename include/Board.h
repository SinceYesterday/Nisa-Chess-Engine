#ifndef BOARD_H
#define BOARD_H

#include <iostream>

#define WHITE 1
#define BLACK -1
#define DELETED 127

class Board
{
    public:

        int x88_representation[128];

        struct Pieces_position {
            int king;
            int queen[8];
            int rook[8];
            int bishop[8];
            int knight[8];
            int pawn[8];
        } wpieces, bpieces;

        struct Pieces_value {
            const int wking = 8888;
            const int wqueen = 900;
            const int wrook = 500;
            const int wbishop = 300;
            const int wknight = 310;
            const int wpawn = 100;

            const int bking = -8888;
            const int bqueen = -900;
            const int brook = -500;
            const int bbishop = -300;
            const int bknight = -310;
            const int bpawn = -100;
        } weight;

        Board();
        virtual ~Board();

        void set_board(std::string);

    protected:

    private:
};

#endif // BOARD_H
