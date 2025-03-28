#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#include <cmath>
#include "board.h"

// render
std::string render(bool turn, int type);

// display
void display_uint64(uint64_t num);

//load moves from hist into vector
std::vector<std::string>  load_moves(std::string input_path);

// timer
void benchmark(const std::function<void()>& func);

std::string translate_to_string(uint64_t);

//compute the final move
std::string final_move(board pre_move, board post_move, bool is_playing_white);

//Compute output csv file. new_move should be output from function final_move
void create_output_file(const std::string& original_file, const std::string& output_file, board pre_move, board post_move);

// find best evaluation at depth
double minimax(board& a_game, int depth, bool side, double alpha = -INFINITY, double beta = INFINITY);

// master function to link all things to do
void master(std::string input_path, std::string output_path, int depth = 4, int quiet = 1);

// interactive game
void play_me(std::string hist, std::string buff, bool starts = 0);

// constexpr functions

constexpr uint64_t position_to_bitboard(const std::string& position) { // position "e4"
    char file = position[0];
    char rank = position[1];

    // Calculate index
    int file_index = file - 'a';      // 'a' -> 0, ..., 'h' -> 7
    int rank_index = rank - '1';     // '1' -> 0, ..., '8' -> 7
    int bit_index = 63 - (rank_index * 8 + file_index); // Flip the index for MSB-first

    // Return the bitboard representation
    return 1LL << bit_index;
}

#endif
