#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <cmath>

const int WINDOW_SIZE = 700;
const int CELL_SIZE = 200;
const int MARGIN = 50;
const float TRANSITION_TIME = 0.5f;

enum class GameState {
    Menu,
    Game,
    GameOver
};

struct WinLine {
    sf::Vector2f start;
    sf::Vector2f end;
    bool exists = false;
};

std::array<std::array<char, 3>, 3> board;
char currentPlayer = 'X';
bool gameOver = false;
std::string winnerText = "";
GameState currentState = GameState::Menu;
WinLine winningLine;
float lineAnimation = 0.f;
sf::Clock animationClock;

void resetBoard() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = ' ';
        }
    }
    currentPlayer = 'X';
    gameOver = false;
    winnerText = "";
    winningLine.exists = false;
    lineAnimation = 0.f;
}

bool checkWinner() {
    // Verificar filas
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            winnerText = std::string("Gana ") + board[i][0] + "!";
            winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * 0 + CELL_SIZE/2, MARGIN + CELL_SIZE * i + CELL_SIZE/2);
            winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE * i + CELL_SIZE/2);
            winningLine.exists = true;
            return true;
        }
    }
    
    // Verificar columnas
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            winnerText = std::string("Gana ") + board[0][i] + "!";
            winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * i + CELL_SIZE/2, MARGIN + CELL_SIZE * 0 + CELL_SIZE/2);
            winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * i + CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
            winningLine.exists = true;
            return true;
        }
    }

    // Verificar diagonal principal
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        winnerText = std::string("Gana ") + board[0][0] + "!";
        winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE/2, MARGIN + CELL_SIZE/2);
        winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
        winningLine.exists = true;
        return true;
    }
    
    // Verificar diagonal secundaria
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        winnerText = std::string("Gana ") + board[0][2] + "!";
        winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE/2);
        winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
        winningLine.exists = true;
        return true;
    }

    // Verificar empate
    bool full = true;
    for (const auto& row : board) {
        for (char c : row) {
            if (c == ' ') {
                full = false;
                break;
            }
        }
        if (!full) break;
    }

    if (full) {
        winnerText = "Empate!";
        return true;
    }

    return false;
}

