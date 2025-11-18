// IA.hpp
#pragma once
#include <array>

// Tablero 3x3
extern std::array<std::array<char, 3>, 3> board;

// Dificultad
enum class Difficulty {
    Easy,
    Medium,
    Hard
};

void cpuMakeMove(Difficulty diff);
