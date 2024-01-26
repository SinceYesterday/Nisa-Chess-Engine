#include "Search.h"

#define WHITE  1
#define BLACK -1

#define INFINITE 1000000000

#define WKING 8888
#define WQUEEN 900
#define WROOK 500
#define WBISHOP 300
#define WKNIGHT 310
#define WPAWN 100

#define BKING -8888
#define BQUEEN -900
#define BROOK -500
#define BBISHOP -300
#define BKNIGHT -310
#define BPAWN -100

Search::Search()
{
    moves_generator = new MovesGenerator();
    init();
}

Search::~Search()
{
    //dto
}

void Search::init() {

    // black
    // left
    promotion_offset[2008] = +15;
    promotion_offset[2006] = +15;
    promotion_offset[2004] = +15;
    promotion_offset[2002] = +15;

    // mid
    promotion_offset[1004] = +16;
    promotion_offset[1003] = +16;
    promotion_offset[1002] = +16;
    promotion_offset[1001] = +16;

    // right
    promotion_offset[2007] = +17;
    promotion_offset[2005] = +17;
    promotion_offset[2003] = +17;
    promotion_offset[2001] = +17;

    // white
    // left
    promotion_offset[16 + 2008] = -17;
    promotion_offset[16 + 2006] = -17;
    promotion_offset[16 + 2004] = -17;
    promotion_offset[16 + 2002] = -17;

    // mid
    promotion_offset[16 + 1004] = -16;
    promotion_offset[16 + 1003] = -16;
    promotion_offset[16 + 1002] = -16;
    promotion_offset[16 + 1001] = -16;

    // right
    promotion_offset[16 + 2007] = -15;
    promotion_offset[16 + 2005] = -15;
    promotion_offset[16 + 2003] = -15;
    promotion_offset[16 + 2001] = -15;

    for (int i = 0; i < 901; i++) {
        game_phase_inc[i] = 0;
    }

    // game phase increment
    game_phase_inc[300] = 1;
    game_phase_inc[310] = 1;
    game_phase_inc[500] = 2;
    game_phase_inc[900] = 4;

    // PeSTO piece value
    for (int i = 0; i < 128; i++) {
        if (!(i&0x88)) {
            // white
            mg_pawn_white[i] += 82;
            eg_pawn_white[i] += 94;

            mg_knight_white[i] += 337;
            eg_knight_white[i] += 281;

            mg_bishop_white[i] += 365;
            eg_bishop_white[i] += 297;

            mg_rook_white[i] += 477;
            eg_rook_white[i] += 512;

            mg_queen_white[i] += 1025;
            eg_queen_white[i] += 936;

            mg_king_white[i] += 8888;
            eg_king_white[i] += 8888;

            // black
            mg_pawn_black[i] += 82;
            eg_pawn_black[i] += 94;

            mg_knight_black[i] += 337;
            eg_knight_black[i] += 281;

            mg_bishop_black[i] += 365;
            eg_bishop_black[i] += 297;

            mg_rook_black[i] += 477;
            eg_rook_black[i] += 512;

            mg_queen_black[i] += 1025;
            eg_queen_black[i] += 936;

            mg_king_black[i] += 8888;
            eg_king_black[i] += 8888;
        }
    }

    // incremental pst
    mg_pst_inc[0] = null_board;
    eg_pst_inc[0] = null_board;

    // white
    mg_pst_inc[96] = mg_pawn_white;
    mg_pst_inc[40] = mg_bishop_white;
    mg_pst_inc[32] = mg_knight_white;
    mg_pst_inc[224] = mg_rook_white;
    mg_pst_inc[128] = mg_queen_white;
    mg_pst_inc[168] = mg_king_white;

    eg_pst_inc[96] = eg_pawn_white;
    eg_pst_inc[40] = eg_bishop_white;
    eg_pst_inc[32] = eg_knight_white;
    eg_pst_inc[224] = eg_rook_white;
    eg_pst_inc[128] = eg_queen_white;
    eg_pst_inc[168] = eg_king_white;

    // black
    mg_pst_inc[136] = mg_pawn_black;
    mg_pst_inc[192] = mg_bishop_black;
    mg_pst_inc[200] = mg_knight_black;
    mg_pst_inc[8] = mg_rook_black;
    mg_pst_inc[104] = mg_queen_black;
    mg_pst_inc[72] = mg_king_black;

    eg_pst_inc[136] = eg_pawn_black;
    eg_pst_inc[192] = eg_bishop_black;
    eg_pst_inc[200] = eg_knight_black;
    eg_pst_inc[8] = eg_rook_black;
    eg_pst_inc[104] = eg_queen_black;
    eg_pst_inc[72] = eg_king_black;

    // pawn structure
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            pawn_struct[i][j] = 0;
        }
    }

    for (int i = 0; i < 128; i++) {
        if (!(i&0x88)) {
            for (int j = 0; j < 8; j++) {
                int square = i - (16 * j);
                if ((square&0x88)) break;
                pawn_struct[i][square] = 1;
            }
            for (int j = 0; j < 8; j++) {
                int square = i + (16 * j);
                if ((square&0x88)) break;
                pawn_struct[i][square] = 1;
            }
        }
    }


    // zobrist initialize keys
    srand(time(0));
	int range_from  = 0;
    int range_to    = LLONG_MAX;
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<unsigned long long>  distr(range_from, range_to);

    zobrist_black_moved = distr(generator);
    zobrist_white_moved = distr(generator);

	std::vector<int> mat {100, 300, 310, 500, 900, 8888};
	for (int _side = 0; _side < 2; _side++) {
		for (int m = 0; m < 6; m++) {
			for (int x = 0; x < 128; x++) {
				if (!(x & 0x88)) {
					zobrist_id[_side][mat[m]][x] =  distr(generator);
				}
			}
		}
	}

    for (int i = 0; i < 128; i++) {
        zobrist_id[0][0][i] = 0;
        zobrist_id[1][0][i] = 0;
	}

	// castle
	for (int i = 0; i < 4; i++) {
        zobrist_castle[i] = distr(generator);
	}

	// enpassant
	for (int i = 0; i < 128; i++) {
        zobrist_enpassant[i] = distr(generator);
	}

}

