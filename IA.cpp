// IA.cpp
#include "IA.hpp"
#include <vector>
#include <limits>
#include <cstdlib>

std::vector<std::pair<int,int>> movimientosDisponibles() {
    std::vector<std::pair<int,int>> moves;
    for (int r=0;r<3;r++)
        for (int c=0;c<3;c++)
            if (board[r][c] == ' ')
                moves.push_back({r, c});
    return moves;
}

// =========================
// IA Fácil
// =========================
void cpuEasy() {
    auto movs = movimientosDisponibles();
    if (movs.empty()) return;

    int idx = rand() % movs.size();
    board[movs[idx].first][movs[idx].second] = 'O';
}

// =========================
// IA Media (heurística)
// =========================
bool intentaGanarOBloquear(char simbolo) {
    for (auto [r, c] : movimientosDisponibles()) {
        board[r][c] = simbolo;
        // comprobar si ganaría
        bool gana = false;

        // filas
        for (int i=0;i<3;i++)
            if (board[i][0]==simbolo && board[i][1]==simbolo && board[i][2]==simbolo)
                gana = true;

        // columnas
        for (int i=0;i<3;i++)
            if (board[0][i]==simbolo && board[1][i]==simbolo && board[2][i]==simbolo)
                gana = true;

        // diagonales
        if (board[0][0]==simbolo && board[1][1]==simbolo && board[2][2]==simbolo) gana = true;
        if (board[0][2]==simbolo && board[1][1]==simbolo && board[2][0]==simbolo) gana = true;

          // --------------------------------------
        // CASO 1: IA intenta ganar (simbolo == 'O')
        // --------------------------------------
        if (gana && simbolo == 'O') {
            return true;  // deja la O colocada
        }

        // --------------------------------------
        // CASO 2: IA intenta bloquear (simbolo == 'X')
        // --------------------------------------
        if (gana && simbolo == 'X') {
            board[r][c] = 'O';  // coloca la O para bloquear
            return true;
        }

        board[r][c] = ' '; // revertir
    }
    return false;
}

void cpuMedium() {
    // 1. ganar si puede
    if (intentaGanarOBloquear('O')) return;

    // 2. bloquear si el jugador va a ganar
    if (intentaGanarOBloquear('X')) return;

    // 3. tomar centro
    if (board[1][1] == ' ') {
        board[1][1] = 'O';
        return;
    }

    // 4. esquinas
    std::vector<std::pair<int,int>> esquinas = {{0,0},{0,2},{2,0},{2,2}};
    for (auto [r,c] : esquinas) {
        if (board[r][c] == ' ') {
            board[r][c] = 'O';
            return;
        }
    }

    // 5. movimiento aleatorio
    cpuEasy();
}

// =========================
// IA Difícil (Minimax)
// =========================

int evaluar() {
    // X gana = -10, O gana = 10
    for (int i=0;i<3;i++) {
        if (board[i][0]==board[i][1] && board[i][1]==board[i][2] && board[i][0] != ' ') {
            return (board[i][0] == 'O') ? 10 : -10;
        }
        if (board[0][i]==board[1][i] && board[1][i]==board[2][i] && board[0][i] != ' ') {
            return (board[0][i] == 'O') ? 10 : -10;
        }
    }

    if (board[0][0]==board[1][1] && board[1][1]==board[2][2] && board[0][0] != ' ')
        return (board[0][0]=='O') ? 10 : -10;

    if (board[0][2]==board[1][1] && board[1][1]==board[2][0] && board[0][2] != ' ')
        return (board[0][2]=='O') ? 10 : -10;

    return 0;
}

bool tableroLleno() {
    for (auto &row : board)
        for (char c : row)
            if (c == ' ') return false;
    return true;
}

int minimax(bool isMax) {
    int score = evaluar();

    if (score == 10 || score == -10) return score;
    if (tableroLleno()) return 0;

    if (isMax) {
        int best = -1000;
        for (auto [r,c] : movimientosDisponibles()) {
            board[r][c] = 'O';
            best = std::max(best, minimax(false));
            board[r][c] = ' ';
        }
        return best;
    } else {
        int best = 1000;
        for (auto [r,c] : movimientosDisponibles()) {
            board[r][c] = 'X';
            best = std::min(best, minimax(true));
            board[r][c] = ' ';
        }
        return best;
    }
}

void cpuHard() {
    int bestVal = -1000;
    int bestR = -1, bestC = -1;

    for (auto [r,c] : movimientosDisponibles()) {
        board[r][c] = 'O';
        int moveVal = minimax(false);
        board[r][c] = ' ';

        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestR = r;
            bestC = c;
        }
    }

    if (bestR != -1)
        board[bestR][bestC] = 'O';
}

// =========================
// Selector de dificultad
// =========================
void cpuMakeMove(Difficulty diff) {
    switch (diff) {
        case Difficulty::Easy:
            cpuEasy();
            break;
        case Difficulty::Medium:
            cpuMedium();
            break;
        case Difficulty::Hard:
            cpuHard();
            break;
    }
}
