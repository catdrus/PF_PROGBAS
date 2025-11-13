#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <cmath>

const int WINDOW_SIZE = 600;
const int CELL_SIZE = 200;
const float TRANSITION_TIME = 0.5f;

enum class GameState {
    Menu,
    Game,
    TransitionToGame,
    TransitionToMenu,
    GameOver
};

std::array<std::array<char, 3>, 3> board;
char currentPlayer = 'X';
bool gameOver = false;
std::string winnerText = "";
GameState currentState = GameState::Menu;
float transitionTimer = 0.f;
sf::Clock gameClock;
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
}

bool checkWinner() {
    // Verificar filas
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            winnerText = std::string("Gana ") + board[i][0];
            return true;
        }
    }
    
    // Verificar columnas
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            winnerText = std::string("Gana ") + board[0][i];
            return true;
        }
    }

    // Verificar diagonal principal
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        winnerText = std::string("Gana ") + board[0][0];
        return true;
    }
    
    // Verificar diagonal secundaria
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        winnerText = std::string("Gana ") + board[0][2];
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
        winnerText = "Empate";
        return true;
    }

    return false;
}

void drawGame(sf::RenderWindow& window, const sf::Font& font) {
    // Dibujar líneas de la cuadrícula
    sf::RectangleShape line(sf::Vector2f(5, WINDOW_SIZE));
    line.setFillColor(sf::Color::Black);
    
    for (int i = 1; i < 3; i++) {
        line.setSize(sf::Vector2f(5, WINDOW_SIZE));
        line.setPosition(static_cast<float>(i * CELL_SIZE), 0);
        window.draw(line);

        line.setSize(sf::Vector2f(WINDOW_SIZE, 5));
        line.setPosition(0, static_cast<float>(i * CELL_SIZE));
        window.draw(line);
    }

    // Dibujar X y O
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != ' ') {
                sf::Text text;
                text.setFont(font);
                text.setString(std::string(1, board[i][j]));
                text.setCharacterSize(100);
                text.setFillColor(board[i][j] == 'X' ? sf::Color::Red : sf::Color::Blue);
                
                sf::FloatRect bounds = text.getLocalBounds();
                float posX = j * CELL_SIZE + (CELL_SIZE - bounds.width) / 2.f - bounds.left;
                float posY = i * CELL_SIZE + (CELL_SIZE - bounds.height) / 2.f - bounds.top - 20.f;
                
                text.setPosition(posX, posY);
                window.draw(text);
            }
        }
    }

    // Mensaje de Game Over
    if (gameOver) {
        sf::Text msg;
        msg.setFont(font);
        msg.setString(winnerText + " - Clic para Menu");
        msg.setCharacterSize(35);
        msg.setFillColor(sf::Color::Black);
        
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setPosition((WINDOW_SIZE - bounds.width) / 2.f, WINDOW_SIZE - 60.f);
        window.draw(msg);
    }
}

void drawMenu(sf::RenderWindow& window, const sf::Font& font) {
    // Título
    sf::Text title;
    title.setFont(font);
    title.setString("Juego del Gato");
    title.setCharacterSize(80);
    title.setFillColor(sf::Color(100, 100, 100));
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition((WINDOW_SIZE - titleBounds.width) / 2.f, 100.f);
    window.draw(title);

    // Botón JUGAR
    sf::RectangleShape playButton(sf::Vector2f(300, 100));
    playButton.setFillColor(sf::Color(50, 200, 50));
    playButton.setPosition(150.f, 250.f);
    window.draw(playButton);

    // Texto del botón
    sf::Text playText;
    playText.setFont(font);
    playText.setString("JUGAR");
    playText.setCharacterSize(50);
    playText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = playText.getLocalBounds();
    playText.setPosition(
        150.f + (300.f - textBounds.width) / 2.f,
        250.f + (100.f - textBounds.height) / 2.f - textBounds.top
    );
    window.draw(playText);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Juego del Gato (SFML)");
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

    resetBoard(); // Inicializar el tablero al inicio

    while (window.isOpen()) {
        sf::Event event;
        
        while (window.pollEvent(event)) { 
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (currentState == GameState::Game && !gameOver) {
                        int x = event.mouseButton.x / CELL_SIZE;
                        int y = event.mouseButton.y / CELL_SIZE;
                        
                        if (x >= 0 && x < 3 && y >= 0 && y < 3 && board[y][x] == ' ') {
                            board[y][x] = currentPlayer;
                            if (checkWinner()) {
                                gameOver = true;
                                currentState = GameState::GameOver;
                            } else {
                                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                            }
                        }
                    } else if (currentState == GameState::Menu) {
                        // Detectar clic en botón JUGAR
                        if (event.mouseButton.x >= 150 && event.mouseButton.x <= 450 && 
                            event.mouseButton.y >= 250 && event.mouseButton.y <= 350) {
                            resetBoard();
                            currentState = GameState::Game;
                        }
                    } else if (currentState == GameState::GameOver) {
                        // Volver al menú
                        currentState = GameState::Menu;
                    }
                }
                
                // Clic derecho para volver al menú
                if (event.mouseButton.button == sf::Mouse::Right) {
                    if (currentState == GameState::Game || currentState == GameState::GameOver) {
                        currentState = GameState::Menu;
                    }
                }
            }
        }

        // Dibujar
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