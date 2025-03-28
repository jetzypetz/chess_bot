// change all displays to have caption parameter
// compute masks - change parameters
// render - change size of array
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "h_files/board.h"
#include "h_files/tools.h"
#include "h_files/constants.h"

void board::display(int quiet) {
    uint64_t all_pieces = bins[1][6] | bins[0][6];
    if (quiet < 2) {
        for (int k=0; k<2;k++) {
            for (int l=0;l<2;l++) {
                for (int m=0;m<3;m++) {
                    std::cout << render(k,3*l + m) << ": "
                        << __builtin_popcountll(bins[k][3*l + m]) << " ";
                    if (quiet == 0) {
                        std::cout << std::endl;
                        display_uint64(bins[k][3*l + m]);
                    }
                }
                std::cout << std::endl;
            }
            if (quiet == 0) {
                display_uint64(bins[k][6]);
            }
            std::cout << std::endl;
        }
    }

    for (int rank = 0; rank < 8; rank++) {
        if (rank != 0) {
            std::cout << "|" << std::endl;
        }

        for (int file = 7; file >= 0; file--) {
            int square = rank * 8 + file;
            uint64_t position = 1LL << square; // Bitmask for the current square
            std::cout << "|";
            if ((all_pieces & position) != 0) {
                for (int i = 0; i < 6; i++) {
                    if ((position & bins[0][i]) != 0) {
                        std::cout << render(0,i);
                        break;
                    } else if ((position & bins[1][i]) != 0) {
                        std::cout << render(1,i);
                        break;
                    }
                }
            }
            else {
                std::cout << " ";
            }
        }
    }
    std::cout << "|" << std::endl << std::endl;
}

// display all legal moves from a position
std::vector<board> board::display_legal_moves(bool side, int depth) {
    std::cout << "original board:" << std::endl;
    std::cout << "eval: " << minimax(*this, depth+1, side) << std::endl;
    display();
    std::cout << "all possible new positons for "
        << (side ? "white" : "black") << std::endl;
    std::vector<board> next = all_legal_moves(side);
    int i = 0;
    for (auto b: next) {
        std::cout << i << ", with eval "
            << minimax(b, depth, !side) << ":" << std::endl;
        b.display(1);
        i++;
    }
    return next;
}

