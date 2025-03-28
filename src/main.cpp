#include "h_files/tools.h"
#include "h_files/board.h"
#include <iostream>
#include <string>

// main
int main(const int argc, char* argv[]) {
    // use to make next move depending on history
    master(argv[2], argv[4], 4, 2); 
    // uncomment for interactive gameplay
    // play_me(argv[2], argv[4]);
    return 0;
}
