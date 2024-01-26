#include "MovesGenerator.h"

MovesGenerator::MovesGenerator()
{

}

MovesGenerator::~MovesGenerator()
{
    //dtor
}


bool MovesGenerator::capture(int origin, int dest, int* board) {
    if (board[origin] > 0 and board[dest] < 0) return true;
    if (board[origin] < 0 and board[dest] > 0) return true;
    return false;
    //return ((board[origin] >> 31) xor (board[dest] >> 31));
}

MovesGenerator::knight_moves(int* board, int* from, int* to, int& num_moves, int* capture_from, int* capture_to, int& capture_num_moves, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
		for (int j = 0; j < 8; j++) {
			dest = origin + dir.knight[j];
			if ( (dest & 0x88) != 0) continue;
			if ( board[dest] == 0) {
                check_killer(origin, dest);
				from[num_moves] = origin;
				to[num_moves++] = dest;
			} else if (capture(origin, dest, board)) {
			    capture_from[capture_num_moves] = origin;
                capture_to[capture_num_moves++] = dest;
			}
		}
    }
}


MovesGenerator::straight_moves(int* board, int* from, int* to, int& num_moves, int* capture_from, int* capture_to, int& capture_num_moves, int* dir, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
		for (int d = 0; d < 4; d++) {
			dest = origin;
			for (int j = 0; j < 7; j++) {
				dest += dir[d];
				if ( (dest & 0x88) != 0) break;
				if ( board[dest] == 0) {
                    check_killer(origin, dest);
					from[num_moves] = origin;
                    to[num_moves++] = dest;
				} else {
					if (capture(origin, dest, board)) {
						capture_from[capture_num_moves] = origin;
                        capture_to[capture_num_moves++] = dest;
					}
					break;
				}
			}
		}
	}
}

MovesGenerator::king_moves(int* board, int* from, int* to, int& num_moves, int* capture_from, int* capture_to, int& capture_num_moves, int origin) {
	int dest;
	if (origin != DELETED) {
		for (int j = 0; j < 8; j++) {
			dest = origin + dir.king[j];
			if ( (dest & 0x88) != 0) continue;
			if ( board[dest] == 0) {
                check_killer(origin, dest);
				from[num_moves] = origin;
                to[num_moves++] = dest;
			} else if (capture(origin, dest, board)) {
				capture_from[capture_num_moves] = origin;
                capture_to[capture_num_moves++] = dest;
                //num_captures++;
			}
		}
	}


    if (board[origin] > 0) {
        if (castle_rights[3] and origin == 116 and board[119] == 500 and board[117] == 0 and board[118] == 0 and !is_in_check(board, 116, 1) and !is_in_check(board, 117, 1) and !is_in_check(board, 118, 1)) {
            //white short castle
            from[num_moves] = origin;
            to[num_moves++] = 4004;
        }
        if (castle_rights[2] and origin == 116 and board[112] == 500 and board[113] == 0 and board[114] == 0 and board[115] == 0 and !is_in_check(board, 116, 1) and !is_in_check(board, 114, 1) and !is_in_check(board, 115, 1)) {
            //white long castle
            from[num_moves] = origin;
            to[num_moves++] = 4003;
        }
    } else {
        if (castle_rights[1] and origin == 4 and board[7] == -500 and board[5] == 0 and board[6] == 0 and !is_in_check(board, 4, 0) and !is_in_check(board, 5, 0) and !is_in_check(board, 6, 0)) {
            //black short castle
            from[num_moves] = origin;
            to[num_moves++] = 4002;
        }
        if (castle_rights[0] and origin == 4 and board[0] == -500 and board[1] == 0 and board[2] == 0 and board[3] == 0 and !is_in_check(board, 4, 0) and !is_in_check(board, 2, 0) and !is_in_check(board, 3, 0)) {
            //black long castle
            from[num_moves] = origin;
            to[num_moves++] = 4001;
        }
    }

}