std::vector<board> board::all_legal_moves(bool to_move) {
    // new board generator
    std::vector<board> boards;

    uint64_t pieces;
    int nb_pieces;
    uint64_t start; // piece starting point
    uint64_t options;
    int nb_options;
    uint64_t end; // piece destination
    
    int attacking; // attacking piece type
    int dying = -1; // dying piece type
    int promoting = 0; // promoting type
    int castling_rights_change = 0;    // 0 -> no change
                                       // 1 -> losing white
                                       // 2 -> losing black
                                       // 3 -> losing white short
                                       // 4 -> losing white long
                                       // 5 -> losing black short
                                       // 6 -> losing black long
    
    for (attacking = 0; attacking < 6; attacking++) {
        pieces = bins[to_move][attacking];
        nb_pieces = __builtin_popcountll(pieces);
        for (int i = 0; i < nb_pieces; i++) {
            start = pieces & -pieces;
            pieces -= start;
            options = compute_mask(to_move, attacking, start);
            nb_options = __builtin_popcountll(options);
            for (int j = 0; j < nb_options; j++) {
                end = options & -options;
                options -= end;

                // ### UPDATE BOARD ###

                // check if losing castling rights
                if (attacking == 5) {
                    if (to_move && start & e1) {
                        if (can_castle_short_white) {
                            if (can_castle_long_white) {
                                castling_rights_change = 1;
                            } else {
                                castling_rights_change = 3;
                            }
                        } else {
                            if (can_castle_long_white) {
                                castling_rights_change = 4;
                            } else {
                                castling_rights_change = 0;
                            }
                        } 
                    } else if (!to_move && start & e8) {
                        if (can_castle_short_black) {
                            if (can_castle_long_black) {
                                castling_rights_change = 2;
                            } else {
                                castling_rights_change = 5;
                            }
                        } else {
                            if (can_castle_long_black) {
                                castling_rights_change = 6;
                            } else {
                                castling_rights_change = 0;
                            }
                        } 
                    }
                } else {
                    if (can_castle_short_white &&
                            ((to_move && start == h1)
                         || (!to_move && end == h1))) {
                        castling_rights_change = 3;
                    } else if (can_castle_long_white &&
                            ((to_move && start == a1)
                         || (!to_move && end == a1))) {
                        castling_rights_change = 4;
                    } else if (can_castle_short_black &&
                            ((!to_move && start == h8)
                          || (to_move && end == h8))) {
                        castling_rights_change = 5;
                    } else if (can_castle_long_black &&
                            ((!to_move && start == a8)
                          || (to_move && end == a8))) {
                        castling_rights_change = 6;
                    }
                }


                // move attacking piece
                bins[to_move][attacking] ^= start;
                bins[to_move][6] ^= start;
                // special case handled later if promoting pawn
                if (attacking == 0 && end & END_RANK[to_move]) {
                    promoting = 1;
                } else {
                    bins[to_move][attacking] ^= end;
                    bins[to_move][6] ^= end;
                }

                // remove dying piece
                if (bins[!to_move][6] & end) {
                    for (dying = 0; dying < 6; dying++) {
                        if (bins[!to_move][dying] & end) {
                            bins[!to_move][dying] ^= end;
                            bins[!to_move][6] ^= end;
                            break;
                }   }   }


                // if legal add board
                if (!is_checked(to_move)) {
                    if (promoting) {
                        bins[to_move][promoting] ^= end;
                        bins[to_move][6] ^= end;
                        boards.push_back(*this);
                        bins[to_move][promoting] ^= end;
                        bins[to_move][6] ^= end;

                        promoting = 4;

                        bins[to_move][promoting] ^= end;
                        bins[to_move][6] ^= end;
                        boards.push_back(*this);
                        bins[to_move][promoting] ^= end;
                        bins[to_move][6] ^= end;
                    } else {
                        switch (castling_rights_change) {
                            case 0:
                                boards.push_back(*this);
                                break;
                            case 1:
                                can_castle_short_white = false;
                                can_castle_long_white = false;
                                boards.push_back(*this);
                                can_castle_short_white = true;
                                can_castle_long_white = true;
                                break;
                            case 2:
                                can_castle_short_black = false;
                                can_castle_long_black = false;
                                boards.push_back(*this);
                                can_castle_short_black = true;
                                can_castle_long_black = true;
                                break;
                            case 3:
                                can_castle_short_white = false;
                                boards.push_back(*this);
                                can_castle_short_white = true;
                                break;
                            case 4:
                                can_castle_long_white = false;
                                boards.push_back(*this);
                                can_castle_long_white = true;
                                break;
                            case 5:
                                can_castle_short_black = false;
                                boards.push_back(*this);
                                can_castle_short_black = true;
                                break;
                            case 6:
                                can_castle_long_black = false;
                                boards.push_back(*this);
                                can_castle_long_black = true;
                                break;
                        }
                        castling_rights_change = 0;
                    }
                }

                // unmove attacking piece
                bins[to_move][attacking] ^= start;
                bins[to_move][6] ^= start;
                if (!promoting) {
                    bins[to_move][attacking] ^= end;
                    bins[to_move][6] ^= end;
                } else {
                    promoting = 0;
                }

                // readd dying piece
                if (dying != -1) {
                    bins[!to_move][dying] ^= end;
                    bins[!to_move][6] ^= end;
                    dying = -1;
                }
            }
        }
    }
    
    if (!to_move) {
        if (can_castle_short_black) {
            if (!(bins[0][6] & CASTLE_BETWEEN[0][0])) {
                if (!is_checked(0)) { // cannot castle when in check
                    bins[0][5] ^= CASTLE_MOVEMENT[0][0][0]; // move the king
                    bins[0][6] ^= CASTLE_MOVEMENT[0][0][0]; // move the king in all pieces
                    bins[0][1] ^= CASTLE_MOVEMENT[0][0][1]; // move the rook
                    bins[0][6] ^= CASTLE_MOVEMENT[0][0][1]; // move the rook in all pieces
                    if (!is_checked(0)) { // cannot castle into check
                        can_castle_short_black = false;
                        can_castle_long_black = false;
                        boards.push_back(*this);
                        can_castle_short_black = true;
                        can_castle_long_black = true;
                    }
                    bins[0][5] ^= CASTLE_MOVEMENT[0][0][0]; // move the king back
                    bins[0][6] ^= CASTLE_MOVEMENT[0][0][0]; // move the king back in all pieces
                    bins[0][1] ^= CASTLE_MOVEMENT[0][0][1]; // move the rook back
                    bins[0][6] ^= CASTLE_MOVEMENT[0][0][1]; // move the rook back in all pieces
                }
            }
        }
        if (can_castle_long_black) {
            if (!(bins[0][6] & CASTLE_BETWEEN[0][1])) {
                if (!is_checked(1)) {
                    bins[0][5] ^= CASTLE_MOVEMENT[0][1][0];
                    bins[0][6] ^= CASTLE_MOVEMENT[0][1][0];
                    bins[0][1] ^= CASTLE_MOVEMENT[0][1][1];
                    bins[0][6] ^= CASTLE_MOVEMENT[0][1][1];
                    if (!is_checked(0)) {
                        boards.push_back(*this);
                        can_castle_short_black = false;
                        can_castle_long_black = false;
                        boards.push_back(*this);
                        can_castle_short_black = true;
                        can_castle_long_black = true;
                    }
                    bins[0][5] ^= CASTLE_MOVEMENT[0][1][0];
                    bins[0][6] ^= CASTLE_MOVEMENT[0][1][0];
                    bins[0][1] ^= CASTLE_MOVEMENT[0][1][1];
                    bins[0][6] ^= CASTLE_MOVEMENT[0][1][1];
                }
            }
        }
    }
    else {
        if (can_castle_short_white) {
            if (!(bins[1][6] & CASTLE_BETWEEN[1][0])) {
                if (!is_checked(1)) {
                    bins[1][5] ^= CASTLE_MOVEMENT[1][0][0];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][0][0];
                    bins[1][1] ^= CASTLE_MOVEMENT[1][0][1];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][0][1];
                    if (!is_checked(1)) {
                        can_castle_long_white = false;
                        can_castle_short_white = false;
                        boards.push_back(*this);
                        can_castle_long_white = true;
                        can_castle_short_white = true;
                    }
                    bins[1][5] ^= CASTLE_MOVEMENT[1][0][0];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][0][0];
                    bins[1][1] ^= CASTLE_MOVEMENT[1][0][1];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][0][1];
                }
            }
        }
        if (can_castle_long_white) {
            if (!(bins[1][6] & CASTLE_BETWEEN[1][1])) {
                if (!is_checked(1)) {
                    bins[1][5] ^= CASTLE_MOVEMENT[1][1][0];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][1][0];
                    bins[1][1] ^= CASTLE_MOVEMENT[1][1][1];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][1][1];
                    if (!is_checked(1)) {
                        can_castle_long_white = false;
                        can_castle_short_white = false;
                        boards.push_back(*this);
                        can_castle_long_white = true;
                        can_castle_short_white = true;
                    }
                    bins[1][5] ^= CASTLE_MOVEMENT[1][1][0];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][1][0];
                    bins[1][1] ^= CASTLE_MOVEMENT[1][1][1];
                    bins[1][6] ^= CASTLE_MOVEMENT[1][1][1];
                }
            }
        }
    }

    return boards;
}

