# contributions

## first week
Here is our initial To-do List

* represent chessboard
    * a structure board 
    * member variables:
        * chess pieces: (white, black) X (pawn, bishop, knight, rook, queen, king)
            * std::int64_t 
        * double eval(board* pos) 
            * probably the thorniest
            * input: current position
            * output: signed bounded number representing evaluation
        * std::vector<board> new_pos_list() 
            * input: no
            * output: a vector as the list of all new valid positions that can be reached.
            * iterate through all possible piece moves, rejecting each pos if not is_legal()
        * bool is_legal()
            * check if:
                * pieces end up overlapping
                * checks or pins mean it is not valid
                * pieces blocking pieces from moving
        * void display()
            * input: no
            * output: no
            * shows the chessboard
* functions:
    * board old_pos_generate(history)
        * input: the history as given by commandline prompt
        * output: a board with the current position
    * represent_move(old_pos, new_pos)
        * input: the position we start with and the position we reach
        * output: what move was made to get there and if its a check or something
* piece details
    * pawn
        * worth 1 point
        * (initial) can go one or two steps forward in the beginning
        * (normal) can go one step forward
        * (capture) can go one step forward with an extra horizontal step
        * (en passant) can go one step forward with an extra horizontal step if an enemy pawn on the same row and an adjacent column that has just made an initial two-square advance.
        * (promotion) can become a knight, rook, bishop, or queen
    * knight
        * worth 3 points
        * (L shape) can go two squares vertically and one square horizontally, or two squares horizontally and one square vertically
    * rook
        * worth 5 points
        * can go horizontally and vertically
        * (castle) can go two steps towards the king if neither the king nor the rook has previously moved, and the king is not under check before and after castling
        * (long castle) can go three steps towards the king if neither the king nor the rook has previously moved, and the king is not under check before and after castling
    * bishop
        * worth 3 points
        * can go diagonally
    * queen
        * worth 9 points
        * can go like bishop or rook
    * king
        * can only make one step farther from itself
        * (castle and long castle) can go beside and over the rook if neither the king nor the rook has previously moved, and the king is not under check before and after castling



Here I presented an abstract idea to the group to introduce uniqueness and efficient computation to our algorithm. The idea begins with using binary numbers to represent pieces on the board. we have a binary for each type of piece that represents where the pieces are on the 64 squares. this way computations like finding a piece can be done extremely efficiently with bitwise operations. In fact, if we want our algorithm to dig deep into the branches of possible future moves, we need to compute as many boards as possible in 5 seconds, and a more efficient algorithm will find more moves and see further into the future. The key to branching efficiently in this system is not representing 'moves' but 'new boards', and this distinction offers in my mind a faster alternative, as we can go directly between bitmaps rather than making new boards from encoded moves. We will see whether this was all a waste of time! :P


## second week
I am working on a function board::all_legal_moves(bool side) which returns all the legal moves from a position. this will be at the core of the branching ML algorithm. using bitmasks will increase the efficiency drastically especially for the fact that our bitmasks and binary encodings are all the same length as a machine word, so computations are basically done in a couple clock cycles.

## third week
here are some approximate speed tests for some functions we have created so far:
make board: 40 µs
compute knight mask: 1 µs
all legal moves: 125 µs

I have been following the team closely and giving orders, working with eiken to proof read his work on board::is_checked(side) that tells if under check, as for the rest of the team, they have been working on reading the history file.
I have been working on bitmasks for knights and pawns, and testing different approaches to see which is faster. comparing two functions to generate knight moves, i found one took 500 nanoseconds and the other took 100 nanoseconds (100 billionths of a second) and will keep the latter which is particularly fast. these are important for streamlining the all_legal_moves function. as of now all_legal_moves iterates through all types of pieces in the same way, meaning it is easier to read and extremely fast.
here is the function's current state:

std::vector<board> board::all_legal_moves(bool side) {
    std::vector<board> boards;
    // new board generator
    board n_board = *this;
    // useful aliases
    int64_t& allies = side ? n_board.white : n_board.black;
    int64_t& enemies = side ? n_board.black :n_board.white;
    // for iteration on pieces to move and where to send them
    std::vector<std::string> pieces_to_move; // e.g. ["wp","wN","wb","wr","wq","wk"]
    if (side) {
    // only knights and white pawns implemented for now
        pieces_to_move = {"wp", "wN"};
    } else {
        pieces_to_move = {"bN"};
    }
        // {"wp","wN","wb","wr","wq","wk"}
        // {"bp","bN","bb","br","bq","bk"}
    int64_t pieces;
    int nb_pieces;
    int64_t start; // piece starting point
    int64_t options;
    int nb_options;
    int64_t end; // piece destination

    for (auto piece_type : pieces_to_move) {
        pieces = n_board.bins[piece_type]; // set of moving pieces, e.g. all knights
        nb_pieces = __builtin_popcountll(pieces);
        for (int i=0;i<nb_pieces;i++) { // for each moving piece, e.g. each knight
            start = pieces & -pieces;
            pieces -= start;
            options = n_board.compute_mask(piece_type, start, side); // set of options for piece
            nb_options = __builtin_popcountll(options);
            for (int j=0;j<nb_options;j++) { // for each piece, for each end
                end = options & -options;
                options -= end;

                // move moving piece
                allies ^= start ^ end;
                n_board.bins[piece_type] ^= start ^ end;

                // if it eats a piece, remove dying piece
                if (enemies & end) {
                    enemies &= ~end;
                    // sloppy code: can be improved with different storage of bins
                    for (auto [key, value] : n_board.bins) { // find and remove dying piece
                        if (enemies & value & end) {
                            n_board.bins[key] &= ~end;
                            break;
                        }
                    }
                }

                // now n_board has been updated,
                // if we are not in check, add n_board to all_legal_moves
                if (~n_board.is_checked(side)) {
                    boards.push_back(n_board);
                    // sloppy code: better to undo move than remake board
                    n_board = *this;
                }
            }
        }
    }



    return boards;
}

I have gone down a rabbit hole of making the mask functions faster. here i compare the deacreasing times for 10000 iterations of computing a pawn mask:
eikens: 69900 nanoseconds
jetzs: 64786 nanoseconds
eikens local: 24011 nanoseconds
eikens local: 26677 nanoseconds
eikens local: 22432 nanoseconds
pawn_mask: 18717 nanoseconds

I have learned that reducing assignments and focusing on composing bitmaps makes code ridiculously fast.
I of course have to remember that I should not focus on this now, as the limiting factor now will be how well I can use the information from the branching we do into future positions to choose the best position.

## fourth week

I have made the master function that links everything together so that we can run the program with one line in the main.cpp file. also a benchmark function to check speeds of functions.

## week 5 i think

i have been working on changing to array implementation and have finally reached a runnable version, which yields a slight improvement over our std::map implementation:
`array implementation all_legal_moves()` call: 6675 ns
`std::map implementation all_legal_moves()` call: 251700 ns

## week 6? ive lost count

array implementation has been finished and is working! it runs very fast, allowing a depth of around 4 moves. i have fixed bugs with castling, which had not been implemented properly. Also I have had sessions with eiken and emilie and peter to work on different things with each one. eiken: we worked on pawn promotion, emilie: reading moves, peter: working on openings, which are not implemented yet.
I have also created an interactive gameplay mode which works perfectly.
Our bot now beats the random bot almost always, if its not a draw, this is because we only have a low depth of forsight and we do not choose good moves, because we evaluate each move at the start equally, as none capture pieces. I am studying how to implement weighted evaluation.

## week 7

added a timeout for the master function.