void drawGame(sf::RenderWindow& window, const sf::Font& font) {
    // Fondo del tablero con sombra
    sf::RectangleShape boardBg(sf::Vector2f(CELL_SIZE * 3 + 10, CELL_SIZE * 3 + 10));
    boardBg.setPosition(MARGIN - 5, MARGIN - 5);
    boardBg.setFillColor(sf::Color(220, 220, 220));
    window.draw(boardBg);

    sf::RectangleShape boardMain(sf::Vector2f(CELL_SIZE * 3, CELL_SIZE * 3));
    boardMain.setPosition(MARGIN, MARGIN);
    boardMain.setFillColor(sf::Color(245, 245, 245));
    window.draw(boardMain);

    // Dibujar líneas de la cuadrícula con estilo redondeado
    sf::RectangleShape line;
    line.setFillColor(sf::Color(100, 100, 120));
    
    for (int i = 1; i < 3; i++) {
        // Líneas verticales
        line.setSize(sf::Vector2f(6, CELL_SIZE * 3));
        line.setPosition(MARGIN + i * CELL_SIZE - 3, MARGIN);
        window.draw(line);

        // Líneas horizontales
        line.setSize(sf::Vector2f(CELL_SIZE * 3, 6));
        line.setPosition(MARGIN, MARGIN + i * CELL_SIZE - 3);
        window.draw(line);
    }

    // Dibujar X y O con sombra y efecto hover
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != ' ') {
                // Sombra
                sf::Text shadow;
                shadow.setFont(font);
                shadow.setString(std::string(1, board[i][j]));
                shadow.setCharacterSize(120);
                shadow.setFillColor(sf::Color(0, 0, 0, 40));
                shadow.setStyle(sf::Text::Bold);
                
                sf::FloatRect bounds = shadow.getLocalBounds();
                float posX = MARGIN + j * CELL_SIZE + (CELL_SIZE - bounds.width) / 2.f - bounds.left;
                float posY = MARGIN + i * CELL_SIZE + (CELL_SIZE - bounds.height) / 2.f - bounds.top - 15.f;
                
                shadow.setPosition(posX + 3, posY + 3);
                window.draw(shadow);

                // Texto principal
                sf::Text text;
                text.setFont(font);
                text.setString(std::string(1, board[i][j]));
                text.setCharacterSize(120);
                text.setStyle(sf::Text::Bold);
                
                if (board[i][j] == 'X') {
                    text.setFillColor(sf::Color(255, 80, 80));
                } else {
                    text.setFillColor(sf::Color(80, 150, 255));
                }
                
                text.setPosition(posX, posY);
                window.draw(text);
            }
        }
    }

    // Dibujar línea ganadora con animación
    if (winningLine.exists && lineAnimation < 1.f) {
        lineAnimation += 0.02f;
        if (lineAnimation > 1.f) lineAnimation = 1.f;
    }

    if (winningLine.exists && lineAnimation > 0.f) {
        sf::Vector2f currentEnd = winningLine.start + (winningLine.end - winningLine.start) * lineAnimation;
        
        // Línea con grosor
        sf::Vector2f direction = winningLine.end - winningLine.start;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y) * lineAnimation;
        float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;
        
        sf::RectangleShape winLine(sf::Vector2f(length, 12));
        winLine.setPosition(winningLine.start);
        winLine.setRotation(angle);
        winLine.setFillColor(sf::Color(255, 215, 0));
        winLine.setOutlineThickness(2);
        winLine.setOutlineColor(sf::Color(200, 170, 0));
        window.draw(winLine);
    }

    // Indicador de turno
    if (!gameOver) {
        sf::Text turnText;
        turnText.setFont(font);
        turnText.setString(std::string("Turno: ") + currentPlayer);
        turnText.setCharacterSize(40);
        turnText.setStyle(sf::Text::Bold);
        turnText.setFillColor(currentPlayer == 'X' ? sf::Color(255, 80, 80) : sf::Color(80, 150, 255));
        
        sf::FloatRect bounds = turnText.getLocalBounds();
        turnText.setPosition((WINDOW_SIZE - bounds.width) / 2.f, 10.f);
        window.draw(turnText);
    }

    // Mensaje de Game Over
    if (gameOver) {
        // Fondo semi-transparente
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, 150));
        overlay.setPosition(0, WINDOW_SIZE - 150);
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text msg;
        msg.setFont(font);
        msg.setString(winnerText);
        msg.setCharacterSize(55);
        msg.setStyle(sf::Text::Bold);
        msg.setFillColor(sf::Color(255, 215, 0));
        
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setPosition((WINDOW_SIZE - bounds.width) / 2.f, WINDOW_SIZE - 120.f);
        window.draw(msg);

        sf::Text instruction;
        instruction.setFont(font);
        instruction.setString("Clic para volver al menu");
        instruction.setCharacterSize(25);
        instruction.setFillColor(sf::Color(200, 200, 200));
        
        bounds = instruction.getLocalBounds();
        instruction.setPosition((WINDOW_SIZE - bounds.width) / 2.f, WINDOW_SIZE - 50.f);
        window.draw(instruction);
    }
}