bool board::is_checked(bool side) {
    // pawn
    uint64_t pawn_attack_mask = compute_pawn_mask_check(bins[side][5], side);
    if (pawn_attack_mask & bins[!side][0]) return true;

    // rook and queen
    uint64_t rook_attack_mask = compute_rook_mask(bins[side][5], side);
    if (rook_attack_mask & (bins[!side][1] | bins[!side][4])) return true;

    // knight
    uint64_t knight_attack_mask = compute_knight_mask(bins[side][5], side);
    if (knight_attack_mask & bins[!side][2]) return true;

    // bishop and queen
    uint64_t bishop_attack_mask = compute_bishop_mask(bins[side][5], side);
    if (bishop_attack_mask & (bins[!side][3] | bins[!side][4])) return true;

    return false;
}

uint64_t board::compute_mask(bool to_move, int type, uint64_t start) {
    if (type == 0) {
        if (to_move) {
            return compute_white_pawn_mask(start);
        }
        else {
            return compute_black_pawn_mask(start);
        }
    }
    else if (type == 1) {
        return compute_rook_mask(start, to_move);
    }
    else if (type == 2) {
        return compute_knight_mask(start, to_move);
    }
    else if (type == 3) {
        return compute_bishop_mask(start, to_move);
    }
    else if (type == 4) {
        return compute_bishop_mask(start, to_move) | compute_rook_mask(start, to_move);
    }
    else if (type == 5) {
        return compute_king_mask(start, to_move);
    }
    return 0;
}

