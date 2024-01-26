#ifndef ENGINE_H
#define ENGINE_H

#include "MovesGenerator.h"
#include "Board.h"
#include "Search.h"

/*

    0,   1,   2,   3,   4,   5,   6,   7,
    16,  17,  18,  19,  20,  21,  22,  23,
    32,  33,  34,  35,  36,  37,  38,  39,
    48,  49,  50,  51,  52,  53,  54,  55,
    64,  65,  66,  67,  68,  69,  70,  71,
    80,  81,  82,  83,  84,  85,  86,  87,
    96,  97,  98,  99,  100, 101, 102, 103,
    112, 113, 114, 115, 116, 117, 118, 119};
*/



class Engine
{
    public:

        MovesGenerator* moves_generator;
        Board* board;
        Search* search;

        //int x64_board[64];
        //int board[128];

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
