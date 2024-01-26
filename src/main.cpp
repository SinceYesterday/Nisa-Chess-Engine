#include "wtypes.h"
#include "Engine.h"
#include "Search.h"
#include <iostream>
#include <ctime>
#include <thread>

char board[64];
int side;
int enpassant;
int castle_rights[4];
int go_depth;
std::string fen;

Engine* engine;

std::unordered_map<std::string, int> map_algebraic_notation;

std::string algebraic_notation[64] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

int x64_to_x88[64] {
    0,   1,   2,   3,   4,   5,   6,   7,
    16,  17,  18,  19,  20,  21,  22,  23,
    32,  33,  34,  35,  36,  37,  38,  39,
    48,  49,  50,  51,  52,  53,  54,  55,
    64,  65,  66,  67,  68,  69,  70,  71,
    80,  81,  82,  83,  84,  85,  86,  87,
    96,  97,  98,  99,  100, 101, 102, 103,
    112, 113, 114, 115, 116, 117, 118, 119};

void ucinewgame() {
    engine->search->repetition_index = 0;

    for (int i = 0; i < 64; i++) {
        map_algebraic_notation.insert({algebraic_notation[i],i});
    }

    side = -1;
    enpassant = 127;
    for (int i = 0; i < 4; i++) {
        castle_rights[i] = 1;
    }
    go_depth = 4;
    fen = "";
    for (int i = 0; i < 64; i++) {
        board[i] = 0;
    }
}

int update_fen() {
    fen = "";
    int counter = 0;
    for (int i = 0; i < 64; i++) {
        if (board[i] != '-') {
            if (counter) {
                fen += (counter + '0');
                counter = 0;
            }
            fen += board[i];
        } else {
            counter++;
        }
        if (((i + 1) % 8) == 0) {
            if (counter) {
                fen += (counter + '0');
            }
            fen += '/';
            counter = 0;
        }
    }
}

// generate a zobrist key from current position for a repetition table
void add_repetition() {
    engine->set_side(side);
    engine->set_castle_rights(castle_rights);
    engine->set_fen(fen);
    engine->set_enpassant(enpassant);

    // copy engine board to search board before create zobrist key
    for (int i = 0; i < 128; i++) {
        engine->search->board[i] = engine->board->x88_representation[i];
    }

    // generate zobrist key
    engine->search->start_zobrist();
    engine->search->repetition_table[engine->search->repetition_index++] = engine->search->zobrist_key;
}

int update_board(std::string fen) {
    std::unordered_map<char, int> valid_char;
    char char_pieces[] = {'K' , 'Q', 'R', 'B', 'N', 'P', 'k' , 'q', 'r', 'b', 'n', 'p',};
    for (int i = 0; i < 12; i++) {
        valid_char.insert({char_pieces[i], 1});
    }

    char cur_board[64];
    for (int i = 0; i < 64; i++) cur_board[i] = '-';

    int count_valid = 0;
    int indice = 0;
    for (int i = 0; i < fen.length(); i++) {
        if (valid_char.count(fen[i]) > 0) {
            count_valid++;
            cur_board[indice++] = fen[i];
        } else if (fen[i] == '/') {
            continue;
        } else {
            count_valid += (fen[i] - '0');
            indice += (fen[i] - '0');
        }
    }

    if (count_valid != 64) {
        return 0;
    }

    for (int i = 0; i < 64; i++) board[i] = cur_board[i];

    return 1;
}

std::string get_next_command(std::string& input) {
    int i = input.find(' ');
    if (i != std::string::npos) {
        std::string next_command = input.substr(0, i);
        input = input.substr(i + 1, input.length() - (i + 1));
        return next_command;
    } else {
        std::string temp = input;
        input = "";
        return temp;
    }
    return "";
}