uint64_t board::compute_white_pawn_mask(uint64_t start) {

    uint64_t moves = (((start >> 8) & ~bins[0][6]) // normal step
            | (bins[0][6] & 
                (((start >> 7) & ~FILE_H) // eating
                | ((start >> 9) & ~FILE_A))))
        & ~bins[1][6]; // avoid collision with allies

    // double step
    if (start & (END_RANK[0] >> 8) // if on the first start
            & ~((bins[0][6] | bins[1][6]) << 8) // and if there are no pieces in front
            & ~((bins[0][6] | bins[1][6]) << 16)) { // and if there are no pieces 2 in front
        moves |= (start >> 16);
    }

    return moves;
}

uint64_t board::compute_black_pawn_mask(uint64_t start) {

    uint64_t moves = (((start << 8) & ~bins[1][6]) // normal step
            | (bins[1][6] & 
                (((start << 7) & ~FILE_A) // eating
                | ((start << 9) & ~FILE_H))))
        & ~bins[0][6]; // avoid collision with allies

    // double step
    if (start & (END_RANK[1] << 8) // if on the first start
            & ~((bins[0][6] | bins[1][6]) >> 8) // and if there are no pieces in front
            & ~((bins[0][6] | bins[1][6]) >> 16)) { // and if there are no pieces 2 in front
        moves |= (start << 16);
    }

    return moves;
}

uint64_t board::compute_knight_mask(uint64_t start, bool side) {
    uint64_t moves;
    int mask_move = 63 - __builtin_ctzll(start) - KNIGHT_MASK_CENTER;
    moves = (mask_move < 0) ? (KNIGHT_MASK << -mask_move) : (KNIGHT_MASK >> mask_move); // move mask to desired pos
    const int file = (mask_move + KNIGHT_MASK_CENTER) % 8;
    if (file < 2) { // remove out of bounds
        moves &= ~(FILES_GH);
    }
    else if (file > 5) {
        moves &= ~(FILES_AB);
    }
    moves &= ~bins[side][6]; // remove moves that conflict with your pieces
    return moves;
}

uint64_t board::compute_pawn_mask_check(uint64_t start, bool side) {
    return side
        ? ((start >> 7) & ~FILE_A) | ((start >> 9) & ~FILE_H)  // white pawns
        : ((start << 9) & ~FILE_A) | ((start << 7) & ~FILE_H); // black pawns
}

uint64_t board::compute_rook_mask(uint64_t start, bool side) {
    const int directions[] = {8, -8, 1, -1};
    uint64_t attack_mask = 0;
    for (int direction : directions) {
        uint64_t current = start;
        while (true) {
            // move one square in the current direction
            current = (direction > 0) ? (current << direction) : (current >> -direction);
            if (current == 0) break; // out of bounds
            
            // stop if current square is outside the board
            if ((direction == 1 && (current & FILE_H)) || 
                (direction == -1 && (current & FILE_A)) || 
                (direction == 8 && (current & END_RANK[1])) || 
                (direction == -8 && (current & END_RANK[0]))) {
                break;
            }

            attack_mask |= current;

            // stop if a piece blocks the path
            if (current & bins[side][6]) break;
            else if (current & bins[!side][6]) {
                attack_mask |= current;
                break;
            }
        }
    }
    return attack_mask & ~bins[side][6];
}