void Search::print_board() {
    std::cout << std::endl;
	for ( int i = 1; i <= 128; i++) {
		if ( ( (i - 1) & 0x88) == 0 ) {
			if (board[i - 1] == 0) {
				std::cout << " - ";
			} else {
				std::cout << " "  << weight_to_char(board[i - 1]) << " ";
			}
		}
		if ( i % 8 == 0)
			std::cout << "   ";

		if ( i % 16 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}


void Search::print_dboard() {
    std::cout << std::endl;
	for ( int i = 1; i <= 128; i++) {
		if ( ( (i - 1) & 0x88) == 0 ) {
			if (dboard[i - 1] == 0) {
				std::cout << " - ";
			} else {
				std::cout << " 1 ";
			}
		}
		if ( i % 8 == 0)
			std::cout << "   ";

		if ( i % 16 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

char Search::weight_to_char(int weight)
{
    if (weight == WKING) return 'K';
    else if (weight == WQUEEN) return 'Q';
    else if (weight == WROOK) return 'R';
    else if (weight == WBISHOP) return 'B';
    else if (weight == WKNIGHT) return 'N';
    else if (weight == WPAWN) return 'P';

    else if (weight == BKING) return 'k';
    else if (weight == BQUEEN) return 'q';
    else if (weight == BROOK) return 'r';
    else if (weight == BBISHOP) return 'b';
    else if (weight == BKNIGHT) return 'n';
    else if (weight == BPAWN) return 'p';
}


void Search::set_board_e(std::string FEN_here) {

	for (int i = 0; i < 128; i++) {
		board[i] = 0;
	}

	int i_setter = 0;

	auto set_piece = [] (int* board_, int& i_setter, int weight) {
		if (i_setter < 128) {
			board_[i_setter] = weight;
			i_setter++;
		} else {
			std::cout << "FEN ERROR" << std::endl;
		}

	};

	for (int i = 0; i < FEN_here.length(); i++) {
		if (FEN_here[i] == '/') {
			i_setter += 8;
		} else if (FEN_here[i] == 'K') {
			set_piece(board, i_setter, WKING);
		} else if (FEN_here[i] == 'Q') {
			set_piece(board, i_setter, WQUEEN);
		} else if (FEN_here[i] == 'R') {
			set_piece(board, i_setter, WROOK);
		} else if (FEN_here[i] == 'B') {
			set_piece(board, i_setter, WBISHOP);
		} else if (FEN_here[i] == 'N') {
			set_piece(board, i_setter, WKNIGHT);
		} else if (FEN_here[i] == 'P') {
			set_piece(board, i_setter, WPAWN);
		}

		else if (FEN_here[i] == 'k') {
			set_piece(board, i_setter, BKING);
		} else if (FEN_here[i] == 'q') {
			set_piece(board, i_setter, BQUEEN);
		} else if (FEN_here[i] == 'r') {
			set_piece(board, i_setter, BROOK);
		} else if (FEN_here[i] == 'b') {
			set_piece(board, i_setter, BBISHOP);
		} else if (FEN_here[i] == 'n') {
			set_piece(board, i_setter, BKNIGHT);
		} else if (FEN_here[i] == 'p') {
			set_piece(board, i_setter, BPAWN);
		}

		else if (isdigit(FEN_here[i])) {
			int n = FEN_here[i] - '0';
			i_setter += n;
		}
	}
}


void Search::start_search(int* board, Board::Pieces_position* wpieces, Board::Pieces_position* bpieces) {

    // time management
    start_time = std::clock();
    time_over = false;

    // metricts
    zobrist_key = 0;
    zobrist_table_state = 0;
    num_positions_generated = 0;
    random_state = 1804289383;
    best_from = -1;
    best_to = -1;

    zobrist_key_saved = 0;
    zobrist_key_duplicated = 0;
    bad_keys = 0;
    recycled_keys = 0;
    has_to_research = 0;
    num_moves_sum = 0;
    extra_evaluation = 0;


    white_mat_score = 0;
    black_mat_score = 0;

    for (int i = 0; i < 128; i++) {
        this->board[i] = board[i];
    }

    // counts for promotion usage
    white_queen_count = 0;
    white_rook_count = 0;
    white_knight_count = 0;
    white_bishop_count = 0;
    white_pawn_count = 0;
    black_queen_count = 0;
    black_rook_count = 0;
    black_knight_count = 0;
    black_bishop_count = 0;
    black_pawn_count = 0;

    // count pieces in the board
    for (int i = 0; i < 128; i++) {
        if ( (0x88 & i) == 0 ) {
                 if (board[i] == 900) white_queen_count++;
            else if (board[i] == 500) white_rook_count++;
            else if (board[i] == 310) white_knight_count++;
            else if (board[i] == 300) white_bishop_count++;
            else if (board[i] == 100) white_pawn_count++;

            else if (board[i] == -900) black_queen_count++;
            else if (board[i] == -500) black_rook_count++;
            else if (board[i] == -310) black_knight_count++;
            else if (board[i] == -300) black_bishop_count++;
            else if (board[i] == -100) black_pawn_count++;
        }
    }


    for (int i = 0; i < 128; i++) {
        if (!(i & 0x88)) {
            if (this->board[i] > 0) {
                white_mat_score += this->board[i];
            } else if (this->board[i] < 0) {
                black_mat_score += this->board[i];
            }
        }
    }

    search_wpieces.king = wpieces->king;

    for (int i = 0; i < 8; i++) {
        search_wpieces.queen[i] = wpieces->queen[i];
        search_wpieces.rook[i] = wpieces->rook[i];
        search_wpieces.bishop[i] = wpieces->bishop[i];
        search_wpieces.knight[i] = wpieces->knight[i];
    }

    for (int i = 0; i < 8; i++) {
        search_wpieces.pawn[i] = wpieces->pawn[i];
    }


    search_bpieces.king = bpieces->king;

    for (int i = 0; i < 8; i++) {
        search_bpieces.queen[i] = bpieces->queen[i];
        search_bpieces.rook[i] = bpieces->rook[i];
        search_bpieces.bishop[i] = bpieces->bishop[i];
        search_bpieces.knight[i] = bpieces->knight[i];
    }

    for (int i = 0; i < 8; i++) {
        search_bpieces.pawn[i] = bpieces->pawn[i];
    }


    // populate dboard
    for (int i = 0; i < 128; i++) {
        dboard[i] = 0;
    }
    if (search_wpieces.king != DELETED)
        dboard[search_wpieces.king] = &search_wpieces.king;

    for (int i = 0; i < 8; i++) {
        if (search_wpieces.rook[i] != DELETED)
            dboard[search_wpieces.rook[i]] =  &search_wpieces.rook[i];

        if (search_wpieces.bishop[i] != DELETED)
            dboard[search_wpieces.bishop[i]] =  &search_wpieces.bishop[i];

        if (search_wpieces.knight[i] != DELETED)
            dboard[search_wpieces.knight[i]] =  &search_wpieces.knight[i];

        if (search_wpieces.queen[i] != DELETED)
        dboard[search_wpieces.queen[i]] = &search_wpieces.queen[i];
    }

    for (int i = 0; i < 8; i++) {
        if (search_wpieces.pawn[i] != DELETED)
            dboard[search_wpieces.pawn[i]] =  &search_wpieces.pawn[i];
    }


    if (search_bpieces.king != DELETED)
        dboard[search_bpieces.king] = &search_bpieces.king;

    for (int i = 0; i < 8; i++) {
        if (search_bpieces.rook[i] != DELETED)
            dboard[search_bpieces.rook[i]] =  &search_bpieces.rook[i];

        if (search_bpieces.bishop[i] != DELETED)
            dboard[search_bpieces.bishop[i]] =  &search_bpieces.bishop[i];

        if (search_bpieces.knight[i] != DELETED)
            dboard[search_bpieces.knight[i]] =  &search_bpieces.knight[i];

        if (search_bpieces.queen[i] != DELETED)
        dboard[search_bpieces.queen[i]] = &search_bpieces.queen[i];
    }

    for (int i = 0; i < 8; i++) {
        if (search_bpieces.pawn[i] != DELETED)
            dboard[search_bpieces.pawn[i]] =  &search_bpieces.pawn[i];
    }


    // populate pst eval
    mg_eval[0] = 0;
    mg_eval[1] = 0;
    eg_eval[0] = 0;
    eg_eval[1] = 0;
    game_phase = 0;
    for (int i = 0; i < 128; i++) {
        if (!(i&0x88)) {
            if (board[i] > 0) {
                mg_eval[1] += mg_pst_inc[board[i] & 232][i];
                eg_eval[1] += eg_pst_inc[board[i] & 232][i];
                if (board[i] < 8888)
                    game_phase += game_phase_inc[abs(board[i])];
            }
            else if (board[i] < 0) {
                mg_eval[0] += mg_pst_inc[board[i] & 232][i];
                eg_eval[0] += eg_pst_inc[board[i] & 232][i];
                if (board[i] > -8888)
                    game_phase += game_phase_inc[abs(board[i])];
            }
        }
    }

    // populate move ordering tables, game_phase is required
    populate_move_ordering_tables();

    // show board
//    print_board();
//    print_dboard();


    start_zobrist();

    // metrics
    num_nodes = 0;
    num_nodes_quiescence = 0;
    max_depth_rearched = 0;

    num_moves_sum = 0;
    extra_evaluation = 0;
    TT_move_recovered = 0;
    TT_move_cuts = 0;
    alpha_beta_cuts = 0;
    first_cut_alpha_beta = 0;
    num_positions_generated = 0;
    repetition_tt_colitions = 0;


//    // perft test
//    // init data
//    num_nodes = 0;
//    num_leaf = 0;
//    check = 0;
//    check_mate = 0;
//    num_captures = 0;
//    num_enpassant = 0;
//    num_castle = 0;
//    num_promotion = 0;
//    flag_capture = 0;
//    zobrist_key_duplicated = 0;
//    recycled_keys = 0;
//    TT_move_recovered = 0;
//    moves_generator->enpassant = 127;
//
//    std::vector<std::pair<int, int>> main_line;
//
//    // fen extra data
//    moves_generator->enpassant = 127;
//    for (int i = 0; i < 4; i++) {
//        moves_generator->castle_rights[i] = 1;
//    }
//
//    // go
//    MAX_DEPTH = 6;
//    perft_depth = MAX_DEPTH;
//    long long val = minimax(WHITE, 1, -88888888888, 88888888888, main_line);
//
//    // results
//    std::cout << "perft results " << std::endl;
//    std::cout << "leaft " << num_leaf << std::endl;
//    std::cout << "check " << check << std::endl;
//    //std::cout << "check mate " << check_mate << std::endl;
//    std::cout << "captures " << num_captures << std::endl;
//    std::cout << "enpassant " << num_enpassant << std::endl;
//
//    std::cout << "castle " << num_castle << std::endl;
//    std::cout << "promotion " << num_promotion << std::endl;


    for (int i = 0; i < 128; i++) {
        branch[i] = 0;
    }

    //clear killers
    for (int i = 0; i < 88; i++) {
        for (int j = 0; j < 4; j++) {
            moves_generator->killer_moves[i][j] = -1;
        }
    }

    // reset history
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 128; j++) {
			for (int k = 0; k < 128; k++) {
				history[i][j][k] = 0;
			}
		}
	}


    while (!pv_queue.empty()) {
        pv_queue.pop();
    }

    int enpassant_state = moves_generator->enpassant;
    int castle_rights_state[4];
    for (int i = 0; i < 4; i++) {
        castle_rights_state[i] = moves_generator->castle_rights[i];
    }

// ------------------------

    #define valWINDOW  50
    int alpha = -INFINITE;
    int beta = INFINITE;

    //time_limit = 4000000;
    wrong_pv_move_count = 0;
    wrong_tt_move_count = 0;
    int prev_val;
	for (int cur_depth = 2; cur_depth < 64 and !time_over; cur_depth++) { //use real depth
        num_nodes = 0;

        // reset branching factor metrics
        for (int i = 0; i < 64; i++) branch[i] = 0;

        moves_generator->enpassant = enpassant_state;
        for (int i = 0; i < 4; i++) {
            moves_generator->castle_rights[i] = castle_rights_state[i];
        }

		MAX_DEPTH = cur_depth;

		std::clock_t start_time = std::clock();

		std::vector<std::pair<int, int>> main_line;
		int val;
		if (side == 1) {
            val = minimax(WHITE, 1, alpha, beta, main_line);
		} else {
            val = minimax(BLACK, 1, alpha, beta, main_line);
		}

		std::clock_t end_time = std::clock();

		// calculate nodes per second
		double elapsed_time = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
		if (elapsed_time == 0) elapsed_time = 0.00022 * cur_depth;
		double operations_per_second = num_nodes / (elapsed_time);
        int nps = operations_per_second;

		while (!pv_queue.empty()) {
            pv_queue.pop();
        }

        // check if last iteration was completed if not then dont show current iteration info
        // best_move of previous iteration is keep and proved first, so if new iteration ends prematurely
        // we will have a better evaluation of best_move
        if (val != -INFINITE) {
            std::cout << std::endl << "info score cp " << val << " depth " << cur_depth - 1 << " selfdepth " << max_depth_rearched << " nodes " << num_nodes << " nps " << nps << " pv ";
        }
		for (int PI = 0; PI < main_line.size(); PI++) {
            std::cout << show_pv_move(main_line[PI].first, main_line[PI].second, side) << " ";
            pv_queue.push(main_line[PI]);
		}
		prev_val = val;

		// calculate branch factor for time management
		float cur_branch = 0;
		int num_depth = 0;
		for (int i = 0; i < 64; i++) {
            if (branch[i] != 0) {
                num_depth++;
                if (branch[i - 1] != 0) {
                    cur_branch += branch[i] / branch[i - 1];
                }
            }
		}

		cur_branch = cur_branch / (num_depth - 1);

        std::clock_t time_ate = std::clock() - start_time;

        // try to predict if we can complete the next iteration in time, end search otherwise
        if ( (time_ate*2 + (time_ate * std::ceil(cur_branch)) > time_limit)) break;

	}

	std::cout << std::endl <<  "bestmove ";
	// show best move and pv line
    std::cout << show_pv_move(best_from, best_to, side) << std::endl;

    time_over = true;

}

std::string Search::show_pv_move(int best_from, int best_to, int _side) {
    std::string move = "";
    std::string x88_to_alg[128] = {
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",    "","","","","","","","",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",    "","","","","","","","",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",    "","","","","","","","",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",    "","","","","","","","",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",    "","","","","","","","",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",    "","","","","","","","",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",    "","","","","","","","",
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",    "","","","","","","",""
	};

	if (best_to == 4001) {
        move = "e8c8";
    }
    else if (best_to == 4002) {
        move = "e8g8";
    }
    else if (best_to == 4003) {
        move = "e1c1";
    }
    else if (best_to == 4004) {
        move = "e1g1";
    }
    else if (best_to == 701) {
        move += x88_to_alg[best_from];
        move += x88_to_alg[best_from + 15];
    }
    else if(best_to == 702) {
        move += x88_to_alg[best_from];
        move += x88_to_alg[best_from + 17];
    }
    else if(best_to == 703) {
        move += x88_to_alg[best_from];
        move += x88_to_alg[best_from - 17];
    }
    else if(best_to == 704) {
        move += x88_to_alg[best_from];
        move += x88_to_alg[best_from - 15];
    }
    else if (best_to > 1000 and best_to < 2010) {
        int real_best_to = best_from + promotion_offset[((_side == 1 ? 1 : 0) << 4) + best_to];
        move += x88_to_alg[best_from];
        move += x88_to_alg[real_best_to];
        if (_side == 1) {
            if (best_to == 2008 or best_to == 1004 or best_to == 2007) {
                move += "Q";
            }
            else if (best_to == 2006 or best_to == 1003 or best_to == 2005) {
                move += "R";
            }
            else if (best_to == 2004 or best_to == 1002 or best_to == 2003) {
                move += "N";
            }
            else if (best_to == 2002 or best_to == 1001 or best_to == 2001) {
                move += "B";
            }
        } else {
            if (best_to == 2008 or best_to == 1004 or best_to == 2007) {
                move += "q";
            }
            else if (best_to == 2006 or best_to == 1003 or best_to == 2005) {
                move += "r";
            }
            else if (best_to == 2004 or best_to == 1002 or best_to == 2003) {
                move += "n";
            }
            else if (best_to == 2002 or best_to == 1001 or best_to == 2001) {
                move += "b";
            }
        }
    }
    else {
        move += x88_to_alg[best_from];
        move += x88_to_alg[best_to];
    }
    return move;
}


unsigned long long Search::generate_zobrist_key(int shift, int depth){
    unsigned long long key = 0;

    for (int i = 0; i < 128; i++) {
		if (!(i & 0x88)) {
			int p = board[i];

			if (p == WKING) {
				key ^= zobrist_id[1][WKING][i];
				if (shift == 3) std::cout << zobrist_id[1][WKING][i] << "^";
			} else if (p == WQUEEN) {
				key ^= zobrist_id[1][WQUEEN][i];
				if (shift == 3) std::cout << zobrist_id[1][WQUEEN][i] << "^";
			} else if (p == WROOK) {
				key ^= zobrist_id[1][WROOK][i];
				if (shift == 3) std::cout << zobrist_id[1][WROOK][i] << "^";
			} else if (p == WKNIGHT) {
				key ^= zobrist_id[1][WKNIGHT][i];
				if (shift == 3) std::cout << zobrist_id[1][WKNIGHT][i] << "^";
			} else if (p == WBISHOP) {
				key ^= zobrist_id[1][WBISHOP][i];
				if (shift == 3) std::cout << zobrist_id[1][WBISHOP][i] << "^";
			} else if (p == WPAWN) {
				key ^= zobrist_id[1][WPAWN][i];
				if (shift == 3) std::cout << zobrist_id[1][WPAWN][i] << "^";
			}

			else if (p == BKING) {
                key ^= zobrist_id[0][-BKING][i];
                if (shift == 3) std::cout << zobrist_id[0][-BKING][i] << "^";
			} else if (p == BQUEEN) {
				key ^= zobrist_id[0][-BQUEEN][i];
				if (shift == 3) std::cout << zobrist_id[0][-BQUEEN][i] << "^";
			} else if (p == BROOK) {
				key ^= zobrist_id[0][-BROOK][i];
				if (shift == 3) std::cout << zobrist_id[0][-BROOK][i] << "^";
			} else if (p == BKNIGHT) {
				key ^= zobrist_id[0][-BKNIGHT][i];
				if (shift == 3) std::cout << zobrist_id[0][-BKNIGHT][i] << "^";
			} else if (p == BBISHOP) {
				key ^= zobrist_id[0][-BBISHOP][i];
				if (shift == 3) std::cout << zobrist_id[0][-BBISHOP][i] << "^";
			} else if (p == BPAWN) {
				key ^= zobrist_id[0][-BPAWN][i];
				if (shift == 3) std::cout << zobrist_id[0][-BPAWN][i] << "^";
			}
		}
	}

	return key;
}


void Search::generate_FEN() {

    int board[64];
    int j = 0;
    for (int i = 0; i < 128; i++) {
        if (!(i & 0x88)) {
            board[j] = this->board[i];
            j++;
        }
    }

	int space = 0;
	FEN = "";
	int c = 0;
	for (int i = 1; i <= 64; i++) {

		if (board[i - 1] != 0) {
			c++;
			if (space != 0) {
				FEN += space + '0';
			}
			space = 0;
		}
		if (board[i - 1] == WKING) FEN += 'K';
		else if (board[i - 1] == WQUEEN) FEN += 'Q';
		else if (board[i - 1] == WROOK) FEN += 'R';
		else if (board[i - 1] == WBISHOP) FEN += 'B';
		else if (board[i - 1] == WKNIGHT) FEN += 'N';
		else if (board[i - 1] == WPAWN) FEN += 'P';

		else if (board[i - 1] == BKING) FEN += 'k';
		else if (board[i - 1] == BQUEEN) FEN += 'q';
		else if (board[i - 1] == BROOK) FEN += 'r';
		else if (board[i - 1] == BBISHOP) FEN += 'b';
		else if (board[i - 1] == BKNIGHT) FEN += 'n';
		else if (board[i - 1] == BPAWN) FEN += 'p';
		else if (board[i - 1] == 0) space++;

		if ( (i % 8) == 0) {
			if (space != 0) {
				FEN += (space + '0');
			}
			FEN += '/';
			space = 0;
		}
	}
}

void Search::start_zobrist() {

    zobrist_key = 0;
    zobrist_key_duplicated = 0;
    zobrist_key_saved = 0;
    bad_keys = 0;
    recycled_keys = 0;
    has_to_research = 0;

	if (moves_generator->enpassant != 127) {
        zobrist_key ^= zobrist_enpassant[moves_generator->enpassant];
	}

	for (int i = 0; i < 4; i++) {
        if (moves_generator->castle_rights[i] == 1) {
            zobrist_key ^= zobrist_castle[i];
        }
	}

    for (int i = 0; i < zobrist_tt_size; i++) {
       zobrist_tt[i].zobrist_key = 0;
       zobrist_tt[i].score = 0;
       zobrist_tt[i].depth = -1000000;
       zobrist_tt[i].best_from = -1;
       zobrist_tt[i].best_to = -1;
       zobrist_tt[i].flags = -1;
    }


    for (int i = 0; i < zobrist_tt_size; i++) {
       zobrist_tt_2nd[i].zobrist_key = 0;
       zobrist_tt_2nd[i].score = 0;
       zobrist_tt_2nd[i].depth = -1000000;
       zobrist_tt_2nd[i].best_from = -1;
       zobrist_tt_2nd[i].best_to = -1;
       zobrist_tt_2nd[i].flags = -1;
    }

     for (int i = 0; i < zobrist_tt_size; i++) {
       zobrist_tt_3nd[i].zobrist_key = 0;
       zobrist_tt_3nd[i].score = 0;
       zobrist_tt_3nd[i].depth = -1000000;
       zobrist_tt_3nd[i].best_from = -1;
       zobrist_tt_3nd[i].best_to = -1;
       zobrist_tt_3nd[i].flags = -1;
    }

	for (int i = 0; i < 128; i++) {
		if (!(i & 0x88)) {
			int p = board[i];

			if (p == WKING) {
				zobrist_key ^= zobrist_id[1][WKING][i];
			} else if (p == WQUEEN) {
				zobrist_key ^= zobrist_id[1][WQUEEN][i];
			} else if (p == WROOK) {
				zobrist_key ^= zobrist_id[1][WROOK][i];
			} else if (p == WKNIGHT) {
				zobrist_key ^= zobrist_id[1][WKNIGHT][i];
			} else if (p == WBISHOP) {
				zobrist_key ^= zobrist_id[1][WBISHOP][i];
			} else if (p == WPAWN) {
				zobrist_key ^= zobrist_id[1][WPAWN][i];
			}

			else if (p == BKING) {
				zobrist_key ^= zobrist_id[0][-BKING][i];
			} else if (p == BQUEEN) {
				zobrist_key ^= zobrist_id[0][-BQUEEN][i];
			} else if (p == BROOK) {
				zobrist_key ^= zobrist_id[0][-BROOK][i];
			} else if (p == BKNIGHT) {
				zobrist_key ^= zobrist_id[0][-BKNIGHT][i];
			} else if (p == BBISHOP) {
				zobrist_key ^= zobrist_id[0][-BBISHOP][i];
			} else if (p == BPAWN) {
				zobrist_key ^= zobrist_id[0][-BPAWN][i];
			}
		}
	}

	if (side == WHITE) {
        zobrist_key ^= zobrist_black_moved;
	} else {
        zobrist_key ^= zobrist_white_moved;
	}


    zobrist_key_duplicated = 0;
    zobrist_key_saved = 0;

}

int Search::minimax(int shift, int depth, int alpha, int beta, std::vector<std::pair<int, int>>& main_line) {
    main_line.clear();

    // peft test

//    int temp_MAX_DEPTH = MAX_DEPTH;
//    if (moves_generator->is_in_check(board, shift == 1? search_bpieces.king : search_wpieces.king, shift == 1? -1 : 1)) {
//        return 555555555;
//    }

//    if (moves_generator->is_in_check(board, shift == WHITE? search_wpieces.king : search_bpieces.king, shift)) {
//        if (depth == perft_depth)
//            check++;
//    }


    // ---

    int temp_MAX_DEPTH = MAX_DEPTH;
    bool im_in_check = moves_generator->is_in_check(board, shift == WHITE? search_wpieces.king : search_bpieces.king, shift == WHITE? 1 : -1);
    if (im_in_check) MAX_DEPTH++;

    if (depth >= MAX_DEPTH) {

        //num_nodes++;

        // perft test

//        if (depth == perft_depth) {
//            num_leaf++;
//            if (flag_capture) num_captures++;
//            if (flag_enpassant) num_enpassant++;
//            if (flag_castle) num_castle++;
//            if (flag_promotion) num_promotion++;
//        }
//
//
//        return 0;

        // ------------

        int val = quiescence(shift, alpha, beta, depth);
        // no best move in leaf
        TT_move_best_from = -1;
        TT_move_best_to = -1;
		return val;
    }

    // null move
    int remaining_depth = (MAX_DEPTH - depth) - 1;
    int lazy_eval = evaluate(shift);
	if (depth > 1 and lazy_eval > (beta) and remaining_depth >= 3 and  (game_phase > 0) ) {
		if (!im_in_check) {
            int R = 3;
			std::vector<std::pair<int, int>> temp_line;

			unsigned long long temp_zobrist = zobrist_key;

			int temp_enpassant = moves_generator->enpassant;

			if (moves_generator->enpassant != 127) {
                zobrist_key ^= zobrist_enpassant[moves_generator->enpassant];
            }
			moves_generator->enpassant = 127;

            if (shift == WHITE) {
                zobrist_key ^= zobrist_black_moved;
                zobrist_key ^= zobrist_white_moved;
            } else {
                zobrist_key ^= zobrist_black_moved;
                zobrist_key ^= zobrist_white_moved;
            }

			int score = -minimax(shift == WHITE? BLACK : WHITE, depth + 1 + R, -beta, -beta + 1, temp_line);

			moves_generator->enpassant = temp_enpassant;
            zobrist_key = temp_zobrist;

			if (score >= beta) {
				zobrist_tt[zobrist_key % zobrist_tt_size].flags = 2;
				TT_move_best_from = -1;
				TT_move_best_to = -1;
				return beta;
			}
		}
	}

    unsigned long long temp_zobrist = zobrist_key;
    if (moves_generator->enpassant != 127) {
        zobrist_key ^= zobrist_enpassant[moves_generator->enpassant];
    }

    int from[120];
    int to[120];
    int num_moves = 0;
    int capture_num_moves = 0;

    alpha = calculate(shift, from, to, alpha, beta, num_moves, capture_num_moves, depth, im_in_check, main_line);


//    if (depth == 1) {
//        std::cout << "search done" << std::endl;
//        std::cout << "val " << alpha << std::endl;
//        std::cout << "MAX DEPTH: " << MAX_DEPTH << std::endl;
//        std::cout << "num leaf : " << num_nodes << std::endl;
//        std::cout << "nodes visited : " << zobrist_key_saved << std::endl;
//        std::cout << "alpha-beta cuts : " << alpha_beta_cuts << std::endl;
//        std::cout << "zobrist duplicated : " << zobrist_key_duplicated << std::endl;
//        std::cout << "zobrist recycled keys : " << recycled_keys << std::endl;
//        //std::cout << "zobrist bad keys : " << bad_keys << std::endl;
//        std::cout << "TT_move recovered : " << TT_move_recovered << std::endl;
//        std::cout << "TT_move alpha-beta cuts : " << TT_move_cuts << std::endl;
//        std::cout << "extra evalutaion : " << extra_evaluation << std::endl;
//        std::cout << "repetition_TT_colitions : " << repetition_tt_colitions << std::endl;
//        std::cout << "repetition encountered: " << repetitions_count << std::endl;
//        std::cout << "from: " << best_from << std::endl;
//        std::cout << "to: " << best_to << std::endl;
//
//    }

    zobrist_key = temp_zobrist;
    MAX_DEPTH = temp_MAX_DEPTH;
    return alpha;

}


int Search::setup_move(int alpha, int beta, int shift, int origin, int dest, int depth, std::vector<std::pair<int, int>>& main_line) {
    num_nodes++;

    // every 2048 nodes we check if time is over
	if (num_nodes & 2048) {
		if ( start_time + time_limit < std::clock() ) {
			time_over = true;
		}
	}

	if (time_over) {
        return INFINITE;
	}

    branch[depth] += 1;

    int depth_left = (MAX_DEPTH - depth);
    unsigned long long prev_zobrist_key = zobrist_key;

    incremental_zobrist(board, origin, dest, shift);
    int zobrist_indice = zobrist_key % zobrist_tt_size;

    // detect repetitions
    if (repetition_found()) {
        zobrist_key = prev_zobrist_key;
        return 0;
    }

    // Check if current position is availabe in transposition table
	if (zobrist_tt[zobrist_indice].zobrist_key == zobrist_key) {
		zobrist_key_duplicated++;
		if (zobrist_tt[zobrist_indice].depth >= depth_left) {
			int z_score = zobrist_tt[zobrist_indice].score;
			int flag = zobrist_tt[zobrist_indice].flags;

            if (flag == 3) {
				recycled_keys++;
				main_line.clear();
				for (int i = 0; i < zobrist_tt[zobrist_indice].line.size(); i++) {
					main_line.push_back(zobrist_tt[zobrist_indice].line[i]);
				}

				zobrist_key = prev_zobrist_key;
				return z_score;
			}

			else if (flag == 1) {
				if (z_score <= alpha) {
					recycled_keys++;
					zobrist_key = prev_zobrist_key;
					return alpha;
				}
			}

			else if (flag == 2) {
				if (z_score >= beta) {
					recycled_keys++;
					zobrist_key = prev_zobrist_key;
					return beta;
				}
			}

		}
	}

	// add position to repetition table
	repetition_table[repetition_index++] = zobrist_key;

	// handle repetition
    flag_enpassant = 0;
    flag_castle = 0;
    flag_promotion = 0;

    // find real to lol
    int real_to = dest;
	if (dest > 1000 and dest < 2010) {
        real_to = origin + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + dest];
        flag_promotion = 1;
	}
	else if (dest > 4000 and dest < 4010) {
        // when castling, this leads to not captures, no material changes
        real_to = 127;
        flag_castle = 1;
	}
	else if (dest > 700 and dest < 710) {
        // perft test
        flag_enpassant = 1;
        //num_enpassant++;
        // -----
		if (dest == 703) real_to = origin - 17;
		else if ( dest == 704) real_to = origin - 15;

		else if (dest == 701) real_to = origin + 15;
		else if (dest == 702) real_to = origin + 17;

	}

	// find out if pawn is moving 2 squares further
	int enpassant = 127;
	if (shift == 1) {
        if (board[origin] == WPAWN) {
            if ((origin - dest) == 32 ) {
                enpassant = origin - 16;
            }
        }
	} else {
        if (board[origin] == BPAWN) {
            if ((dest - origin) == 32) {
                enpassant = origin + 16;
            }
        }
	}
	moves_generator->enpassant = enpassant;

    // save currrent castle rights state
    int cur_castle_rights[4];
    cur_castle_rights[0] = moves_generator->castle_rights[0];
    cur_castle_rights[1] = moves_generator->castle_rights[1];
    cur_castle_rights[2] = moves_generator->castle_rights[2];
    cur_castle_rights[3] = moves_generator->castle_rights[3];
    // set castle rights for next position
    evaluate_castle_rights(board, shift, origin);

    // pesto incremental eval
    int temp_mg_white = mg_eval[1];
	int temp_mg_black = mg_eval[0];
	int temp_eg_white = eg_eval[1];
	int temp_eg_black = eg_eval[0];
	int temp_game_phase = game_phase;

	flag_capture = 0;

	if (board[real_to] != 0 or (dest > 700 and dest < 710) ) flag_capture = 1;

    int* ptr_deleted = NULL;
    int val_deleted;
    int flag_promotion;
    int* ptr_deleted_pawn = NULL;
    make_move(board, origin, dest, ptr_deleted, val_deleted, flag_promotion, ptr_deleted_pawn, shift);

//    // zobrist key generation test
//    unsigned long long z = generate_zobrist_key(shift, 10);
//
//    if (enpassant != 127) {
//        z ^= zobrist_enpassant[enpassant];
//    }
//
//	if (dest == 4001) {
//		z ^= zobrist_castle[0];
//	} else if (dest == 4002) {
//		z ^= zobrist_castle[1];
//	} else if (dest == 4003) {
//		z ^= zobrist_castle[2];
//	} else if (dest == 4004) {
//		z ^= zobrist_castle[3];
//	}
//
//    if (shift == BLACK) {
//        z ^= zobrist_black_moved;
//	} else {
//        z ^= zobrist_white_moved;
//    }
//
//
//    if (z != zobrist_key) {
//        wrong_pv_move_count++;
//        //std::cout << "wrong" << std::endl;
//    }


    zobrist_tt[zobrist_indice].line.clear();
	int val;
	if (shift == WHITE) {
        val = minimax(BLACK, depth + 1, alpha, beta, main_line);
	} else {
        val = minimax(WHITE, depth + 1, alpha, beta, main_line);
	}

    unmake_move(board, origin, dest, ptr_deleted, val_deleted, flag_promotion, ptr_deleted_pawn, shift);

    // remove position from repetitio table
    repetition_index--;

	// restore castle rights state
    moves_generator->castle_rights[0] = cur_castle_rights[0];
    moves_generator->castle_rights[1] = cur_castle_rights[1];
    moves_generator->castle_rights[2] = cur_castle_rights[2];
    moves_generator->castle_rights[3] = cur_castle_rights[3];

    // pesto incremental eval
    mg_eval[1] = temp_mg_white;
    mg_eval[0] = temp_mg_black;
    eg_eval[1] = temp_eg_white;
    eg_eval[0] = temp_eg_black;
    game_phase = temp_game_phase;

    // save position to main transposition table
	zobrist_tt[zobrist_indice].zobrist_key = zobrist_key;
    zobrist_tt[zobrist_indice].depth = depth_left;
    zobrist_tt[zobrist_indice].best_from = TT_move_best_from;
    zobrist_tt[zobrist_indice].best_to = TT_move_best_to;
    zobrist_tt[zobrist_indice].score = val;


    // save best move in 2nd bucket of transposition table
    if ((zobrist_tt_2nd[zobrist_indice].depth - 1) <= depth_left){
        zobrist_tt_2nd[zobrist_indice].zobrist_key = zobrist_key;
        zobrist_tt_2nd[zobrist_indice].depth = depth_left;
        //zobrist_tt_2nd[zobrist_indice].score = val;
        zobrist_tt_2nd[zobrist_indice].best_from = TT_move_best_from;
        zobrist_tt_2nd[zobrist_indice].best_to = TT_move_best_to;
    }

    // save best move in 3nd bucket of transposition table
    if (TT_move_best_from != -1) {
        if (history[shift  == WHITE ? -1 : 1 ][TT_move_best_from][TT_move_best_to] >
            history[shift  == WHITE ? -1 : 1 ][zobrist_tt_3nd[zobrist_indice].best_from][zobrist_tt_3nd[zobrist_indice].best_to] ) {
                zobrist_tt_3nd[zobrist_indice].zobrist_key = zobrist_key;
                zobrist_tt_3nd[zobrist_indice].best_from = TT_move_best_from;
                zobrist_tt_3nd[zobrist_indice].best_to = TT_move_best_to;
		}
    }

	zobrist_key = prev_zobrist_key;

	hashf = -1;

	// every 2048 nodes we check if time is over
	if (num_nodes & 2048) {
		if ( start_time + time_limit < std::clock() ) {
			time_over = true;
		}
	}

	if (time_over) {
        return INFINITE;
	}

	return val;
}


int Search::calculate(int shift, int* from, int* to, int alpha, int  beta, int& num_moves, int& capture_num_moves, int depth, bool im_in_check, std::vector<std::pair<int, int>>& main_line) {
	num_positions_generated++;
	int cur_king_location;

	int capture_from[80];
	int capture_to[80];

	if (shift == WHITE) {
		cur_king_location = search_wpieces.king;
	} else {
		cur_king_location = search_bpieces.king;
	}

	int prev_num_moves = 0;
	int remaining_detph = (MAX_DEPTH - depth) - 1;

	int zobrist_indice = zobrist_key % zobrist_tt_size;

	bool check = false;

    num_moves = 0;
    capture_num_moves = 0;
    moves_generator->depth = depth;
    moves_generator->killer1_found = false;
    moves_generator->killer2_found = false;
    bool killer1_found = false;
    bool killer2_found = false;
    int killer1_from;
    int killer1_to;
    int killer2_from;
    int killer2_to;

	int info_moves = generate_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, shift);
	if (info_moves == 555555555) {
        TT_move_best_from = -1;
        TT_move_best_to = -1;
		zobrist_tt[zobrist_indice].flags = -1;
		return 555555555; //king was captured, ilegal.
	}

    if (moves_generator->killer1_found) {
        killer1_found = true;
        killer1_from = moves_generator->killer_moves[depth][0];
        killer1_to = moves_generator->killer_moves[depth][1];
    }
    if (moves_generator->killer2_found) {
        killer2_found = true;
        killer2_from = moves_generator->killer_moves[depth][2];
        killer2_to = moves_generator->killer_moves[depth][3];
        if (killer1_from == killer2_from and killer1_to == killer2_to) killer2_found = false;
    }



    // use for perft test

//     ordering_captures(capture_from, capture_to, capture_num_moves, shift);
//     ordering_moves(from, to, num_moves, shift);
//
//	std::vector<std::pair<int, int>> cur_line_test;
//	bool all_ilegal = true;
//    for (int i = 0; i < capture_num_moves; i++) {
//        long long val = -setup_move(-beta, -alpha, shift, capture_from[i], capture_to[i], depth, cur_line_test);
//        if (val != -555555555) all_ilegal = false;
//    }
//
//    for (int i = 0; i < num_moves; i++) {
//        long long val = -setup_move(-beta, -alpha, shift, from[i], to[i], depth, cur_line_test);
//        if (val != -555555555) all_ilegal = false;
//    }
//
//    if (all_ilegal) {
//        check_mate++;
//    }
//
//    return 0;

    // ---------------------



    // check if tt move is available
	bool has_tt_move = false;
	int tt_from = -1;
	int tt_to = -1;
	if (zobrist_tt_2nd[zobrist_indice].zobrist_key == zobrist_key) {
		tt_from = zobrist_tt_2nd[zobrist_indice].best_from;
		tt_to = zobrist_tt_2nd[zobrist_indice].best_to;
		if (tt_from != -1 and tt_to != -1) {
        has_tt_move = true;
        if (tt_from == killer1_from and tt_to == killer1_to) killer1_found = false;
        if (tt_from == killer2_from and tt_to == killer2_to) killer2_found = false;
//            TT_move_recovered++;
//            bool prove_tt_mode = false;
//            for (int i = 0; i < capture_num_moves; i++) {
//            if (capture_from[i] == tt_from and capture_to[i] == tt_to)
//                prove_tt_mode = true;
//            }
//            for (int i = 0; i < num_moves; i++) {
//            if (from[i] == tt_from and to[i] == tt_to)
//                prove_tt_mode = true;
//            }
//            if (prove_tt_mode == false) wrong_tt_move_count++;
//            has_tt_move = prove_tt_mode;
		}

	}

	else if (zobrist_tt[zobrist_indice].zobrist_key == zobrist_key) {
		tt_from = zobrist_tt[zobrist_indice].best_from;
		tt_to = zobrist_tt[zobrist_indice].best_to;
		if (tt_from != -1 and tt_to != -1) {
        has_tt_move = true;
        if (tt_from == killer1_from and tt_to == killer1_to) killer1_found = false;
        if (tt_from == killer2_from and tt_to == killer2_to) killer2_found = false;

		}
	}

	else if (zobrist_tt_3nd[zobrist_indice].zobrist_key == zobrist_key) {
		tt_from = zobrist_tt_3nd[zobrist_indice].best_from;
		tt_to = zobrist_tt_3nd[zobrist_indice].best_to;
		if (tt_from != -1 and tt_to != -1) {
        has_tt_move = true;
        if (tt_from == killer1_from and tt_to == killer1_to) killer1_found = false;
        if (tt_from == killer2_from and tt_to == killer2_to) killer2_found = false;

		}
	}

	long long temp_hashf = 1;
	int cur_reduction_flag = 0;

	bool low_alpha = abs(alpha) < 70000? true:false;

	//set tt_move for this node
	int temp_TT_move_best_from = -1;
	int temp_TT_move_best_to = -1;

	int num_moves_before_cut = 0;

	int best = alpha;
	std::vector<std::pair<int, int>> cur_line;

	// check if we are in pv line
	// Nisa was generating pv lines wrongly so we prove if current pv lines is legal
	// this problem has been fixed but proving pv is cheap so we do it anyway
	int pv_from = -1;
	int pv_to = -1;
	bool has_pv_move = false;
	if (!pv_queue.empty()) {
        std::pair<int, int> cur_pv_move = pv_queue.front(); pv_queue.pop();
        pv_from = cur_pv_move.first;
        pv_to = cur_pv_move.second;

        bool prove_pv_mode = false;
        for (int i = 0; i < capture_num_moves; i++) {
            if (capture_from[i] == pv_from and capture_to[i] == pv_to)
                prove_pv_mode = true;
        }
        for (int i = 0; i < num_moves; i++) {
            if (from[i] == pv_from and to[i] == pv_to)
                prove_pv_mode = true;
        }
        has_pv_move = prove_pv_mode;
	}

	// history data
	int history_best_from = -1;
	int history_best_to   = -1;

	int cur_best_from = -1;
	int cur_best_to = -1;

	//lets assume all moves are ilegal until we find a not 555555 value
	bool all_ilegal_moves = true;
	int moves_played = 0;

	// do all moves in order
	// 1.- pv move
	// 2.- tt move
	// 3.- captures
	// 4.- killer moves
	// 5.- quiet moves
	if (!check) {
        bool alpha_beta_finished = false;
        int pre_best;

		if (has_pv_move) {
			num_moves_before_cut++;
			int val = -setup_move(-beta, -alpha, shift, pv_from, pv_to, depth, cur_line);

			if (val != -555555555) all_ilegal_moves = false;
			else {
				zobrist_tt[zobrist_indice].line.clear();
                goto n;
			}
			moves_played++;

			if (val > best) {
				best = val;
				if (depth == 1 and time_over == false) {
					best_from = pv_from;
					best_to = pv_to;
				}
				temp_TT_move_best_from = pv_from;
				temp_TT_move_best_to = pv_to;
			}

			if (val >= beta) {
				TT_move_best_from = temp_TT_move_best_from;
				TT_move_best_to = temp_TT_move_best_to;
				zobrist_tt[zobrist_indice].flags = 2;
				return beta;
			}
			if (val > alpha) {

				main_line.clear();
				zobrist_tt[zobrist_indice].line.clear();
				std::pair<int, int> cur_move = std::make_pair(pv_from, pv_to);
				main_line.push_back(cur_move);
				zobrist_tt[zobrist_indice].line.push_back(cur_move);
				for (int q = 0; q < cur_line.size(); q++) {
					main_line.push_back(cur_line[q]);
					zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
				}

				temp_hashf = 3;
				alpha = val;
			}
		}
n:

        if (has_tt_move) {
			num_moves_before_cut++;

			int val = -setup_move(-beta, -alpha, shift, tt_from, tt_to, depth, cur_line);


			if (val != -555555555) all_ilegal_moves = false;

			else {
				zobrist_tt[zobrist_indice].line.clear();
                goto ntt_move;
			}
			moves_played++;

			if (val > alpha and tt_to < 127) {
				if (cur_best_from != -1) {
					history[shift][cur_best_from][cur_best_to] -= (1 << remaining_detph);
				}
				history[shift][tt_from][tt_to] += (1 << remaining_detph);

				cur_best_from = tt_from;
				cur_best_to   = tt_to;;
			}

			if (val > best) {
				best = val;
				if (depth == 1) {
					best_from = tt_from;
					best_to = tt_to;
				}
				temp_TT_move_best_from = tt_from;
				temp_TT_move_best_to = tt_to;

			}


			if (val >= beta) {
				TT_move_best_from = temp_TT_move_best_from;
				TT_move_best_to = temp_TT_move_best_to;
				zobrist_tt[zobrist_indice].flags = 2;
				return beta;
			}
			if (val > alpha) {

				main_line.clear();
				zobrist_tt[zobrist_indice].line.clear();
				std::pair<int, int> cur_move = std::make_pair(tt_from, tt_to);
				main_line.push_back(cur_move);
				zobrist_tt[zobrist_indice].line.push_back(cur_move);
				for (int q = 0; q < cur_line.size(); q++) {
					main_line.push_back(cur_line[q]);
					zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
				}

				temp_hashf = 3;
				alpha = val;
			}
		}
ntt_move:


		if (alpha_beta_finished == false) {

            ordering_captures(capture_from, capture_to, capture_num_moves, shift);

			for (int i = 0; i < capture_num_moves; i++) {
                if (has_pv_move and capture_from[i] == pv_from and capture_to[i] == pv_to ) {
                    continue;
                }
                if (has_tt_move and capture_from[i] == tt_from and capture_to[i] == tt_to ) {
                    continue;
                }
                num_moves_before_cut++;
                int val = -setup_move(-beta, -alpha, shift, capture_from[i], capture_to[i], depth, cur_line);


				if (val != -555555555) all_ilegal_moves = false;
				else {
                    zobrist_tt[zobrist_indice].line.clear();
                    continue;
				}

				moves_played++;

				if (val > best) {
                    best = val;
                    if (depth == 1 and time_over == false) {
                        best_from = capture_from[i];
                        best_to = capture_to[i];
                    }
                    temp_TT_move_best_from = capture_from[i];
                    temp_TT_move_best_to = capture_to[i];

				}

				if (val >= beta) {
					TT_move_best_from = temp_TT_move_best_from;
					TT_move_best_to = temp_TT_move_best_to;
					zobrist_tt[zobrist_indice].flags = 2;
					return beta;
				}
				if (val > alpha) {

				    main_line.clear();
				    zobrist_tt[zobrist_indice].line.clear();
                    std::pair<int, int> cur_move = std::make_pair(capture_from[i] , capture_to[i]);
                    main_line.push_back(cur_move);
                    zobrist_tt[zobrist_indice].line.push_back(cur_move);
                    for (int q = 0; q < cur_line.size(); q++) {
                        main_line.push_back(cur_line[q]);
                        zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
                    }


                    temp_hashf = 3;
                    alpha = val;
				}
			}
		}

		//check killer
		if (killer1_found) {
			num_moves_before_cut++;
            int val = -setup_move(-beta, -alpha, shift, killer1_from, killer1_to, depth, cur_line);

			if (val != -555555555) all_ilegal_moves = false;

			else {
				zobrist_tt[zobrist_indice].line.clear();
                goto n3;
			}
			moves_played++;

			if (val > alpha) {
				if (cur_best_from != -1) {
					history[shift][cur_best_from][cur_best_to] -= (1 << remaining_detph);
				}
				history[shift][killer1_from][killer1_to] += (1 << remaining_detph);

				cur_best_from = killer1_from;
				cur_best_to   = killer1_to;
			}

			if (val > best) {
				best = val;
				if (depth == 1 and time_over == false) {
					best_from = killer1_from;
					best_to = killer1_to;
				}
				temp_TT_move_best_from = killer1_from;
				temp_TT_move_best_to = killer1_to;

			}

			if (val >= beta) {
				TT_move_best_from = temp_TT_move_best_from;
				TT_move_best_to = temp_TT_move_best_to;
				moves_generator->save_killer(killer1_from, killer1_to, depth);
				zobrist_tt[zobrist_indice].flags = 2;
				return beta;
			}
			if (val > alpha) {
				main_line.clear();
				zobrist_tt[zobrist_indice].line.clear();
				std::pair<int, int> cur_move = std::make_pair(killer1_from, killer1_to);
				main_line.push_back(cur_move);
				zobrist_tt[zobrist_indice].line.push_back(cur_move);
				for (int q = 0; q < cur_line.size(); q++) {
					main_line.push_back(cur_line[q]);
					zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
				}

				temp_hashf = 3;
				alpha = val;
			}
		}
n3:

        if (killer2_found) {
			num_moves_before_cut++;
            int val = -setup_move(-beta, -alpha, shift, killer2_from, killer2_to, depth, cur_line);

			if (val != -555555555) all_ilegal_moves = false;
			else {
				zobrist_tt[zobrist_indice].line.clear();
                goto n4;
			}
			moves_played++;

			if (val > alpha) {
				if (cur_best_from != -1) {
					history[shift][cur_best_from][cur_best_to] -= (1 << remaining_detph);
				}
				history[shift][killer2_from][killer2_to] += (1 << remaining_detph);

				cur_best_from = killer2_from;
				cur_best_to   = killer2_to;
			}

			if (val > best) {
				best = val;
				if (depth == 1 and time_over == false) {
					best_from = killer2_from;
					best_to = killer2_to;
				}
				temp_TT_move_best_from = killer2_from;
				temp_TT_move_best_to = killer2_to;
			}



			if (val >= beta) {
				TT_move_best_from = temp_TT_move_best_from;
				TT_move_best_to = temp_TT_move_best_to;
				moves_generator->save_killer(killer2_from, killer2_to, depth);
				zobrist_tt[zobrist_indice].flags = 2;
				return beta;
			}
			if (val > alpha) {
				main_line.clear();
				zobrist_tt[zobrist_indice].line.clear();
				std::pair<int, int> cur_move = std::make_pair(killer2_from, killer2_to);
				main_line.push_back(cur_move);
				zobrist_tt[zobrist_indice].line.push_back(cur_move);
				for (int q = 0; q < cur_line.size(); q++) {
					main_line.push_back(cur_line[q]);
					zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
				}

				temp_hashf = 3;
				alpha = val;
			}
		}
n4:

		if (!alpha_beta_finished) {

            ordering_moves(from, to, num_moves, shift);

			for (int i = 0; i < num_moves; i++) {
                if (has_pv_move and from[i] == pv_from and to[i] == pv_to) {
                    continue;
                }
                if (has_tt_move and from[i] == tt_from and to[i] == tt_to ) {
                    continue;
                }
                if (killer1_found and killer1_from == from[i] and killer1_to == to[i]) {
                    continue;
                }
                if (killer2_found and killer2_from == from[i] and killer2_to == to[i]) {
                    continue;
                }

                int val;

                bool i_do_check = move_gives_check(board, from[i], to[i], shift);

                int remaining_detph = (MAX_DEPTH - depth) - 1;

                //--- FUTILITY PRUNING
                if (!im_in_check and
                    !i_do_check and
                    remaining_detph <= 4 and
                    (evaluate(shift) + (70 * remaining_detph)) <= alpha and
                    moves_played > 1
                    ) {
                        continue;
                    }
                // ------------------

                num_moves_before_cut++;

				// LMR ----
				if (!im_in_check and remaining_detph >= 2 and moves_played > 1) {
					int R = 0;

					if (!i_do_check and (moves_played > 3) ) R += 2;

					// avoid drop to quiescence search if has transposition
					if ( (depth + R) >= MAX_DEPTH) depth = MAX_DEPTH - 1;

					val = -setup_move( -alpha -1, -alpha, shift, from[i], to[i], depth + R, cur_line);

					if (val  > alpha) {
						val = -setup_move(-beta, -alpha, shift, from[i], to[i], depth, cur_line);
					}
				} else {
					val = -setup_move(-beta, -alpha, shift, from[i], to[i], depth, cur_line);
				}


				if (val != -555555555) all_ilegal_moves = false;
				else {
                    zobrist_tt[zobrist_indice].line.clear();
                    continue;
				}

				moves_played++;

				if (val > alpha) {
                    if (cur_best_from != -1) {
                        history[shift][cur_best_from][cur_best_to] -= (1 << remaining_detph);
                    }
					history[shift][from[i]][to[i]] += (1 << remaining_detph);

					cur_best_from = from[i];
                    cur_best_to   = to[i];
				}

				if (val > best) {
                    best = val;
                    if (depth == 1 and time_over == false) {
                        best_from = from[i];
                        best_to = to[i];
                    }
                    temp_TT_move_best_from = from[i];
                    temp_TT_move_best_to = to[i];
				}

				if (val >= beta) {
					TT_move_best_from = temp_TT_move_best_from;
					TT_move_best_to = temp_TT_move_best_to;
					moves_generator->save_killer(from[i], to[i], depth);
					zobrist_tt[zobrist_indice].flags = 2;
					return beta;
				}
				if (val > alpha) {

                    main_line.clear();
				    zobrist_tt[zobrist_indice].line.clear();
                    std::pair<int, int> cur_move = std::make_pair(from[i] , to[i]);
                    main_line.push_back(cur_move);
                    zobrist_tt[zobrist_indice].line.push_back(cur_move);
                    for (int q = 0; q < cur_line.size(); q++) {
                        main_line.push_back(cur_line[q]);
                        zobrist_tt[zobrist_indice].line.push_back(cur_line[q]);
                    }

                    temp_hashf = 3;
                    alpha = val;
				}
			}
		}

	}

	//set tt_move
	TT_move_best_from = temp_TT_move_best_from;
	TT_move_best_to = temp_TT_move_best_to;

	// are all moves ilegal?
	if (all_ilegal_moves) {
        // are we in check?
        if (moves_generator->is_in_check(board, cur_king_location, shift)) {
            zobrist_tt[zobrist_indice].flags = 3;
            // return check mate score
            return -7777777 + depth; //try to do fastest check mate
        }
        zobrist_tt[zobrist_indice].flags = 3;
        // return stalemate score
        return 0;
	}

    zobrist_tt[zobrist_indice].flags = temp_hashf;

	return alpha;

}

// do captures ordering with MVV-LVA using the pesto tables to obtain the value of the pieces
void Search::ordering_captures(int* from, int* to, int num_moves, int shift) {

	int temp_from[44];
	int temp_to[44];
	for (int i = 0; i < num_moves; i++) {
		temp_from[i] = from[i];
		temp_to[i] = to[i];
	}

	int mg_phase = game_phase;
	if (game_phase > 24) mg_phase = 24;
	int eg_phase = 24 - mg_phase;

	std::vector<std::pair<int, int>> moves(num_moves);
	for (int i = 0; i < num_moves; i++) {
        int val = 0;

        if (to[i] <= 127) {
            int pc_from = board[from[i]];
            int pc_to = board[to[i]];
            int score_dest = (mg_pst_inc[pc_to & 232][to[i]] * mg_phase + eg_pst_inc[pc_to & 232][to[i]] * eg_phase) / 24;
            int score_origin = (mg_pst_inc[pc_from & 232][from[i]] * mg_phase + eg_pst_inc[pc_from & 232][from[i]] * eg_phase) / 24;
            val = (score_dest << 3 ) - score_origin;
        }
        else if (to[i] > 1000 and to[i] < 2010) {
            val = 20000000;
        } else  { // enpassant
            val = 20000000;
        }

		std::pair<int, int> move(val, i);
		moves[i] = move;
	}
	std::sort(std::rbegin(moves), std::rend(moves));

	for (int i = 0; i < num_moves; i++) {
		from[i] = temp_from[moves[i].second];
		to[i] = temp_to[moves[i].second];
	}
}

// order quiet moves with historic heuristic
void Search::ordering_moves(int* from, int* to, int num_moves, int shift) {
	std::vector<std::pair<int, int>> order(num_moves);

	int mg_phase = game_phase;
	if (game_phase > 24) mg_phase = 24;
	int eg_phase = 24 - mg_phase;

	for (int i = 0; i < num_moves; i++) {
		int origin = from[i];
		int dest = to[i];
		int val = 45454545;

		if (dest > 4000 and dest < 4010) {
            val = 100000;
		} else {

//            Piece-square evaluation
//            if (shift == WHITE) {
//				if (board[origin] == 100) {
//                    val = mo_pawn_white[dest] - mo_pawn_white[origin];
//				} else if (board[origin] == 300) {
//				    val = mo_bishop_white[dest] - mo_bishop_white[origin];
//				} else if (board[origin] == 310) {
//				    val = mo_knight_white[dest] - mo_knight_white[origin];
//				} else if (board[origin] == 500) {
//				    val = mo_rook_white[dest] - mo_rook_white[origin];
//				} else if (board[origin] == 900) {
//				    val = mo_queen_white[dest] - mo_queen_white[origin];
//				} else if (board[origin] == 8888) {
//				    val = mo_king_white[dest] - mo_king_white[origin];
//				}
//			} else {
//				if (board[origin] == -100) {
//                    val = mo_pawn_black[dest] - mo_pawn_black[origin];
//				} else if (board[origin] == -300) {
//				    val = mo_bishop_black[dest] - mo_bishop_black[origin];
//				} else if (board[origin] == -310) {
//				    val = mo_knight_black[dest] - mo_knight_black[origin];
//				} else if (board[origin] == -500) {
//				    val = mo_rook_black[dest] - mo_rook_black[origin];
//				} else if (board[origin] == -900) {
//				    val = mo_queen_black[dest] - mo_queen_black[origin];
//				} else if (board[origin] == -8888) {
//				    val = mo_king_black[dest] - mo_king_black[origin];
//				}
//			}

            val = history[shift][origin][dest];

		}

		std::pair<int, int> p(val, i);
		order[i] = p;
	}

    sort(order.rbegin(), order.rend());
	int temp_from[num_moves];
	int temp_to[num_moves];

	for (int i = 0; i < num_moves; i++) {
		temp_from[i] = from[i];
		temp_to[i] = to[i];
	}

	for (int i = 0; i < num_moves; i++) {
		from[i] = temp_from[order[i].second];
		to[i] = temp_to[order[i].second];
	}
}

void Search::populate_move_ordering_tables() {
    int game_phase = this->game_phase;
    int mg_phase = game_phase;
    if (game_phase > 24) mg_phase = 24;
    int eg_phase = 24 - mg_phase;

    for (int i = 0; i < 128; i++) {
        mo_king_white[i] = (mg_king_white[i]*mg_phase + eg_king_white[i]*eg_phase) / 24;
        mo_queen_white[i] = (mg_queen_white[i]*mg_phase + eg_queen_white[i]*eg_phase) / 24;
        mo_rook_white[i] = (mg_rook_white[i]*mg_phase + eg_rook_white[i]*eg_phase) / 24;
        mo_knight_white[i] = (mg_knight_white[i]*mg_phase + eg_knight_white[i]*eg_phase) / 24;
        mo_bishop_white[i] = (mg_bishop_white[i]*mg_phase + eg_bishop_white[i]*eg_phase) / 24;
        mo_pawn_white[i] = (mg_pawn_white[i]*mg_phase + eg_pawn_white[i]*eg_phase) / 24;

        mo_king_black[i] = (mg_king_black[i]*mg_phase + eg_king_black[i]*eg_phase) / 24;
        mo_queen_black[i] = (mg_queen_black[i]*mg_phase + eg_queen_black[i]*eg_phase) / 24;
        mo_rook_black[i] = (mg_rook_black[i]*mg_phase + eg_rook_black[i]*eg_phase) / 24;
        mo_knight_black[i] = (mg_knight_black[i]*mg_phase + eg_knight_black[i]*eg_phase) / 24;
        mo_bishop_black[i] = (mg_bishop_black[i]*mg_phase + eg_bishop_black[i]*eg_phase) / 24;
        mo_pawn_black[i] = (mg_pawn_black[i]*mg_phase + eg_pawn_black[i]*eg_phase) / 24;
    }
}

int Search::generate_moves(int* board, int* from, int* to, int& num_moves, int* capture_from, int* capture_to, int& capture_num_moves, int shift){
    int* king;
    int* queen;
    int* rook;
    int* bishop;
    int* knight;
    int* pawn;

    int val_king;
    int val_queen;
    int val_pawn;
    int val_knight;
    int val_bishop;
    int val_rook;

    int queen_count;
    int rook_count;
    int knight_count;
    int bishop_count;
    int pawn_count;

    if (shift == WHITE){
        king = &search_wpieces.king;
        queen = search_wpieces.queen;
        rook = search_wpieces.rook;
        bishop = search_wpieces.bishop;
        knight = search_wpieces.knight;
        pawn = search_wpieces.pawn;

        val_king = 8888;
        val_queen = 900;
        val_pawn = 100;
        val_rook = 500;
        val_bishop = 300;
        val_knight = 310;

        queen_count  = white_queen_count;
        rook_count   = white_rook_count;
        knight_count = white_knight_count;
        bishop_count = white_bishop_count;
        pawn_count = white_pawn_count;

    } else {
        king = &search_bpieces.king;
        queen = search_bpieces.queen;
        rook = search_bpieces.rook;
        bishop = search_bpieces.bishop;
        knight = search_bpieces.knight;
        pawn = search_bpieces.pawn;

        val_king = -8888;
        val_queen = -900;
        val_pawn = -100;
        val_rook = -500;
        val_bishop = -300;
        val_knight = -310;

        queen_count  = black_queen_count;
        rook_count   = black_rook_count;
        knight_count = black_knight_count;
        bishop_count = black_bishop_count;
        pawn_count = black_pawn_count;

    }


    for (int i = 0; i < pawn_count; i++) {
		int cur_piece = *(pawn);
		pawn++;
		if (cur_piece != DELETED and board[cur_piece] == val_pawn) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->pawn_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}


	for (int i = 0; i < bishop_count; i++) {
		int cur_piece = *(bishop);
		bishop++;
		if (cur_piece != DELETED and board[cur_piece] == val_bishop) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->straight_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, dir.diagonal, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	for (int i = 0; i < knight_count; i++) {
		int cur_piece = *(knight);
		knight++;
		if (cur_piece != DELETED and board[cur_piece] == val_knight) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->knight_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	for (int i = 0; i < rook_count; i++) {
		int cur_piece = *(rook);
		rook++;
		if (cur_piece != DELETED and board[cur_piece] == val_rook) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->straight_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, dir.vertical, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

    for (int i = 0; i < queen_count; i++) {
		int cur_piece = *(queen);
		queen++;
		if (cur_piece != DELETED and board[cur_piece] == val_queen) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->straight_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, dir.vertical, cur_piece, *king);
			moves_generator->straight_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, dir.diagonal, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
                int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	if (*king != DELETED and board[*king] == val_king) {
		int prev_capture_num_moves = capture_num_moves;
		moves_generator->king_moves(board, from, to, num_moves, capture_from, capture_to, capture_num_moves, *king);
		for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
			int dest = capture_to[k];
            if (dest > 700 and dest < 710) dest = 127;
            else if (dest > 4000 and dest < 4010) dest = 127;
            else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

            if (abs(board[dest]) == 8888) {
                TT_move_best_from = capture_from[k];
                TT_move_best_to = capture_to[k];
                return 555555555;
            }
		}
	}

    return 1;
}

int Search::generate_attacks(int* board, int* capture_from, int* capture_to, int& capture_num_moves, int shift){
    int* king;
    int* queen;
    int* rook;
    int* bishop;
    int* knight;
    int* pawn;

    int val_king;
    int val_queen;
    int val_pawn;
    int val_knight;
    int val_bishop;
    int val_rook;

    int queen_count;
    int rook_count;
    int knight_count;
    int bishop_count;
    int pawn_count;

    if (shift == WHITE){
        king = &search_wpieces.king;
        queen = search_wpieces.queen;
        rook = search_wpieces.rook;
        bishop = search_wpieces.bishop;
        knight = search_wpieces.knight;
        pawn = search_wpieces.pawn;

        val_king = 8888;
        val_queen = 900;
        val_pawn = 100;
        val_rook = 500;
        val_bishop = 300;
        val_knight = 310;

        queen_count  = white_queen_count;
        rook_count   = white_rook_count;
        knight_count = white_knight_count;
        bishop_count = white_bishop_count;
        pawn_count = white_pawn_count;

    } else {
        king = &search_bpieces.king;
        queen = search_bpieces.queen;
        rook = search_bpieces.rook;
        bishop = search_bpieces.bishop;
        knight = search_bpieces.knight;
        pawn = search_bpieces.pawn;

        val_king = -8888;
        val_queen = -900;
        val_pawn = -100;
        val_rook = -500;
        val_bishop = -300;
        val_knight = -310;

        queen_count  = black_queen_count;
        rook_count   = black_rook_count;
        knight_count = black_knight_count;
        bishop_count = black_bishop_count;
        pawn_count = black_pawn_count;

    }


    for (int i = 0; i < pawn_count; i++) {
		int cur_piece = *(pawn);
		pawn++;
		if (cur_piece != DELETED and board[cur_piece] == val_pawn) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->attacks_pawn_moves(board, capture_from, capture_to, capture_num_moves, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}


	for (int i = 0; i < bishop_count; i++) {
		int cur_piece = *(bishop);
		bishop++;
		if (cur_piece != DELETED and board[cur_piece] == val_bishop) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->attacks_straight_moves(board, capture_from, capture_to, capture_num_moves, dir.diagonal, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	for (int i = 0; i < knight_count; i++) {
		int cur_piece = *(knight);
		knight++;
		if (cur_piece != DELETED and board[cur_piece] == val_knight) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->attacks_knight_moves(board, capture_from, capture_to, capture_num_moves, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	for (int i = 0; i < rook_count; i++) {
		int cur_piece = *(rook);
		rook++;
		if (cur_piece != DELETED and board[cur_piece] == val_rook) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->attacks_straight_moves(board, capture_from, capture_to, capture_num_moves, dir.vertical, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
				int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

    for (int i = 0; i < queen_count; i++) {
		int cur_piece = *(queen);
		queen++;
		if (cur_piece != DELETED and board[cur_piece] == val_queen) {
			int prev_capture_num_moves = capture_num_moves;
			moves_generator->attacks_straight_moves(board, capture_from, capture_to, capture_num_moves, dir.vertical, cur_piece, *king);
			moves_generator->attacks_straight_moves(board, capture_from, capture_to, capture_num_moves, dir.diagonal, cur_piece, *king);
			for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
                int dest = capture_to[k];
                if (dest > 700 and dest < 710) dest = 127;
                else if (dest > 4000 and dest < 4010) dest = 127;
                else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

				if (abs(board[dest]) == 8888) {
					TT_move_best_from = capture_from[k];
					TT_move_best_to = capture_to[k];
					return 555555555;
				}
			}
		}
	}

	if (*king != DELETED and board[*king] == val_king) {
		int prev_capture_num_moves = capture_num_moves;
		moves_generator->attacks_king_moves(board, capture_from, capture_to, capture_num_moves, *king);
		for (int k = prev_capture_num_moves; k < capture_num_moves; k++) {
			int dest = capture_to[k];
            if (dest > 700 and dest < 710) dest = 127;
            else if (dest > 4000 and dest < 4010) dest = 127;
            else if (dest > 1000 and dest < 2010) dest = capture_from[k] + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + dest];

            if (abs(board[dest]) == 8888) {
                TT_move_best_from = capture_from[k];
                TT_move_best_to = capture_to[k];
                return 555555555;
            }
		}
	}

    return 1;
}

void Search::evaluate_castle_rights(int* board, int shift, int from) {
	if (shift == WHITE) {
		if (from == 116) {
			moves_generator->castle_rights[2] = 0;
			moves_generator->castle_rights[3] = 0;
		}
		if (from == 112) {
			moves_generator->castle_rights[2] = 0;
		}
		if (from == 119) {
			moves_generator->castle_rights[3] = 0;
		}
	} else {
		if (from == 4) {
			moves_generator->castle_rights[0] = 0;
			moves_generator->castle_rights[1] = 0;
		}
		if (from == 0) {
			moves_generator->castle_rights[0] = 0;
		}
		if (from == 7) {
			moves_generator->castle_rights[1] = 0;
		}
	}
}



inline void Search::make_move(int* board, int from, int to, int*& ptr_deleted, int& val_deleted, int& flag_promotion, int*& ptr_deleted_pawn, int shift) {

    mg_incremental_evaluation(board, from, to, shift);

	// handle promotion
	if (to > 1000 and to < 2010) {
		//flag_promotion = 1;

		int promotion_data = to;
		// define real "to"
		to = from + promotion_offset[( (shift == WHITE ? 1 : 0) << 4) + to];

		// delete pawn
		ptr_deleted_pawn = dboard[from];
		*ptr_deleted_pawn = DELETED;
		board[from] = 0;
		dboard[from] = 0;

		// delete piece in to if exists
		if (board[to] != 0) {
			ptr_deleted = dboard[to];
			*ptr_deleted = DELETED;
            val_deleted = board[to];
		}

		// add promoted piece
		promote_piece(board, from, promotion_data, shift);
	}
	// Castle handling
	else if (to > 4000 and to < 4010) {
		if (shift == WHITE) {
			if (to == 4003) {
				dboard[114] = dboard[116];
				dboard[116] = 0;
				board[114] = board[116];
				board[116] = 0;

				dboard[115] = dboard[112];
				dboard[112] = 0;
				board[115] = board[112];
				board[112] = 0;

				*dboard[114] = 114;
				*dboard[115] = 115;
			} else {
				dboard[118] = dboard[116];
				dboard[116] = 0;
				board[118] = board[116];
				board[116] = 0;

				dboard[117] = dboard[119];
				dboard[119] = 0;
				board[117] = board[119];
				board[119] = 0;

				*dboard[118] = 118;
				*dboard[117] = 117;
			}
		} else {
			if (to == 4001) {
				dboard[2] = dboard[4];
				dboard[4] = 0;
				board[2] = board[4];
				board[4] = 0;

				dboard[3] = dboard[0];
				dboard[0] = 0;
				board[3] = board[0];
				board[0] = 0;

				*dboard[2] = 2;
				*dboard[3] = 3;
			} else {
                dboard[6] = dboard[4];
				dboard[4] = 0;
				board[6] = board[4];
				board[4] = 0;

				dboard[5] = dboard[7];
				dboard[7] = 0;
				board[5] = board[7];
				board[7] = 0;

				*dboard[6] = 6;
				*dboard[5] = 5;
			}
		}
	}
	// handle enpasant
	else if (to > 700 and to < 710) {
        // from of pawn deleted
		int from_deleted;

        // "to" is the enpassant square
		if (to == 703) {
			to = from - 17;
			from_deleted = from - 1;
		} else if (to == 704) {
			to = from - 15;
			from_deleted = from + 1;
		}

		else if (to == 701) {
			to = from + 15;
			from_deleted = from - 1;
		} else if (to == 702) {
			to = from + 17;
			from_deleted = from + 1;
		}

		// delete pawn captured
		ptr_deleted = dboard[from_deleted];
		*ptr_deleted = DELETED;
		dboard[from_deleted] = 0;
		board[from_deleted] = 0;

		// move own pawn
		dboard[to] = dboard[from];
		dboard[from] = 0;
		board[to] = board[from];
		board[from] = 0;
		*dboard[to] = to;

		//
		if (shift == WHITE) {
            black_mat_score += 100;
		} else {
            white_mat_score -= 100;
		}
	} else {
	    // normal moves
		if (board[to] != 0) {
            val_deleted = board[to];
			ptr_deleted = dboard[to];
			*ptr_deleted = DELETED;
		}
		dboard[to] = dboard[from];
		dboard[from] = 0;
		board[to] = board[from];
		board[from] = 0;
		*dboard[to] = to;
	}
}

inline void Search::unmake_move(int* board, int from, int to, int*& ptr_deleted, int& val_deleted, int& flag_promotion, int*& ptr_deleted_pawn, int shift) {

	if (to > 1000 and to < 2010) {
		int promotion_data = to;
		to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + to];

		// remove promoted piece
		unpromote_piece(board, from, promotion_data, shift);

		// restore piece in to
		if (ptr_deleted != NULL) {
			*ptr_deleted = to;
			dboard[to] = ptr_deleted;
			board[to] = val_deleted;
		}

		// restore pawn
		*ptr_deleted_pawn = from;
		board[from] = shift == 1? 100 : -100;
		dboard[from] = ptr_deleted_pawn;
	}
	// castle handling
	else if (to > 4000 and to < 4010) {
		if (shift == WHITE) {
			if (to == 4003) {
				dboard[116] = dboard[114];
				dboard[114] = 0;
				board[116] = board[114];
				board[114] = 0;

				dboard[112] = dboard[115];
				dboard[115] = 0;
				board[112] = board[115];
				board[115] = 0;

				*dboard[116] = 116;
				*dboard[112] = 112;
			} else {
				dboard[116] = dboard[118];
				dboard[118] = 0;
				board[116] = board[118];
				board[118] = 0;

				dboard[119] = dboard[117];
				dboard[117] = 0;
				board[119] = board[117];
				board[117] = 0;

				*dboard[116] = 116;
				*dboard[119] = 119;
			}
		} else {
			if (to == 4001) {
                dboard[4] = dboard[2];
				dboard[2] = 0;
				board[4] = board[2];
				board[2] = 0;

				dboard[0] = dboard[3];
				dboard[3] = 0;
				board[0] = board[3];
				board[3] = 0;

				*dboard[4] = 4;
				*dboard[0] = 0;
			} else {
                dboard[4] = dboard[6];
				dboard[6] = 0;
				board[4] = board[6];
				board[6] = 0;

				dboard[7] = dboard[5];
				dboard[5] = 0;
				board[7] = board[5];
				board[5] = 0;

				*dboard[4] = 4;
				*dboard[7] = 7;
			}
		}
	}
	// handle enpasant
	else if (to > 700 and to < 710) {
		// paste code to set to and from_deleted here
		int from_deleted;
        // "to" is the enpassant square
		if (to == 703) {
			to = from - 17;
			from_deleted = from - 1;
		}
		else if (to == 704) {
			to = from - 15;
			from_deleted = from + 1;
		}

		else if (to == 701) {
			to = from + 15;
			from_deleted = from - 1;
		} else if (to == 702) {
			to = from + 17;
			from_deleted = from + 1;
		}

		// restore captured pawn
        *ptr_deleted = from_deleted;
        dboard[from_deleted] = ptr_deleted;
        board[from_deleted] = shift == 1? -100 : 100;

		// unmake_move own pawn
		dboard[from] = dboard[to];
		dboard[to] = 0;
		board[from] = board[to];
		board[to] = 0;
		*dboard[from] = from;
	} else {
	    // normal moves
		board[from] = board[to];
		board[to] = 0;
		dboard[from] = dboard[to];
		dboard[to] = 0;
		*dboard[from] = from;
		if (ptr_deleted != NULL) {
            board[to] = val_deleted;
			dboard[to] = ptr_deleted;
			*dboard[to] = to;
		}
	}
}



void Search::promote_piece(int* board, int from, int promotion_data, int shift) {
	int to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + promotion_data];
	if (shift == WHITE) {
		if (promotion_data == 2008 or promotion_data == 1004 or promotion_data == 2007) {
			board[to] = 900;
			search_wpieces.queen[white_queen_count++] = to;
			dboard[to] = &search_wpieces.queen[white_queen_count - 1];
			white_mat_score += 800;
		} else if (promotion_data == 2006 or promotion_data == 1003 or promotion_data == 2005) {
			board[to] = 500;
			search_wpieces.rook[white_rook_count++] = to;
			dboard[to] = &search_wpieces.rook[white_rook_count - 1];
			white_mat_score += 400;
		} else if (promotion_data == 2004 or promotion_data == 1002 or promotion_data == 2003) {
			board[to] = 310;
			search_wpieces.knight[white_knight_count++] = to;
			dboard[to] = &search_wpieces.knight[white_knight_count - 1];
			white_mat_score += 210;
		} else if (promotion_data == 2002 or promotion_data == 1001 or promotion_data == 2001) {
			board[to] = 300;
			search_wpieces.bishop[white_bishop_count++] = to;
			dboard[to] = &search_wpieces.bishop[white_bishop_count - 1];
			white_mat_score += 200;
		}
	} else {
		if (promotion_data == 2008 or promotion_data == 1004 or promotion_data == 2007) {
			board[to] = -900;
			search_bpieces.queen[black_queen_count++] = to;
			dboard[to] = &search_bpieces.queen[black_queen_count - 1];
			black_mat_score += -800;
		} else if (promotion_data == 2006 or promotion_data == 1003 or promotion_data == 2005) {
			board[to] = -500;
			search_bpieces.rook[black_rook_count++] = to;
			dboard[to] = &search_bpieces.rook[black_rook_count - 1];
			black_mat_score += -400;
		} else if (promotion_data == 2004 or promotion_data == 1002 or promotion_data == 2003) {
			board[to] = -310;
			search_bpieces.knight[black_knight_count++] = to;
			dboard[to] = &search_bpieces.knight[black_knight_count - 1];
			black_mat_score += -210;
		} else if (promotion_data == 2002 or promotion_data == 1001 or promotion_data == 2001) {
			board[to] = -300;
			search_bpieces.bishop[black_bishop_count++] = to;
			dboard[to] = &search_bpieces.bishop[black_bishop_count - 1];
			black_mat_score += -200;
		}
	}
}

void Search::unpromote_piece(int* board, int from, int promotion_data, int shift) {
        int to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4)  + promotion_data];
        board[to] = 0;
        dboard[to] = 0;

        if (shift == WHITE) {
		if (promotion_data == 2008 or promotion_data == 1004 or promotion_data == 2007) {
            white_queen_count--;
		} else if (promotion_data == 2006 or promotion_data == 1003 or promotion_data == 2005) {
		    white_rook_count--;
		} else if (promotion_data == 2004 or promotion_data == 1002 or promotion_data == 2003) {
		    white_knight_count--;
		} else if (promotion_data == 2002 or promotion_data == 1001 or promotion_data == 2001) {
		    white_bishop_count--;
		}
	} else {
		if (promotion_data == 2008 or promotion_data == 1004 or promotion_data == 2007) {
            black_queen_count--;
		} else if (promotion_data == 2006 or promotion_data == 1003 or promotion_data == 2005) {
            black_rook_count--;
		} else if (promotion_data == 2004 or promotion_data == 1002 or promotion_data == 2003) {
		    black_knight_count--;
		} else if (promotion_data == 2002 or promotion_data == 1001 or promotion_data == 2001) {
		    black_bishop_count--;
		}
	}
}


int Search::evaluate(int shift) {
	int white_value;
	int black_value;

	int mg_phase = game_phase;

	if (mg_phase > 24) mg_phase = 24;
	int eg_phase = 24 - mg_phase;

	white_value = (mg_eval[1] * mg_phase + eg_eval[1] * eg_phase) / 24;
	black_value = (mg_eval[0] * mg_phase + eg_eval[0] * eg_phase) / 24;

	if (shift == WHITE) white_value += 10;
	else black_value += 10;

    if (shift == WHITE) {
        return white_value - black_value;
    }

	return black_value - white_value;
}

//  & 232 method
//  white
//  100 96
//  300 40
//  310 32
//  500 224
//  900 128
//  black
// -100 136
// -300 192
// -310 200
// -500 8
// -900 104

inline void Search::mg_incremental_evaluation(int* board, int from, int to, int shift) {
    int side_2move = 0;
    int other_side = 1;

    if (shift == WHITE) {
        side_2move = 1;
        other_side = 0;
    }

    if (to <= 127) { // normal move
        // mg
        mg_eval[side_2move] -= mg_pst_inc[board[from] & 232][from];
        mg_eval[side_2move] += mg_pst_inc[board[from] & 232][to];

        mg_eval[other_side] -= mg_pst_inc[board[to] & 232][to];

        // eg
        eg_eval[side_2move] -= eg_pst_inc[board[from] & 232][from];
        eg_eval[side_2move] += eg_pst_inc[board[from] & 232][to];

        eg_eval[other_side] -= eg_pst_inc[board[to] & 232][to];

        if ( std::abs(board[to]) >= 300 and std::abs(board[to]) <= 900 )  {
            game_phase -= game_phase_inc[std::abs(board[to])];
        }
    }
    else if (to > 4000 and to < 4010) { // castle
        if (to == 4001) { // black, long castle
            // mg
            mg_eval[0] -= mg_pst_inc[-8888 & 232][4];
            mg_eval[0] -= mg_pst_inc[-500 & 232][0];

            mg_eval[0] += mg_pst_inc[-8888 & 232][2];
            mg_eval[0] += mg_pst_inc[-500 & 232][3];

            // eg
            eg_eval[0] -= eg_pst_inc[-8888 & 232][4];
            eg_eval[0] -= eg_pst_inc[-500 & 232][0];

            eg_eval[0] += eg_pst_inc[-8888 & 232][2];
            eg_eval[0] += eg_pst_inc[-500 & 232][3];
        }
        else if (to == 4002) { // black, short castle
            // mg
            mg_eval[0] -= mg_pst_inc[-8888 & 232][4];
            mg_eval[0] -= mg_pst_inc[-500 & 232][7];

            mg_eval[0] += mg_pst_inc[-8888 & 232][6];
            mg_eval[0] += mg_pst_inc[-500 & 232][5];

            // eg
            eg_eval[0] -= eg_pst_inc[-8888 & 232][4];
            eg_eval[0] -= eg_pst_inc[-500 & 232][7];

            eg_eval[0] += eg_pst_inc[-8888 & 232][6];
            eg_eval[0] += eg_pst_inc[-500 & 232][5];
        }
        else if (to == 4003) { // white, long castle
            // mg
            mg_eval[1] -= mg_pst_inc[8888 & 232][116];
            mg_eval[1] -= mg_pst_inc[500 & 232][112];

            mg_eval[1] += mg_pst_inc[8888 & 232][114];
            mg_eval[1] += mg_pst_inc[500 & 232][115];

            // eg
            eg_eval[1] -= eg_pst_inc[8888 & 232][116];
            eg_eval[1] -= eg_pst_inc[500 & 232][112];

            eg_eval[1] += eg_pst_inc[8888 & 232][114];
            eg_eval[1] += eg_pst_inc[500 & 232][115];
        }
        else if (to == 4004) { //white, short castle
            // mg
            mg_eval[1] -= mg_pst_inc[8888 & 232][116];
            mg_eval[1] -= mg_pst_inc[500 & 232][119];

            mg_eval[1] += mg_pst_inc[8888 & 232][118];
            mg_eval[1] += mg_pst_inc[500 & 232][117];

            // eg
            eg_eval[1] -= eg_pst_inc[8888 & 232][116];
            eg_eval[1] -= eg_pst_inc[500 & 232][119];

            eg_eval[1] += eg_pst_inc[8888 & 232][118];
            eg_eval[1] += eg_pst_inc[500 & 232][117];
        }
    }
    else if (to > 1000 and to < 2010) { // promotion
        int real_to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + to];

        // remove own pawn
        // mg
        mg_eval[side_2move] -= mg_pst_inc[board[from] & 232][from];
        // eg
        eg_eval[side_2move] -= eg_pst_inc[board[from] & 232][from];

        // remove pience in to
        // mg
        mg_eval[other_side] -= mg_pst_inc[board[real_to] & 232][real_to];
        // eg
        eg_eval[other_side] -= eg_pst_inc[board[real_to] & 232][real_to];

        // add new piece
        int queen, rook, knight, bishop;
        if (shift == WHITE) {
            queen = 900;
            rook = 500;
            knight = 310;
            bishop = 300;
        } else {
            queen = -900;
            rook = -500;
            knight = -310;
            bishop = -300;
        }
        if (to == 2008 or to == 1004 or to == 2007) {
            // mg
            mg_eval[side_2move] += mg_pst_inc[queen & 232][real_to];
            // eg
            eg_eval[side_2move] += eg_pst_inc[queen & 232][real_to];
            game_phase += 4;
        }
        else if (to == 2006 or to == 1003 or to == 2005) {
            // mg
            mg_eval[side_2move] += mg_pst_inc[rook & 232][real_to];
            // eg
            eg_eval[side_2move] += eg_pst_inc[rook & 232][real_to];
            game_phase += 2;
        }
        else if (to == 2004 or to == 1002 or to == 2003) {
            // mg
            mg_eval[side_2move] += mg_pst_inc[knight & 232][real_to];
            // eg
            eg_eval[side_2move] += eg_pst_inc[knight & 232][real_to];
            game_phase += 1;
        }
        else if (to == 2002 or to == 1001 or to == 2001) {
            // mg
            mg_eval[side_2move] += mg_pst_inc[bishop & 232][real_to];
            // eg
            eg_eval[side_2move] += eg_pst_inc[bishop & 232][real_to];
            game_phase += 1;
        }

        if ( std::abs(board[real_to]) >= 300 and std::abs(board[real_to]) <= 900 )  {
            game_phase -= game_phase_inc[std::abs(board[real_to])];
        }

    }
    else if (to > 700 and to < 710) { // enpassant
        if (to == 701) { // black, left
            // mg
            mg_eval[0] -= mg_pst_inc[-100 & 232][from];
            mg_eval[0] += mg_pst_inc[-100 & 232][from + 15];

            mg_eval[1] -= mg_pst_inc[100 & 232][from - 1];

            // eg
            eg_eval[0] -= eg_pst_inc[-100 & 232][from];
            eg_eval[0] += eg_pst_inc[-100 & 232][from + 15];

            eg_eval[1] -= eg_pst_inc[100 & 232][from - 1];

        }
        else if (to == 702) { // black, right
            // mg
            mg_eval[0] -= mg_pst_inc[-100 & 232][from];
            mg_eval[0] += mg_pst_inc[-100 & 232][from + 17];

            mg_eval[1] -= mg_pst_inc[100 & 232][from + 1];

            // eg
            eg_eval[0] -= eg_pst_inc[-100 & 232][from];
            eg_eval[0] += eg_pst_inc[-100 & 232][from + 17];

            eg_eval[1] -= eg_pst_inc[100 & 232][from + 1];
        }
        else if (to == 703) { // white, left
            // mg
            mg_eval[1] -= mg_pst_inc[100 & 232][from];
            mg_eval[1] += mg_pst_inc[100 & 232][from - 17];

            mg_eval[0] -= mg_pst_inc[-100 & 232][from - 1];

            // eg
            eg_eval[1] -= eg_pst_inc[100 & 232][from];
            eg_eval[1] += eg_pst_inc[100 & 232][from - 17];

            eg_eval[0] -= eg_pst_inc[-100 & 232][from - 1];
        }
        else if (to == 704) { // white, right
            // mg
            mg_eval[1] -= mg_pst_inc[100 & 232][from];
            mg_eval[1] += mg_pst_inc[100 & 232][from - 15];

            mg_eval[0] -= mg_pst_inc[-100 & 232][from + 1];

            // eg
            eg_eval[1] -= eg_pst_inc[100 & 232][from];
            eg_eval[1] += eg_pst_inc[100 & 232][from - 15];

            eg_eval[0] -= eg_pst_inc[-100 & 232][from + 1];
        }
    }
}

int Search::quiescence(int shift, int alpha, int beta, int depth) {
    num_nodes_quiescence++;

    // selfdepth
    if (depth > max_depth_rearched) {
        max_depth_rearched = depth;
    }

    bool im_in_check = moves_generator->is_in_check(board, shift == WHITE? search_wpieces.king : search_bpieces.king, shift == WHITE? 1 : -1);

    int val = evaluate(shift);

    int flag_hash = 1;

    // do stand-pat when not in-check
	if (!im_in_check) {
		if (val >= beta) {
			if (depth == MAX_DEPTH)
				zobrist_tt[zobrist_key % zobrist_tt_size].flags = 2;
			return beta;
		}
		if (val > alpha) {
			flag_hash = 3;
			alpha = val;
		}
	}

	int capture_from[44];
	int capture_to[44];

	int capture_num_moves = 0;

	// generate captures only
	int info_moves = generate_attacks(board, capture_from, capture_to, capture_num_moves, shift);
	if (info_moves == 555555555) {
		return 555555555; //king was captured, ilegal.
	}

	ordering_captures(capture_from, capture_to, capture_num_moves, shift);

	int legal_moves = 0;
	for (int i = 0; i < capture_num_moves; i++) {

		val = -quiescence_setup_move(-beta, -alpha, shift, capture_from[i], capture_to[i], depth);

		if (val == -555555555) continue;
		legal_moves++;

		if (val >= beta) {
            if (depth == MAX_DEPTH)
                zobrist_tt[zobrist_key % zobrist_tt_size].flags = 2;
			return beta;
		}
		if (val > alpha) {
		    flag_hash = 3;
			alpha = val;
		}
	}

    if (capture_num_moves == 0) flag_hash = 3;

    // if we are in-check and no legal moves are found, then this position at least is not that good.
    // returns a slightly low value
    if (im_in_check and legal_moves == 0) {
        alpha = -100;
    }

	if (depth == MAX_DEPTH) {
        zobrist_tt[zobrist_key % zobrist_tt_size].flags = flag_hash;
	}
	return alpha;

}


int Search::quiescence_setup_move(int alpha, int beta, int shift, int origin, int dest, int depth) {

    // find real to lol
    int real_to = dest;
	if (dest > 1000 and dest < 2010) {
        real_to = origin + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + dest];
        flag_promotion = 1;
	}
	else if (dest > 700 and dest < 710) {
        // perft test
        flag_enpassant = 1;
        //num_enpassant++;
        // -----
		if (dest == 703) real_to = origin - 17;
		else if ( dest == 704) real_to = origin - 15;

		else if (dest == 701) real_to = origin + 15;
		else if (dest == 702) real_to = origin + 17;

	}

	moves_generator->enpassant = 127;

    // incremental pesto evaluation
	int temp_mg_white = mg_eval[1];
	int temp_mg_black = mg_eval[0];
	int temp_eg_white = eg_eval[1];
	int temp_eg_black = eg_eval[0];
	int temp_game_phase = game_phase;

	flag_capture = 0;

	if (board[real_to] != 0 or (dest > 700 and dest < 710) ) flag_capture = 1;

	// do move
    int* ptr_deleted = NULL;
    int val_deleted;
    int flag_promotion;
    int* ptr_deleted_pawn = NULL;
    make_move(board, origin, dest, ptr_deleted, val_deleted, flag_promotion, ptr_deleted_pawn, shift);

	int val;
	if (shift == WHITE) {
        val = quiescence(BLACK, alpha, beta, depth + 1);
	} else {
        val = quiescence(WHITE, alpha, beta, depth + 1);
	}

    unmake_move(board, origin, dest, ptr_deleted, val_deleted, flag_promotion, ptr_deleted_pawn, shift);

    // incremental pesto evaluation
    mg_eval[1] = temp_mg_white;
    mg_eval[0] = temp_mg_black;
    eg_eval[1] = temp_eg_white;
    eg_eval[0] = temp_eg_black;
    game_phase = temp_game_phase;

	return val;
}

inline void Search::incremental_zobrist(int* board, int from, int to, int shift) {

    int real_to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + to];
    int side_2move;
    int other_side;
    // castle rights, enpassant and side2move data here
	if (shift == WHITE) {
        zobrist_key ^= zobrist_white_moved;
		zobrist_key ^= zobrist_black_moved;
		side_2move = 1;
		other_side = 0;

		// enpassant white
		if (board[from] == WPAWN) {
            if ((from - to) == 32 ) {
                zobrist_key ^= zobrist_enpassant[from - 16];
            }
        }

        // castle right
        if (to == 4003) {
            zobrist_key ^= zobrist_castle[2];
        }
        else if (to == 4004) {
            zobrist_key ^= zobrist_castle[3];
        }
	} else {
		zobrist_key ^= zobrist_black_moved;
		zobrist_key ^= zobrist_white_moved;
		side_2move = 0;
		other_side = 1;

		// enpassant black
		if (board[from] == BPAWN) {
            if ((to - from) == 32) {
                zobrist_key ^=  zobrist_enpassant[from + 16];
            }
        }

         // castle right
        if (to == 4001) {
            zobrist_key ^= zobrist_castle[0];
        }
        else if (to == 4002) {
            zobrist_key ^= zobrist_castle[1];
        }
	}
    // --------------


	if (to <= 127) { // normal move
		zobrist_key ^= zobrist_id[side_2move][abs(board[from])][from]; //remove from
        zobrist_key ^= zobrist_id[other_side][abs(board[to])][to]; //remove dest
        zobrist_key ^= zobrist_id[side_2move][abs(board[from])][to]; //set from to dest
	}
	else if (to > 4000 and to < 4010) { // castle
        if (to == 4001) { // black, long castle
            // remove
            zobrist_key ^= zobrist_id[0][8888][4];
            zobrist_key ^= zobrist_id[0][500][0];

            // add
            zobrist_key ^= zobrist_id[0][8888][2];
            zobrist_key ^= zobrist_id[0][500][3];
        }
        else if (to == 4002) { // black, short castle
            // remove
            zobrist_key ^= zobrist_id[0][8888][4];
            zobrist_key ^= zobrist_id[0][500][7];

            // add
            zobrist_key ^= zobrist_id[0][8888][6];
            zobrist_key ^= zobrist_id[0][500][5];
        }
        else if (to == 4003) { // white, long castle
            // remove
            zobrist_key ^= zobrist_id[1][8888][116];
            zobrist_key ^= zobrist_id[1][500][112];

            // add
            zobrist_key ^= zobrist_id[1][8888][114];
            zobrist_key ^= zobrist_id[1][500][115];
        }
        else if (to == 4004) { // white, short castle
            // remove
            zobrist_key ^= zobrist_id[1][8888][116];
            zobrist_key ^= zobrist_id[1][500][119];

            // add
            zobrist_key ^= zobrist_id[1][8888][118];
            zobrist_key ^= zobrist_id[1][500][117];
        }
	}
	else if (to > 700 and to < 710) { // enpassant
        if (to == 701) { // black, left
            zobrist_key ^= zobrist_id[0][100][from];
            zobrist_key ^= zobrist_id[0][100][from + 15];

            zobrist_key ^= zobrist_id[1][100][from - 1];
        }
        else if (to == 702) { // black, right
            zobrist_key ^= zobrist_id[0][100][from];
            zobrist_key ^= zobrist_id[0][100][from + 17];

            zobrist_key ^= zobrist_id[1][100][from + 1];
        }
        else if (to == 703) { // white, left
            zobrist_key ^= zobrist_id[1][100][from];
            zobrist_key ^= zobrist_id[1][100][from - 17];

            zobrist_key ^= zobrist_id[0][100][from - 1];
        }
        else if (to == 704) { // white, right
            zobrist_key ^= zobrist_id[1][100][from];
            zobrist_key ^= zobrist_id[1][100][from - 15];

            zobrist_key ^= zobrist_id[0][100][from + 1];
        }
	}
	else if (to > 1000 and to < 2010) { // promotion
        int real_to = from + promotion_offset[((shift == WHITE ? 1 : 0) << 4) + to];

        // remove pawn
        zobrist_key ^= zobrist_id[side_2move][100][from];

        // remove attacked piece
        zobrist_key ^= zobrist_id[other_side][abs(board[real_to])][real_to];

        // add promotion piece
        if (to == 2008 or to == 1004 or to == 2007) {
            zobrist_key ^= zobrist_id[side_2move][900][real_to];
        }
        else if (to == 2006 or to == 1003 or to == 2005) {
            zobrist_key ^= zobrist_id[side_2move][500][real_to];
        }
        else if (to == 2004 or to == 1002 or to == 2003) {
            zobrist_key ^= zobrist_id[side_2move][310][real_to];
        }
        else if (to == 2002 or to == 1001 or to == 2001) {
            zobrist_key ^= zobrist_id[side_2move][300][real_to];
        }
	}
}

bool Search::repetition_found() {
    for (int i = 0; i < repetition_index; i++) {
        if (repetition_table[i] == zobrist_key) {
            return true;
        }
    }
    return false;
}

bool Search::move_gives_check(int* board, int from, int to, int shift) {
    bool in_check = false;
    // when to > 127 then move is as relevant as an check
    if (to > 127) return true;
    // ilegal move, end prematurely
    if (abs(board[to]) == 8888) return false;

    int prev_to = board[to];
    board[to] = board[from];
    board[from] = 0;

    if (moves_generator->is_in_check(board, shift == WHITE? search_bpieces.king : search_wpieces.king, shift == WHITE? -1 : 1)) {
        in_check = true;
    }
    board[from] = board[to];
    board[to] = prev_to;

    return in_check;
}




