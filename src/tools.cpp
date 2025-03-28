#include <functional>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <map>
#include <bitset>
#include "h_files/tools.h"
#include "h_files/board.h"

// render
static const std::string map[2][7] =
{{
    "♙", "♖", "♘", "♗", "♕", "♔", "█"
},{
    "♟", "♜", "♞", "♝", "♛", "♚", "░"
}};

std::string render(bool turn, int type) {
    return map[turn][type];
}

// display_uint64
void display_uint64(uint64_t num) {
    for (int rank = 0; rank < 8; rank++) {
        if (rank != 0) {
            std::cout << "|" << std::endl;
        }

        for (int file = 7; file >= 0; file--) {
            int square = rank * 8 + file;
            uint64_t position = 1ll << square; // bitmask for the current square
            std::cout << "|";
            if ((num & position) != 0) {
                std::cout << "*";
            } else {
                std::cout << " ";
            }
        }
    }
    std::cout << "|" << std::endl;
    std::cout << std::endl;
}

//load moves from hist file into vector
std::vector<std::string> load_moves(std::string input_path){
    // Open the file
        std::ifstream file(input_path);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open the file " << input_path << std::endl;
            exit(1);
        }

        std::vector<std::string> history;
        std::string line;

        // Read each line from the file
        while (std::getline(file, line)) {
            history.push_back(line);
        }

        // Close the file
        file.close();
        
        return history;
}

// Function to benchmark another function
// Allows passing arguments as a lambda or bound function
void benchmark(const std::function<void()>& func) {
    const int iterations = 1000;

    // Record the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Run the function 100,000 times
    for (int i = 0; i < iterations; ++i) {
        func();
    }

    // Record the end time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Execution time: "
              << duration.count() / 1000 << " ns" << std::endl;
}

// final move
//Function that returns moves from two boards
std::string final_move(board pre_move, board post_move, bool is_playing_white){
    
    uint64_t pre_move_pieces = pre_move.bins[is_playing_white][6];
    uint64_t post_move_pieces = post_move.bins[is_playing_white][6];

    //xor between moved pieces
    uint64_t moved_pieces = pre_move_pieces ^ post_move_pieces;

    //compute 1s where piece have been moved from
    uint64_t init_pos = moved_pieces & pre_move_pieces;
    //compute 1s where piece have been moved to
    uint64_t finish_pos = moved_pieces & post_move_pieces;

    //castling moves
    uint64_t kingside_white = 0b0000111100000000000000000000000000000000000000000000000000000000;
    uint64_t queenside_white = 0b1011100000000000000000000000000000000000000000000000000000000000;
    uint64_t kingside_black = 0b0000000000000000000000000000000000000000000000000000000000001111;
    uint64_t queenside_black = 0b0000000000000000000000000000000000000000000000000000000010111000;
    //Castling
    if (is_playing_white && (moved_pieces == kingside_white || moved_pieces == queenside_white)){
        if (moved_pieces == kingside_white){
            return "e1g1";
        }
        else{
            return "e1c1";
        }
    }
    else if(!is_playing_white && (moved_pieces == kingside_black || moved_pieces == queenside_black)){
        if (moved_pieces == kingside_black){
            return "e8g8";
        }
        else{
            return "e8c8";
        }
    }

    //translate moves into a string
    std::string initial = bitboard_to_position(init_pos);
    std::string final = bitboard_to_position(finish_pos);



    //Check for promotion
    //Firstly, check if moved piece was pawn
    bool pawn_moved = 0;
    if(pre_move.bins[is_playing_white][0]^post_move.bins[is_playing_white][0]){
        pawn_moved = 1;
    }
    
    uint64_t rank_1 = 0b1111111100000000000000000000000000000000000000000000000000000000;
    uint64_t rank_8 = 0b0000000000000000000000000000000000000000000000000000000011111111;
    bool promo_rank_reached = (is_playing_white && (finish_pos & rank_8)) || (!is_playing_white && (finish_pos & rank_1));
    std::string type = "";

    //Adds promotion piece if relevant
    if (pawn_moved && promo_rank_reached){

        //std::cout << "Inside of if" << std::endl;
        //Find the piece it becomes post_move
        int promo = 0;
        for(int i = 1; i<5;i++){
            if(pre_move.bins[is_playing_white][i]^post_move.bins[is_playing_white][i]){
                promo = i;
                
            }
                      
        }
        std::cout << "print i:" << promo << std::endl;
        if (promo) {
        switch (promo) {
            case 1:
                type = 'r';
                break;
            case 2:
                type = 'k';
                break;
            case 3:
                type = 'b';
                break;
            case 4:
                type = 'q';
                break;
        }
    }
    final += type;
    }
    
    //return intial string of move
    return initial + final;
}


// make output file
//Function computing output file. new_move should be output from function final_move
void create_output_file(const std::string& original_file, const std::string& final_file, std::string new_move){

    //creates output file if it does not exist already, otherwise it overwrites it
    std::ofstream output_file(final_file);
    if (!output_file.is_open()){
        std::cerr << "File cannot be opened" << std::endl;
        return;
    }

    //Add the move
    output_file << new_move << std::endl;

    //Close file
    output_file.close();
}