uint64_t board::compute_bishop_mask(uint64_t position, bool side) {
    const int directions[] = {9, -9, 7, -7};
    uint64_t attack_mask = 0;
    for (int direction : directions) {
        uint64_t current = position;
        while (true) {
            // move one square in the current direction
            current = (direction > 0) ? (current << direction) : (current >> -direction);
            if (current == 0) break; // out of bounds

            // stop if current square is outside the diagonal bounds
            if ((direction == 9 && (current & (FILE_H | END_RANK[1]))) || 
                (direction == -9 && (current & (FILE_A | END_RANK[0]))) || 
                (direction == 7 && (current & (FILE_A | END_RANK[1]))) || 
                (direction == -7 && (current & (FILE_H | END_RANK[0])))) {
                break;
            }

            attack_mask |= current;

            // stop if a piece blocks the path
            if (current & bins[side][6]) break;
            else if (current & bins[!side][6]) {
                attack_mask |= current;
                break;
            }
        }
    }
    return attack_mask & ~bins[side][6];
}

uint64_t board::compute_king_mask(uint64_t position, bool side) {
    uint64_t mask = 0;

    mask |= (position << 8) & ~END_RANK[1];               // down
    mask |= (position >> 8) & ~END_RANK[0];               // up
    mask |= (position << 1) & ~FILE_H;                    // left
    mask |= (position >> 1) & ~FILE_A;                    // right
    mask |= (position << 9) & ~(FILE_H | END_RANK[1]);    // bottom-left
    mask |= (position >> 9) & ~(FILE_A | END_RANK[0]);    // top-right
    mask |= (position << 7) & ~(FILE_A | END_RANK[1]);    // bottom-right
    mask |= (position >> 7) & ~(FILE_H | END_RANK[0]);    // top-left

    return mask & ~bins[side][6];
}

void board::move(std::pair<uint64_t, uint64_t> movement, char promo) {
    uint64_t starting_point = movement.first;
    uint64_t ending_point = movement.second;

    bool piece_moved = false;

    int side = -1, type = -1;
            
    while (side < 2 && piece_moved == false) {
        side += 1;
        type = -1;
        while (type < 5 && piece_moved == false) {
            type += 1;
            if (bins[side][type] & starting_point) {
                bins[side][type] &= ~starting_point;
                bins[side][6] &= ~starting_point;
                piece_moved = true;
            }
        }
    }

    if (type == 0) {
        uint64_t behind_position = side ? (ending_point << 8) : (ending_point >> 8);
        bool is_diagonal_move = ending_point & (side 
            ? (starting_point >> 7) | (starting_point >> 9)   // White pawns
            : (starting_point << 9) | (starting_point << 7)); // Black pawns
        if (!(ending_point & bins[!side][6])) {
            if (is_diagonal_move) {
                if (bins[!side][0] & behind_position) {
                    bins[!side][0] &= ~behind_position;
                    bins[!side][6] &= ~behind_position;
                }
            }
        }
    }

    bool piece_captured = false;
    if (bins[!side][6] & ending_point) {
        for (int i = 0; i < 6; i++) {
            if (bins[!side][i] & ending_point) {
                bins[!side][i] &= ~ending_point;
                bins[!side][6] &= ~ending_point;                
                piece_captured = true;
            }
        }
    }

    if (promo) {
        switch (promo) {
            case 'r':
                type = 1;
                break;
            case 'k':
                type = 2;
                break;
            case 'b':
                type = 3;
                break;
            case 'q':
                type = 4;
                break;
        }
    }

    bins[side][type] |= ending_point;
    bins[side][6] |= ending_point;
}

