cmake_minimum_required(VERSION 3.15...3.31)
set(CMAKE_CXX_STANDARD 20)

project(bot.exe
  VERSION 1.0
  LANGUAGES CXX)

include_directories(../src)

add_library(board ../src/board.cpp ../src/tools.cpp)

add_executable(bot.exe ../src/main.cpp)

target_link_libraries(bot.exe PRIVATE board)
