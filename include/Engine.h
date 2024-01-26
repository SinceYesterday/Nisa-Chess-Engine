#ifndef ENGINE_H
#define ENGINE_H

#include "MovesGenerator.h"
#include "Board.h"
#include "Search.h"

class Engine
{
    public:

        MovesGenerator* moves_generator;
        Board* board;
        Search* search;

        Engine();
        generate_moves(int*, int*, int, int&);
        calculate();
        set_fen(std::string);
        set_side(int);
        set_castle_rights(int*);
        set_enpassant(int);
        print_board();
        char weight_to_char(int);
        virtual ~Engine();

    protected:

    private:
};

#endif // ENGINE_H
