#ifndef MOVESGENERATOR_H
#define MOVESGENERATOR_H

#include "Board.h"

class MovesGenerator
{
    public:
        struct Moves_direction {
            int vertical[4] = {1, 16, -1, -16};
            int diagonal[4] = {15, 17, -15, -17};
            int knight[8] = {33, 31, 14, 18, -33, -31, -14, -18};
            int king[8] = {1, 16, -1, -16, 15, 17, -15, -17};
        } dir;

        int num_captures;

        //Killer moves
        int killer_moves[88][4];
        int depth;
        bool killer1_found;
        int killer1_from;
        int killer1_to;
        //-----
        bool killer2_found;
        int killer2_from;
        int killer2_to;
        //.........................

        // castle
        int castle_rights[4];

        // enpassant square
        int enpassant = 127;

        MovesGenerator();
        generate_moves(int*, int*, int*, int, int&);

        knight_moves(int*, int*, int*, int&, int*, int*, int&, int, int);
        straight_moves(int*, int*, int*, int&, int*, int*, int&, int*, int, int);
        king_moves(int*, int*, int*,int&, int*, int*, int&, int);
        pawn_moves(int*, int*, int*, int&, int*, int*, int&, int, int);


        attacks_knight_moves(int*, int*, int*, int&, int, int);
        attacks_straight_moves(int*, int*, int*, int&, int*, int, int);
        attacks_king_moves(int*, int*, int*, int&, int);
        attacks_pawn_moves(int*, int*, int*, int&, int, int);

        bool is_in_check(int*, int, int);

        bool capture(int, int, int*);

        void save_killer(int, int, int);
        void check_killer(int, int);

        virtual ~MovesGenerator();

    protected:

    private:
};


#endif // MOVESGENERATOR_H
