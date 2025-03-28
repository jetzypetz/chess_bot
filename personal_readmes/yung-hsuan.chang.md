## 2024 Nov 25
- Created the repository.

## 2024 Nov 28
- Made a scratch of the project. (It was discarded after the discussion; used Jetz's idea.)

## 2024 Dec 2
- Changed from Makefile to CMake.
- Completed `void board::display()`.

## 2024 Dec 5
- Implemented an initial `move()` function for debugging.
- Completed construction of `is_checked()`.
- Fixed `display()` with new variable settings. 
- Fix GDB under CMake.

## 2024 Dec 9
- Re-implemented `is_checked()` with masks.

## 2024 Dec 12
- Modified the `move()` function for promotion.

## 2024 Dec 16
- Started to study the minimax algorithm.
- Tried the first time to implemented the `minimax()`.

## 2024 Dec 18
- Made `all_lagal_moves()` contain castling.

## 2024 Dec 22
- Made a suggestion for debugging to Jetz about `master()`. (The first loop is already a layer in the sense of minimax, so that it should start to evaluate from the opposite side.)

## 2025 Jan 8
- Modified codes for my part (`is_checked()`, `move()`, `all_legal_moves()` for castling and promotion, and `minimax()`) to the new array framework aprat from the old map framework.

## 2025 Jan 12
- Fixed a box in `all_legal_moves()` (verifying under checks for castling).

## 2025 Jan 21
- Modified `master()` in `tools.cpp` to avoid crashes. (Using a pointer `board* game` instead of just a variable.)