// minimax
double minimax(board& a_game, int depth, bool side, double alpha, double beta) {
    if (depth == 0) {
        return a_game.eval2();
    }

    std::vector<board> all_legal_moves = a_game.all_legal_moves(side);

    if (side) {
        double maxEval = -INFINITY;
        for (auto& next : all_legal_moves) {
            // next.display();
            double evaluation = minimax(next, depth - 1, false, alpha, beta);
            maxEval = std::max(maxEval, evaluation);
            alpha = std::max(alpha, evaluation);
            if (beta <= alpha) break;
        }
        return maxEval;
    }
    else {
        double minEval = INFINITY;
        for (auto& next : all_legal_moves) {
            // next.display();
            double evaluation = minimax(next, depth - 1, true, alpha, beta);
            minEval = std::min(minEval, evaluation);
            beta = std::min(beta, evaluation);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

// master function to read board, minimax through possible boards, and return the csv
void master(std::string input_path, std::string output_path, int depth, int quiet) {

    // start counting time passed for timeout
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
    auto timeout = std::chrono::milliseconds(9000);
    start_time = std::chrono::high_resolution_clock::now();

    // load moves from csv into strings vector
    std::vector<std::string> history = load_moves(input_path);

    // initial setup
    board* game;
    game = new board;
    int to_move;
    // for iteration on legal moves
    int i;
    double ith_eval;
    int best_index = 0;
    double best_eval;
    // other steps
    std::vector<board> next;
    std::string move;

    if (quiet < 3) {
        std::cout << "reading moves... "; // step one
    }

    // find turn (white or black) and update the board with history
    to_move = (history.size() + 1) % 2;
    game->read_moves(history);

    if (quiet < 3) {
        std::cout << "done." << std::endl << "finding moves... "; // step two
    }

    // if quiet == 0, display all the possible moves and their evaluation, otherwise, compute them quietly
    if (!quiet) {
        next = game->display_legal_moves(to_move, depth);
    }
    else {
        next = game->all_legal_moves(to_move);
    }

    if (quiet < 3) {
        std::cout << "done." <<std::endl << "finding best one..."; // step three
    }

    // find move with best eval out of options, stored in "next" variable
    if (to_move) {
        best_eval = -INFINITY;

        for (i = 0; i < next.size(); i++) {
            // check time left: if < 0.5 seconds, break out
            end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            if (duration > timeout) break;

            ith_eval = minimax(next[i], depth, !to_move);
            if (ith_eval > best_eval) {
                best_eval = ith_eval;
                best_index = i;
            }
        }
    }
    else {
        best_eval = INFINITY;

        for (i = 0; i < next.size(); i++) {
            // check time left: if < 0.5 seconds, break out
            end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            if (duration > timeout) break;

            ith_eval = minimax(next[i], depth, !to_move);
            if (ith_eval < best_eval) {
                best_eval = ith_eval;
                best_index = i;
            }
        }
    }

    if (quiet < 3) {
        std::cout << "done." << std::endl << std::endl << "original board:" << std::endl;
        game->display(1);
        std::cout << "new board:" << std::endl;
        next[best_index].display(1);
        std::cout << "creating move string... "; // step four
    }

    move = final_move(*game, next[best_index], to_move);

    if (quiet < 3) {
        std::cout << "done, played " << move << std::endl << "creating output file... "; // step five
    }

    // pass output move to output file
    create_output_file(input_path, output_path, move);

    if (quiet < 3) {
        std::cout << "done." << std::endl << std::endl;
        if (quiet < 2) {
            end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "took " << duration << " to execute." << std::endl;
        }
    }
}

// interactive game
void play_me(std::string hist, std::string buff, bool starts) {
    std::string your_move;
    std::string line;
    
    //open and empty hist file
    std::ofstream hist_file(hist, std::ios::trunc);
    if (!hist_file.is_open()){
        std::cerr << "File cannot be opened" << std::endl;
        return;
    }

    if (starts) {
        // add your beginning move
        std::cout << "your move: (type none to end) ";
        std::cin >> your_move;
        if ("none" != your_move) {
            hist_file << your_move << std::endl;
        } else {
            std::cout << "no move, ended game" << std::endl;
            return;
        }

    }

    hist_file.close();

    while (1) {
        master(hist, buff);

        //open hist and output files
        std::ofstream hist_file(hist, std::ios::app);
        std::ifstream buff_file(buff);
        if (!hist_file.is_open() || !buff_file.is_open()){
            std::cerr << "File cannot be opened" << std::endl;
            return;
        }

        if (std::getline(buff_file, line)) {
            std::cout << "your move: (type none to end) ";
            std::cin >> your_move;
            if ("none" != your_move) {
                hist_file << line << std::endl << your_move << std::endl;
            } else {
                std::cout << "no move, ended game" << std::endl;
                return;
            }
        }
    
        hist_file.close();
        buff_file.close();
    }
}
















