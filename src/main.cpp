#include "h_files/tools.h"
#include "h_files/board.h"
#include <iostream>
#include <string>

// main
int main(const int argc, char* argv[]) {
    if (argc == 1) {
        play_me("../io_files/move.csv", "../io_files/history.csv"); // check tools.cpp to see this code
    } else if (argc == 5) {
        play_me(argv[2], argv[4]); // check tools.cpp to see this code
    } else {
        std::cout << "correct use: ./bot.exe" << std::endl;
        return 1;
    }

    // uncomment to make next move depending on a history file (io_files/history.csv)
    // master(argv[2], argv[4], 4, 2); 
    return 0;
}
