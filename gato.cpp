#include <SFML/Graphics.hpp>
#include <array>
#include <string>

// No se necesita <optional> en SFML 2.6

const int CELL_SIZE = 200;

std::array<std::array<char, 3>, 3> board = {0};
char currentPlayer = 'X';
bool gameOver = false;
std::string winnerText = "";

void resetBoard() {
    board = {0};
    currentPlayer = 'X';
    gameOver = false;
    winnerText = "";
}

bool checkWinner() {
    for (int i = 0; i < 3; i++) {
        // Filas
        if (board[i][0] && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            winnerText = std::string("Gana ") + board[i][0];
            return true;
        }
        // Columnas
        if (board[0][i] && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            winnerText = std::string("Gana ") + board[0][i];
            return true;
        }
    }

    // Diagonales
    if (board[0][0] && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        winnerText = std::string("Gana ") + board[0][0];
        return true;
    }
    if (board[0][2] && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        winnerText = std::string("Gana ") + board[0][2];
        return true;
    }

    // Empate
    bool full = true;
    for (auto& row : board)
        for (auto c : row)
            if (c == 0)
                full = false;

    if (full) {
        winnerText = "Empate";
        return true;
    }

    return false;
}

int main() {
    // Se usa sf::Vector2u en lugar de {600, 600}
    sf::RenderWindow window(sf::VideoMode(600, 600), "Juego del Gato (SFML 2.6)");

    
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        printf("No se pudo cargar arial.ttf\n");
        return -1;
    }

    // Objeto sf::Event para usar en el bucle de eventos
    sf::Event event; 

    while (window.isOpen()) {
        // --- BUENA PRÁCTICA: BUCLE DE EVENTOS SFML 2.x ---
        while (window.pollEvent(event)) { 
            // Manejar evento de cerrar ventana
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Manejar click del ratón
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left && !gameOver) {
                    // Acceder a la posición con event.mouseButton.x/y
                    int x = event.mouseButton.x / CELL_SIZE;
                    int y = event.mouseButton.y / CELL_SIZE;
                    
                    if (x < 3 && y < 3 && board[y][x] == 0) {
                        board[y][x] = currentPlayer;
                        if (checkWinner()) {
                            gameOver = true;
                        } else {
                            currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                        }
                    }
                }
                
                // Click derecho para reiniciar
                if (event.mouseButton.button == sf::Mouse::Right) {
                    resetBoard();
                }
            }
        }
        // ----------------------------------------------------

        window.clear(sf::Color(240, 240, 240));

        // --- Dibujar la rejilla ---
        sf::RectangleShape line;
        line.setFillColor(sf::Color::Black);

        for (int i = 1; i < 3; i++) {
            // Línea Vertical
            line.setSize(sf::Vector2f(5, 600)); // Usar sf::Vector2f
            line.setPosition(static_cast<float>(i * CELL_SIZE), 0);
            window.draw(line);

            // Línea Horizontal
            line.setSize(sf::Vector2f(600, 5)); // Usar sf::Vector2f
            line.setPosition(0, static_cast<float>(i * CELL_SIZE));
            window.draw(line);
        }

        // --- Dibujar X y O ---
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] != 0) {
                    // Usar el constructor sf::Text(const sf::String& string, const sf::Font& font, unsigned int characterSize)
                    sf::Text text(sf::String(board[i][j]), font, 100); 
                    text.setFillColor(board[i][j] == 'X' ? sf::Color::Red : sf::Color::Blue);
                    
                    // En SFML 2.x, el Bounds se obtiene con getGlobalBounds() y el ancho/alto con width/height
                    sf::FloatRect bounds = text.getLocalBounds(); 
                    
                    // Cálculo de la posición similar, aunque la compensación del texto puede variar ligeramente
                    text.setPosition({
                        j * CELL_SIZE + (CELL_SIZE - bounds.width) / 2.f - bounds.left, // bounds.left para centrado más exacto
                        i * CELL_SIZE + (CELL_SIZE - bounds.height) / 2.f - bounds.top - 20.f // bounds.top y el ajuste de -20.f
                    });
                    
                    window.draw(text);
                }
            }
        }

        // --- Mensaje de Fin de Juego ---
        if (gameOver) {
            sf::Text msg(winnerText, font, 50);
            msg.setFillColor(sf::Color::Black);
            
            sf::FloatRect bounds = msg.getLocalBounds();
            
            msg.setPosition({
                (600 - bounds.width) / 2.f - bounds.left, 
                600.f - 80.f - bounds.top
            });
            window.draw(msg);
        }

        window.display();
    }
}