int parse_fen(std::string input) {
    fen = get_next_command(input);
    update_board(fen);
    std::string active_color = get_next_command(input);
    if (active_color == "w") {
        side = 1;
    } else if (active_color == "b") {
        side = 0;
    } else {
        return 0;
    }

    // castling rights
    std::string castling = get_next_command(input);
    for (int i = 0; i < 4; i++)
        castle_rights[i] = 0;
    for (int i = 0; i < castling.length(); i++) {
        if (castling[i] == 'K') {
            castle_rights[0] = 1;
        } else if (castling[i] == 'Q') {
            castle_rights[1] = 1;
        } else if (castling[i] == 'k') {
            castle_rights[2] = 1;
        } else if (castling[i] == 'q') {
            castle_rights[3] = 1;
        }
    }

    std::string enpassant = get_next_command(input);
    //set enpassant data

    std::string halfmove = get_next_command(input);
    //to do

    std::string fullmove = get_next_command(input);
    //to do

    // update engine here

    return 1;
}

int parse_moves(std::string& input) {
    std::string next_move;
    while ( (next_move = get_next_command(input)) != "" ) {
        side = side == 1? 0 : 1;
        enpassant = 127;

        // handling castle
        if (next_move == "e1g1" and board[60] == 'K') {
            board[60] = '-';
            board[63] = '-';
            board[62] = 'K';
            board[61] = 'R';
            castle_rights[2] = 0;
            castle_rights[3] = 0;
            continue;
        } else if (next_move == "e1c1" and board[60] == 'K') {
            board[60] = '-';
            board[56] = '-';
            board[58] = 'K';
            board[59] = 'R';
            castle_rights[2] = 0;
            castle_rights[3] = 0;
            continue;
        } else if (next_move == "e8g8" and board[60 - 56] == 'k') {
            board[60 - 56] = '-';
            board[63 - 56] = '-';
            board[62 - 56] = 'k';
            board[61 - 56] = 'r';
            castle_rights[0] = 0;
            castle_rights[1] = 0;
            continue;
        } else if (next_move == "e8c8" and board[60 - 56] == 'k') {
            board[60 - 56] = '-';
            board[56 - 56] = '-';
            board[58 - 56] = 'k';
            board[59 - 56] = 'r';
            castle_rights[0] = 0;
            castle_rights[1] = 0;
            continue;
        }

        // handling normal moves
        std::string move_from = "";
        move_from.push_back(next_move[0]);
        move_from.push_back(next_move[1]);

        std::string move_to = "";
        move_to.push_back(next_move[2]);
        move_to.push_back(next_move[3]);

        int from = map_algebraic_notation[move_from];
        int to = map_algebraic_notation[move_to];

        char pc_from = board[from];
        char pc_to = board[to];
        board[from] = '-';
        board[to] = pc_from;

        // check if enpassant ocurrs
        if (pc_from == 'p' or pc_from == 'P') {
            int dif = abs(from - to);
            if (dif == 16) {
                if (side == 1) {
                    enpassant = x64_to_x88[from - 8];
                } else {
                    enpassant = x64_to_x88[from + 8];
                }
            }
        }

        // check if the capture is an enpassant
        if (pc_from == 'p') {
            int dif = abs(from - to);
            if (dif == 7 and pc_to == '-') {
                board[from - 1] = '-';
            }
            else if (dif == 9 and pc_to == '-') {
                board[from + 1] = '-';
            }
        } else if (pc_from == 'P') {
            int dif = abs(from - to);
            if (dif == 7 and pc_to == '-') {
                board[from + 1] = '-';
            }
            else if (dif == 9 and pc_to == '-') {
                board[from - 1] = '-';
            }
        }

        // handling promotions
        if (next_move.length() == 5) {
            char c = std::tolower(next_move[4]);
            if (!side) {
                if (c == 'q') board[to] = 'Q';
                else if (c == 'r') board[to] = 'R';
                else if (c == 'n') board[to] = 'N';
                else if (c == 'b') board[to] = 'B';
            } else {
                if (c == 'q') board[to] = 'q';
                else if (c == 'r') board[to] = 'r';
                else if (c == 'n') board[to] = 'n';
                else if (c == 'b') board[to] = 'b';
            }
        }

        // handling castle rights
        if (move_from == "e1") {
            castle_rights[2] = 0;
            castle_rights[3] = 0;
        }
        else if (move_from == "a1" or move_to == "a1") {
            castle_rights[2] = 0;
        }
        else if (move_from == "h1" or move_to == "h1") {
            castle_rights[3] = 0;
        }

        if (move_from == "e8") {
            castle_rights[0] = 0;
            castle_rights[1] = 0;
        }
        else if (move_from == "a8" or move_to == "a8") {
            castle_rights[0] = 0;
        }
        else if (move_from == "h8" or move_to == "h8") {
            castle_rights[1] = 0;
        }

    }
    update_fen();

    // add position to repetition table
    add_repetition();

    return 1;
}

