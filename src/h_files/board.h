#ifndef _BOARD_H_
#define _BOARD_H_

#include <cstdint>
#include <vector>
#include <string>
#include <map>

struct board {
    // member variables
    uint64_t bins[2][7] =
    {{
        0b0000000000000000000000000000000000000000000000001111111100000000, // black pawns
        0b0000000000000000000000000000000000000000000000000000000010000001, // black rooks
        0b0000000000000000000000000000000000000000000000000000000001000010, // black knights
        0b0000000000000000000000000000000000000000000000000000000000100100, // black bishops
        0b0000000000000000000000000000000000000000000000000000000000010000, // black queens
        0b0000000000000000000000000000000000000000000000000000000000001000, // back king
        0b0000000000000000000000000000000000000000000000001111111111111111 // all black
     },{
        0b0000000011111111000000000000000000000000000000000000000000000000, // white pawns
        0b1000000100000000000000000000000000000000000000000000000000000000, // white rooks
        0b0100001000000000000000000000000000000000000000000000000000000000, // white knights
        0b0010010000000000000000000000000000000000000000000000000000000000, // white bishops
        0b0001000000000000000000000000000000000000000000000000000000000000, // white queens
        0b0000100000000000000000000000000000000000000000000000000000000000, // white king
        0b1111111111111111000000000000000000000000000000000000000000000000 // all white
     }};

    bool can_castle_long_white = true; // These need to be modified when pieces moved.
    bool can_castle_short_white = true;
    bool can_castle_long_black = true;
    bool can_castle_short_black = true;    

    // member functions
    void display(int quiet = 1);
    std::vector<board> display_legal_moves(bool side, int depth = 4);
    std::vector<board> all_legal_moves(bool to_move);
    bool is_checked(bool side);
    uint64_t compute_mask(bool to_move, int type, uint64_t start);
    uint64_t compute_white_pawn_mask(uint64_t start);
    uint64_t compute_black_pawn_mask(uint64_t start);
    uint64_t compute_knight_mask(uint64_t start, bool side);
    uint64_t compute_pawn_mask_check(uint64_t start, bool side);
    uint64_t compute_rook_mask(uint64_t start, bool side);
    uint64_t compute_bishop_mask(uint64_t position, bool side);
    uint64_t compute_king_mask(uint64_t position, bool side);
    void move(std::pair<uint64_t, uint64_t> movement, char promo = 0);
    void move(const std::string& movement);
    void read_moves(std::vector<std::string> moves_history);
    double eval2();
    double eval();
};

std::string bitboard_to_position(uint64_t bitboard);
#endif