void drawMenu(sf::RenderWindow& window, const sf::Font& font) {
    // Fondo degradado (simulado)
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 1));
        line.setPosition(0, i);
        float ratio = static_cast<float>(i) / WINDOW_SIZE;
        sf::Uint8 color = 230 - static_cast<sf::Uint8>(ratio * 50);
        line.setFillColor(sf::Color(color, color, color + 10));
        window.draw(line);
    }

    // Título con sombra
    sf::Text titleShadow;
    titleShadow.setFont(font);
    titleShadow.setString("JUEGO DEL GATO");
    titleShadow.setCharacterSize(75);
    titleShadow.setStyle(sf::Text::Bold);
    titleShadow.setFillColor(sf::Color(0, 0, 0, 60));
    sf::FloatRect titleBounds = titleShadow.getLocalBounds();
    titleShadow.setPosition((WINDOW_SIZE - titleBounds.width) / 2.f + 4, 84.f);
    window.draw(titleShadow);

    sf::Text title;
    title.setFont(font);
    title.setString("JUEGO DEL GATO");
    title.setCharacterSize(75);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(60, 60, 80));
    title.setPosition((WINDOW_SIZE - titleBounds.width) / 2.f, 80.f);
    window.draw(title);

    // Símbolos decorativos
    sf::Text xDecor;
    xDecor.setFont(font);
    xDecor.setString("X");
    xDecor.setCharacterSize(100);
    xDecor.setStyle(sf::Text::Bold);
    xDecor.setFillColor(sf::Color(255, 80, 80, 150));
    xDecor.setPosition(80.f, 200.f);
    xDecor.setRotation(-15.f);
    window.draw(xDecor);

    sf::Text oDecor;
    oDecor.setFont(font);
    oDecor.setString("O");
    oDecor.setCharacterSize(100);
    oDecor.setStyle(sf::Text::Bold);
    oDecor.setFillColor(sf::Color(80, 150, 255, 150));
    oDecor.setPosition(520.f, 200.f);
    oDecor.setRotation(15.f);
    window.draw(oDecor);

    // Botón JUGAR con hover
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    bool isHovering = (mousePos.x >= 200 && mousePos.x <= 500 && 
                       mousePos.y >= 320 && mousePos.y <= 420);

    // Sombra del botón
    sf::RectangleShape buttonShadow(sf::Vector2f(300, 100));
    buttonShadow.setFillColor(sf::Color(0, 0, 0, 60));
    buttonShadow.setPosition(205.f, 325.f);
    window.draw(buttonShadow);

    sf::RectangleShape playButton(sf::Vector2f(300, 100));
    playButton.setFillColor(isHovering ? sf::Color(70, 220, 70) : sf::Color(50, 200, 50));
    playButton.setPosition(200.f, 320.f);
    playButton.setOutlineThickness(4);
    playButton.setOutlineColor(sf::Color(30, 150, 30));
    window.draw(playButton);

    sf::Text playText;
    playText.setFont(font);
    playText.setString("JUGAR");
    playText.setCharacterSize(55);
    playText.setStyle(sf::Text::Bold);
    playText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = playText.getLocalBounds();
    playText.setPosition(
        200.f + (300.f - textBounds.width) / 2.f - textBounds.left,
        320.f + (100.f - textBounds.height) / 2.f - textBounds.top
    );
    window.draw(playText);

    // Instrucciones
    sf::Text instructions;
    instructions.setFont(font);
    instructions.setString("Clic derecho para volver al menu");
    instructions.setCharacterSize(22);
    instructions.setFillColor(sf::Color(100, 100, 100));
    sf::FloatRect instBounds = instructions.getLocalBounds();
    instructions.setPosition((WINDOW_SIZE - instBounds.width) / 2.f, 550.f);
    window.draw(instructions);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Juego del Gato");
    window.setFramerateLimit(60);
    
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/calibri.ttf") &&
            !font.loadFromFile("C:/Windows/Fonts/verdana.ttf") &&
            !font.loadFromFile("arial.ttf")) {
            printf("No se pudo cargar ninguna fuente.\n");
            return -1;
        }
    }

    resetBoard();

    while (window.isOpen()) {
        sf::Event event;
        
        while (window.pollEvent(event)) { 
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (currentState == GameState::Game && !gameOver) {
                        int x = (event.mouseButton.x - MARGIN) / CELL_SIZE;
                        int y = (event.mouseButton.y - MARGIN) / CELL_SIZE;
                        
                        if (x >= 0 && x < 3 && y >= 0 && y < 3 && board[y][x] == ' ') {
                            board[y][x] = currentPlayer;
                            if (checkWinner()) {
                                gameOver = true;
                                currentState = GameState::GameOver;
                                animationClock.restart();
                            } else {
                                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                            }
                        }
                    } else if (currentState == GameState::Menu) {
                        if (event.mouseButton.x >= 200 && event.mouseButton.x <= 500 && 
                            event.mouseButton.y >= 320 && event.mouseButton.y <= 420) {
                            resetBoard();
                            currentState = GameState::Game;
                        }
                    } else if (currentState == GameState::GameOver) {
                        currentState = GameState::Menu;
                    }
                }
                
                if (event.mouseButton.button == sf::Mouse::Right) {
                    if (currentState == GameState::Game || currentState == GameState::GameOver) {
                        currentState = GameState::Menu;
                    }
                }
            }
        }

        window.clear(sf::Color::White);

        if (currentState == GameState::Menu) {
            drawMenu(window, font);
        } else if (currentState == GameState::Game || currentState == GameState::GameOver) {
            drawGame(window, font);
        }
        
        window.display();
    }
    
    return 0;
}