int parse_position(std::string& input) {
    std::string cur_command = get_next_command(input);
    if (cur_command == "fen") {
        parse_fen(input);
    } else if (cur_command == "startpos") {
        side = 1;
        std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        parse_fen(startpos);
    } else {
        return 0;
    }

    cur_command = get_next_command(input);
    if (cur_command == "moves") {
        parse_moves(input);
    }

    return 1;
}

void print_board() {
    for (int i = 0; i < 64; i++) {
        std::cout << " " << board[i] << " ";

        if ( (i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}

HANDLE t;

// non-blocking input
// code took from VICE Engine
int input_waiting() {
#ifndef WIN32
    fd_set readfds;
    struct timeval tv;
    FD_ZERO (&readfds);
    FD_SET (fileno(stdin), &readfds);
    tv.tv_sec=0;
    tv.tv_usec=0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    }

    else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
}

// non-blocking input
// code took from VICE engine
DWORD WINAPI t_function(LPVOID data) {

    while (true) {
        if (input_waiting()) {
            //close(fileno);
            int bytes;

            // GUI/user input
            char input[256] = "",*endc;

            do {
                // read bytes from STDIN
                bytes = read(fileno(stdin), input, 256);
            }

            // until bytes available
            while (bytes < 0);

            endc = strchr(input,'\n');

            // if found new line set value at pointer to 0
            if (endc) *endc=0;

            if (strlen(input) > 0) {
                // match UCI "quit" command
                if (!strncmp(input, "stop", 4))
                    // tell engine to end search
                    engine->search->time_over = true;
            }
        }

        if (engine->search->time_over) break;

        Sleep(10);
    }

    return 1;
}

int parse_go(std::string& input) {

    // run a thread for a non-blocking input while searching
    t = CreateThread(NULL, 0, t_function, &engine->search->time_over, 0, NULL );

    get_next_command(input);
    int wtime = stoi(get_next_command(input));
    get_next_command(input);
    int btime = stoi(get_next_command(input));

    if (side == 1) {
        engine->search->time_limit = wtime / 25;
    } else {
        engine->search->time_limit = btime / 25;
    }

    engine->search->time_over = false;

    engine->set_side(side);
    engine->set_castle_rights(castle_rights);
    engine->set_fen(fen);
    engine->set_enpassant(enpassant);
    engine->calculate();

    // set time over to end theard
    engine->search->time_over = true;

    WaitForMultipleObjects(1, &t, TRUE, INFINITE);
    CloseHandle(t);
    return 1;
}

int main (int argc, char** argv) {
    engine = new Engine();
    ucinewgame();

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        std::string command = get_next_command(input);

        if (command == "position") {
            parse_position(input);
        } else if (command == "go") {
            parse_go(input);
        } else if (command == "ucinewgame") {
            ucinewgame();
        } else if (command == "uci") {
            std::string name[7];

            name[0] = " _   _ _            ";
            name[1] = "| \\ | (_)___  __ _ ";
            name[2] = "|  \\| | / __|/ _` |";
            name[3] = "| |\\  | \\__ \\ (_| |";
            name[4] = "|_| \\_|_|___/\\__,_|";
            name[5] = " ";
            name[6] = "Nisa Chess Engine";
            for (int i = 0; i < 7; i++) {
                for (int j = 0; j < 2; j++ )
                    std::cout << " ";
                std::cout << name[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "id name Nisa" << std::endl;
            std::cout << "id author Fernando Parada" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (command == "quit"){
            break;
        } else {

        }

    }
}