void board::move(const std::string& movement) { // movement "e4e5" or "e7e8k"
    std::string start = {movement[0], movement[1]};
    std::string end = {movement[2], movement[3]};


    // Call the other move function with bitboards
    if (movement.length() == 4) {
    
        // if king or rook moves, lose castling rights, if king moves to castling square, move rook too
        if (start == "e1") {
            can_castle_long_white = false;
            can_castle_short_white = false;
            if (end == "g1") {
                move(std::pair<uint64_t, uint64_t> {position_to_bitboard("h1"), position_to_bitboard("f1")});
            } else if (end == "c1") {
                move(std::pair<uint64_t, uint64_t> {position_to_bitboard("a1"), position_to_bitboard("d1")});
            }
        } else if (start == "e8") {
            can_castle_long_black = false;
            can_castle_short_black = false;
            if (end == "g8") {
                move(std::pair<uint64_t, uint64_t> {position_to_bitboard("h8"), position_to_bitboard("f8")});
            } else if (end == "c8") {
                move(std::pair<uint64_t, uint64_t> {position_to_bitboard("a8"), position_to_bitboard("d8")});
            }
        } else if (start == "h1") {
            can_castle_short_white = false;
        } else if (start == "a1") {
            can_castle_long_white = false;
        } else if (start == "h8") {
            can_castle_short_black = false;
        } else if (start == "a8") {
            can_castle_long_black = false;
        }
        return move(std::pair<uint64_t, uint64_t>{position_to_bitboard(start), position_to_bitboard(end)});
    }
    else if (movement.length() == 5) {
        char promo = movement[4];
        return move(std::pair<uint64_t, uint64_t>{position_to_bitboard(start), position_to_bitboard(end)}, promo);
    }
    else std::cerr << "Error: Invalid movement syntax" << std::endl;
    return;
}

// Read moves from history file
void board::read_moves(std::vector<std::string> moves_history) {
    for (size_t i = 0; i < moves_history.size(); i++) {
        move(moves_history[i]);
    }
}

double board::eval2() {
    uint64_t options = 0;
    uint64_t piece = 0;
    double points = 0;
    int side = 0;
    int type = 0;
    int first = 0;

    for (side=0;side<2;side++) {
        for (type=0;type<6;type++) {
            options = bins[side][type];
            while (options) {
                piece = options & -options;
                points += weights[side][type][63 - __builtin_ctzll(piece)];
                options -= piece;
            }
        }
    }

    return points;
}

double board::eval() {

    double white_points = 0;
    double black_points = 0;

    // for each white piece type, call popcountll and calculate each point value
    white_points += __builtin_popcountll(bins[1][0]) * 1;
    white_points += __builtin_popcountll(bins[1][1]) * 5;
    white_points += __builtin_popcountll(bins[1][2]) * 3;
    white_points += __builtin_popcountll(bins[1][3]) * 3;
    white_points += __builtin_popcountll(bins[1][4]) * 9;
    white_points += __builtin_popcountll(bins[1][5]) * 1000;

    black_points += __builtin_popcountll(bins[0][0]) * 1;
    black_points += __builtin_popcountll(bins[0][1]) * 5;
    black_points += __builtin_popcountll(bins[0][2]) * 3;
    black_points += __builtin_popcountll(bins[0][3]) * 3;
    black_points += __builtin_popcountll(bins[0][4]) * 9;
    black_points += __builtin_popcountll(bins[0][5]) * 1000;

    //final return is white - black
    return white_points - black_points;
}

std::string bitboard_to_position(uint64_t bitboard){
    // important that "board" only consists of one 1 and 0s
    // returns index of 1 if everything else is zero
    int index = 0;
    while(bitboard > 1){
        bitboard >>= 1;
        ++index;
    }

    int rank = 8 - (index / 8);

    // compute file
    int file = index% 8 ;
    // Convert file int to letter
    char file_char = 'h' - file;

    std::string position = std::string(1, file_char) + std::to_string(rank);


    return position;
}