MovesGenerator::pawn_moves(int* board, int* from, int* to, int& num_moves, int* capture_from, int* capture_to, int& capture_num_moves, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
        if (board[origin] > 0 and origin >= 16 and origin <= 23) {
            // mid
            dest = origin - 16;
            if (board[dest] == 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1003;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1002;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1001;
            }
            // left
            dest = origin - 17;
            if ( !(dest & 0x88) and board[dest] < 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2008;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2006;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2002;
            }
            // right
            dest = origin - 15;
            if ( !(dest & 0x88) and board[dest] < 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2007;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2005;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2001;
            }
            return 0;
        } else if (board[origin] < 0 and origin >= 96 and origin <= 103) {
            dest = origin + 16;
            if (board[dest] == 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1003;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1002;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1001;
            }
            // left
            dest = origin + 15;
            if ( !(dest & 0x88) and board[dest] > 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2008;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2006;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2002;
            }
            // right
            dest = origin + 17;
            if ( !(dest & 0x88) and board[dest] > 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2007;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2005;
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2001;
            }
            return 0;
        }


		int r1, r2;
		int d1, d2;
		int left_bound,right_bound;

		if (board[origin] > 0) {
			r1 = -16;
			r2 = -32;
			d1 = -17;
			d2 = -15;
			left_bound = 96;
			right_bound = 103;
		} else {
			r1 = +16;
			r2 = +32;
			d1 = +17;
			d2 = +15;
			left_bound = 16;
			right_bound = 23;
		}

		//For direct moves
		dest = origin + r1;
		if (!(dest & 0x88) and board[dest] == 0) {
            check_killer(origin, dest);
			from[num_moves] = origin;
            to[num_moves++] = dest;

            dest = origin + r2;
            if (!(dest & 0x88) and board[dest] == 0 and (origin >= left_bound and origin <= right_bound)) {
                check_killer(origin, dest);
                from[num_moves] = origin;
                to[num_moves++] = dest;
            }

		}

		//for diagonal captures
		dest = origin + d1;
		if (!(dest & 0x88) and capture(origin, dest, board)) {
			capture_from[capture_num_moves] = origin;
            capture_to[capture_num_moves++] = dest;
            //num_captures++;
		}
		dest = origin + d2;
		if (!(dest & 0x88) and capture(origin, dest, board)) {
			capture_from[capture_num_moves] = origin;
            capture_to[capture_num_moves++] = dest;
            //num_captures++;
		}

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

		// handle enpassant moves
		if (enpassant != 127) {
            if (board[origin] == 100) {
                if ((origin - 17) == enpassant and !((origin - 17)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 703;
                } else if ((origin - 15) == enpassant and !((origin - 15)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 704;
                }
            } else {
                if ((origin + 15) == enpassant and !((origin + 15)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 701;
                } else if ((origin + 17) == enpassant and !((origin + 17)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 702;
                }
            }
		}
	}

}

//////////////////////////////////////////////
//
// Quiescence Only-attack
//
/////////////////////////////////////////////

MovesGenerator::attacks_knight_moves(int* board, int* capture_from, int* capture_to, int& capture_num_moves, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
		for (int j = 0; j < 8; j++) {
			dest = origin + dir.knight[j];
			if ( (dest & 0x88) != 0) continue;
			if ( board[dest] == 0) {
//				from[num_moves] = origin;
//				to[num_moves++] = dest;
			} else if (capture(origin, dest, board)) {
			    capture_from[capture_num_moves] = origin;
                capture_to[capture_num_moves++] = dest;
			}
		}
    }
}


MovesGenerator::attacks_straight_moves(int* board, int* capture_from, int* capture_to, int& capture_num_moves, int* dir, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
		for (int d = 0; d < 4; d++) {
			dest = origin;
			for (int j = 0; j < 7; j++) {
				dest += dir[d];
				if ( (dest & 0x88) != 0) break;
				if ( board[dest] == 0) {
//					from[num_moves] = origin;
//                    to[num_moves++] = dest;
				} else {
					if (capture(origin, dest, board)) {
						capture_from[capture_num_moves] = origin;
                        capture_to[capture_num_moves++] = dest;
					}
					break;
				}
			}
		}
	}
}



MovesGenerator::attacks_king_moves(int* board, int* capture_from, int* capture_to, int& capture_num_moves, int origin) {
	int dest;
	if (origin != DELETED) {
		for (int j = 0; j < 8; j++) {
			dest = origin + dir.king[j];
			if ( (dest & 0x88) != 0) continue;
			if ( board[dest] == 0) {
//				from[num_moves] = origin;
//                to[num_moves++] = dest;
			} else if (capture(origin, dest, board)) {
				capture_from[capture_num_moves] = origin;
                capture_to[capture_num_moves++] = dest;
			}
		}
	}
}


MovesGenerator::attacks_pawn_moves(int* board, int* capture_from, int* capture_to, int& capture_num_moves, int origin, int king_l) {
	int dest;
	if (origin != DELETED) {
        if (board[origin] > 0 and origin >= 16 and origin <= 23) {
            // mid
            dest = origin - 16;
            if (board[dest] == 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1002;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1001;
            }
            // left
            dest = origin - 17;
            if ( !(dest & 0x88) and board[dest] < 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2008;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2006;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2002;
            }
            // right
            dest = origin - 15;
            if ( !(dest & 0x88) and board[dest] < 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2007;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2005;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2001;
            }
            return 0;
        } else if (board[origin] < 0 and origin >= 96 and origin <= 103) {
            dest = origin + 16;
            if (board[dest] == 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1002;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 1001;
            }
            // left
            dest = origin + 15;
            if ( !(dest & 0x88) and board[dest] > 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2008;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2006;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2004;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2002;
            }
            // right
            dest = origin + 17;
            if ( !(dest & 0x88) and board[dest] > 0) {
                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2007;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2005;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2003;
//                capture_from[capture_num_moves] = origin; capture_to[capture_num_moves++] = 2001;
            }
            return 0;
        }


		int r1, r2;
		int d1, d2;
		int left_bound,right_bound;

		if (board[origin] > 0) {
			r1 = -16;
			r2 = -32;
			d1 = -17;
			d2 = -15;
			left_bound = 96;
			right_bound = 103;
		} else {
			r1 = +16;
			r2 = +32;
			d1 = +17;
			d2 = +15;
			left_bound = 16;
			right_bound = 23;
		}

		//for diagonal captures
		dest = origin + d1;
		if (!(dest & 0x88) and capture(origin, dest, board)) {
			capture_from[capture_num_moves] = origin;
            capture_to[capture_num_moves++] = dest;
            //num_captures++;
		}
		dest = origin + d2;
		if (!(dest & 0x88) and capture(origin, dest, board)) {
			capture_from[capture_num_moves] = origin;
            capture_to[capture_num_moves++] = dest;
            //num_captures++;
		}

		// handle enpassant moves
		if (enpassant != 127) {
            if (board[origin] == 100) {
                if ((origin - 17) == enpassant and !((origin - 17)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 703;
                } else if ((origin - 15) == enpassant and !((origin - 15)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 704;
                }
            } else {
                if ((origin + 15) == enpassant and !((origin + 15)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 701;
                } else if ((origin + 17) == enpassant and !((origin + 17)&0x88)) {
                    capture_from[capture_num_moves] = origin;
                    capture_to[capture_num_moves++] = 702;
                }
            }
		}
	}

}




bool MovesGenerator::is_in_check(int* board, int king_pos, int shift) {
    int king, queen, rook, knight, bishop, pawn;
    int dest;

    if (shift == 1) {
        king = -8888;
        queen = -900;
        rook = -500;
        knight = -310;
        bishop = -300;
        pawn = -100;
    } else {
        king = 8888;
        queen = 900;
        rook = 500;
        knight = 310;
        bishop = 300;
        pawn = 100;
    }


    for (int i = 0; i < 8; i++) {
        dest = king_pos + dir.knight[i];
        if ( (dest & 0x88) !=  0) continue;
        if (board[dest] == knight) {
            return true;
        }
    }

    for (int i = 0; i < 8; i++) {
        dest = king_pos + dir.king[i];
        if ( (dest & 0x88) !=  0) continue;
        if (board[dest] == king) {
            return true;
        }
    }


    for (int i = 0; i < 4; i++) {
        dest = king_pos;
        for (int j = 0; j < 7; j++) {
            dest += dir.vertical[i];
            if ( (dest & 0x88) == 0) {
                if (board[dest] == 0) continue;
                else {
                    if (board[dest] == queen or board[dest] == rook) {
                        return true;
                    }
                    break;
                }
            } else {
                break;
            }
        }

        dest = king_pos;
        for (int j = 0; j < 7; j++) {
            dest += dir.diagonal[i];
            if ( (dest & 0x88) == 0) {
                if (board[dest] == 0) continue;
                else {
                    if (board[dest] == queen or board[dest] == bishop) {
                        return true;
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }

    if (shift == 1) {
        dest = king_pos - 15;
        if ( (dest & 0x88) == 0) {
            if (board[dest] == pawn) {
                return true;
            }
        }
        dest = king_pos - 17;
        if ( (dest & 0x88) == 0) {
            if (board[dest] == pawn) {
                return true;
            }
        }
    } else {
        dest = king_pos + 15;
        if ( (dest & 0x88) == 0) {
            if (board[dest] == pawn) {
                return true;
            }
        }
        dest = king_pos + 17;
        if ( (dest & 0x88) == 0) {
            if (board[dest] == pawn) {
                return true;
            }
        }
    }

    return false;
}

MovesGenerator::generate_moves(int* board, int* from, int* to, int shift, int& num_moves) {
	num_moves = 0;
	int origin;
	int dest;
}


void MovesGenerator::save_killer(int from, int to, int depth) {
    killer_moves[depth][2] = killer_moves[depth][0];
    killer_moves[depth][3] = killer_moves[depth][1];

    killer_moves[depth][0] = from;
    killer_moves[depth][1] = to;
}

void MovesGenerator::check_killer(int from, int to) {
    if (killer_moves[depth][0] == from and killer_moves[depth][1] == to) {
        killer1_from = from;
        killer1_to = to;
        killer1_found = true;
    }
    else if (killer_moves[depth][2] == from and killer_moves[depth][3] == to) {
        killer2_from = from;
        killer2_to = to;
        killer2_found = true;
    }
}
