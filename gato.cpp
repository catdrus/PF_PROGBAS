#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <array>
#include <string>
#include <cmath>
#include <cstdint>
#include <ctime>
#include "IA.hpp"

const int WINDOW_SIZE = 700;
const int CELL_SIZE = 200;
const int MARGIN = 50;
const float TRANSITION_TIME = 0.5f;

struct GameState {
    static const int Menu = 0;
    static const int Game = 1;
    static const int GameOver = 2;
    static const int Settings = 3;
    static const int ModeSelect = 4;
    static const int DifficultySelect = 5;
};

struct WinLine {
    sf::Vector2f start;
    sf::Vector2f end;
    bool exists = false;
};

struct AudioManager {
    sf::SoundBuffer clickBuffer;
    sf::SoundBuffer moveBuffer;
    sf::SoundBuffer winBuffer;
    sf::SoundBuffer drawBuffer;
    sf::SoundBuffer hoverBuffer;
    
    sf::Sound clickSound;
    sf::Sound moveSound;
    sf::Sound winSound;
    sf::Sound drawSound;
    sf::Sound hoverSound;
    
    sf::Music bgMusic;
    
    float sfxVolume = 70.f;
    float musicVolume = 30.f;
    bool sfxMuted = false;
    bool musicMuted = false;

    AudioManager()
        : clickSound(clickBuffer),
          moveSound(moveBuffer),
          winSound(winBuffer),
          drawSound(drawBuffer),
          hoverSound(hoverBuffer)
    {
    }
    
    bool loadAudio() {
        bool allLoaded = true;
        std::string baseDir = "C:/Users/catdr/Downloads/PF_Progbas/sounds/";
        
        if (!clickBuffer.loadFromFile(baseDir + "click.wav") && !clickBuffer.loadFromFile("sounds/click.wav") && !clickBuffer.loadFromFile("click.wav")) {
            printf("Advertencia: No se pudo cargar click.wav\n"); allLoaded = false;
        } else { clickSound.setBuffer(clickBuffer); }
        
        if (!moveBuffer.loadFromFile(baseDir + "move.wav") && !moveBuffer.loadFromFile("sounds/move.wav") && !moveBuffer.loadFromFile("move.wav")) {
            printf("Advertencia: No se pudo cargar move.wav\n"); allLoaded = false;
        } else { moveSound.setBuffer(moveBuffer); }
        
        if (!winBuffer.loadFromFile(baseDir + "win.wav") && !winBuffer.loadFromFile("sounds/win.wav") && !winBuffer.loadFromFile("win.wav")) {
            printf("Advertencia: No se pudo cargar win.wav\n"); allLoaded = false;
        } else { winSound.setBuffer(winBuffer); }
        
        if (!drawBuffer.loadFromFile(baseDir + "draw.wav") && !drawBuffer.loadFromFile("sounds/draw.wav") && !drawBuffer.loadFromFile("draw.wav")) {
            printf("Advertencia: No se pudo cargar draw.wav\n"); allLoaded = false;
        } else { drawSound.setBuffer(drawBuffer); }
        
        if (!hoverBuffer.loadFromFile(baseDir + "hover.wav") && !hoverBuffer.loadFromFile("sounds/hover.wav") && !hoverBuffer.loadFromFile("hover.wav")) {
            printf("Advertencia: No se pudo cargar hover.wav\n"); allLoaded = false;
        } else { hoverSound.setBuffer(hoverBuffer); }
        
        if (!bgMusic.openFromFile(baseDir + "background.ogg") && !bgMusic.openFromFile("sounds/background.ogg") && !bgMusic.openFromFile("background.ogg")) {
            printf("Advertencia: No se pudo cargar background.ogg\n");
        } else {
            bgMusic.setLooping(true);
        }
        
        updateVolumes();
        return allLoaded;
    }
    
    void updateVolumes() {
        float actualSfxVol = sfxMuted ? 0.f : sfxVolume;
        float actualMusicVol = musicMuted ? 0.f : musicVolume;
        clickSound.setVolume(actualSfxVol);
        moveSound.setVolume(actualSfxVol * 0.85f);
        winSound.setVolume(actualSfxVol * 1.1f);
        drawSound.setVolume(actualSfxVol);
        hoverSound.setVolume(actualSfxVol * 0.57f);
        bgMusic.setVolume(actualMusicVol);
    }
    
    void playClick() { if (clickBuffer.getDuration() > sf::Time::Zero && !sfxMuted) { clickSound.play(); } }
    void playMove() { if (moveBuffer.getDuration() > sf::Time::Zero && !sfxMuted) { moveSound.play(); } }
    void playWin() { if (winBuffer.getDuration() > sf::Time::Zero && !sfxMuted) { winSound.play(); } }
    void playDraw() { if (drawBuffer.getDuration() > sf::Time::Zero && !sfxMuted) { drawSound.play(); } }
    void playHover() { if (hoverBuffer.getDuration() > sf::Time::Zero && hoverSound.getStatus() != sf::Sound::Status::Playing && !sfxMuted) { hoverSound.play(); } }
    void playMusic() { if (bgMusic.getDuration() > sf::Time::Zero && !musicMuted) { bgMusic.play(); } }
    void stopMusic() { bgMusic.stop(); }
    
    void toggleMusicMute() {
        musicMuted = !musicMuted;
        updateVolumes();
        if (musicMuted) { bgMusic.pause(); } else { bgMusic.play(); }
    }
    
    void toggleSfxMute() {
        sfxMuted = !sfxMuted;
        updateVolumes();
    }
};

std::array<std::array<char, 3>, 3> board;
char currentPlayer = 'X';
bool gameOver = false;
std::string winnerText = "";
int currentState = GameState::Menu;
WinLine winningLine;
float lineAnimation = 0.f;
sf::Clock animationClock;
AudioManager audio;
bool wasHovering = false;

// Variables para modo de juego
bool vsIA = false;
Difficulty cpuDifficulty = Difficulty::Medium;
sf::Clock cpuMoveClock;
bool waitingForCPU = false;

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
    waitingForCPU = false;
}

bool checkWinner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            winnerText = std::string("Gana ") + board[i][0] + "!";
            winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * 0 + CELL_SIZE/2, MARGIN + CELL_SIZE * i + CELL_SIZE/2);
            winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE * i + CELL_SIZE/2);
            winningLine.exists = true;
            return true;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            winnerText = std::string("Gana ") + board[0][i] + "!";
            winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * i + CELL_SIZE/2, MARGIN + CELL_SIZE * 0 + CELL_SIZE/2);
            winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * i + CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
            winningLine.exists = true;
            return true;
        }
    }

    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        winnerText = std::string("Gana ") + board[0][0] + "!";
        winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE/2, MARGIN + CELL_SIZE/2);
        winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
        winningLine.exists = true;
        return true;
    }
    
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        winnerText = std::string("Gana ") + board[0][2] + "!";
        winningLine.start = sf::Vector2f(MARGIN + CELL_SIZE * 3 - CELL_SIZE/2, MARGIN + CELL_SIZE/2);
        winningLine.end = sf::Vector2f(MARGIN + CELL_SIZE/2, MARGIN + CELL_SIZE * 3 - CELL_SIZE/2);
        winningLine.exists = true;
        return true;
    }

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
    sf::RectangleShape boardBg(sf::Vector2f(CELL_SIZE * 3 + 10, CELL_SIZE * 3 + 10));
    boardBg.setPosition({(float)(MARGIN - 5), (float)(MARGIN - 5)});
    boardBg.setFillColor(sf::Color(220, 220, 220));
    window.draw(boardBg);

    sf::RectangleShape boardMain(sf::Vector2f(CELL_SIZE * 3, CELL_SIZE * 3));
    boardMain.setPosition({(float)MARGIN, (float)MARGIN});
    boardMain.setFillColor(sf::Color(245, 245, 245));
    window.draw(boardMain);

    sf::RectangleShape line;
    line.setFillColor(sf::Color(100, 100, 120));
    for (int i = 1; i < 3; i++) {
        line.setSize(sf::Vector2f(6, CELL_SIZE * 3));
        line.setPosition({(float)(MARGIN + i * CELL_SIZE - 3), (float)MARGIN});
        window.draw(line);
        line.setSize(sf::Vector2f(CELL_SIZE * 3, 6));
        line.setPosition({(float)MARGIN, (float)(MARGIN + i * CELL_SIZE - 3)});
        window.draw(line);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != ' ') {
                sf::Text shadow(font);
                shadow.setString(std::string(1, board[i][j]));
                shadow.setCharacterSize(120);
                shadow.setFillColor(sf::Color(0, 0, 0, 40));
                shadow.setStyle(sf::Text::Bold);
                
                sf::FloatRect bounds = shadow.getLocalBounds();
                float posX = MARGIN + j * CELL_SIZE + (CELL_SIZE - bounds.size.x) / 2.f - bounds.position.x;
                float posY = MARGIN + i * CELL_SIZE + (CELL_SIZE - bounds.size.y) / 2.f - bounds.position.y - 15.f;
                
                shadow.setPosition({posX + 3, posY + 3});
                window.draw(shadow);

                sf::Text text(font);
                text.setString(std::string(1, board[i][j]));
                text.setCharacterSize(120);
                text.setStyle(sf::Text::Bold);
                
                if (board[i][j] == 'X') {
                    text.setFillColor(sf::Color(255, 80, 80));
                } else {
                    text.setFillColor(sf::Color(80, 150, 255));
                }
                
                text.setPosition({posX, posY});
                window.draw(text);
            }
        }
    }

    if (winningLine.exists && lineAnimation < 1.f) {
        lineAnimation += 0.02f;
        if (lineAnimation > 1.f) lineAnimation = 1.f;
    }
    if (winningLine.exists && lineAnimation > 0.f) {
        sf::Vector2f direction = winningLine.end - winningLine.start;
        float length = std::hypot(direction.x, direction.y) * lineAnimation; 
        float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;
        sf::RectangleShape winLine(sf::Vector2f(length, 12));
        winLine.setPosition(winningLine.start);
        winLine.setRotation(sf::degrees(angle));
        winLine.setFillColor(sf::Color(255, 215, 0));
        winLine.setOutlineThickness(2);
        winLine.setOutlineColor(sf::Color(200, 170, 0));
        window.draw(winLine);
    }

    if (!gameOver) {
        sf::Text turnText(font);
        if (waitingForCPU) {
            turnText.setString("Turno: IA pensando...");
        } else {
            turnText.setString(std::string("Turno: ") + currentPlayer);
        }
        turnText.setCharacterSize(40);
        turnText.setStyle(sf::Text::Bold);
        turnText.setFillColor(currentPlayer == 'X' ? sf::Color(255, 80, 80) : sf::Color(80, 150, 255));
        
        sf::FloatRect bounds = turnText.getLocalBounds();
        turnText.setPosition({(WINDOW_SIZE - bounds.size.x) / 2.f, 10.f});
        window.draw(turnText);
    }

    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, 150));
        overlay.setPosition({0.f, WINDOW_SIZE - 150.f});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text msg(font);
        msg.setString(winnerText);
        msg.setCharacterSize(55);
        msg.setStyle(sf::Text::Bold);
        msg.setFillColor(sf::Color(255, 215, 0));
        
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setPosition({(WINDOW_SIZE - bounds.size.x) / 2.f, WINDOW_SIZE - 120.f});
        window.draw(msg);

        sf::Text instruction(font);
        instruction.setString("Clic para volver al menu");
        instruction.setCharacterSize(25);
        instruction.setFillColor(sf::Color(200, 200, 200));
        
        bounds = instruction.getLocalBounds();
        instruction.setPosition({(WINDOW_SIZE - bounds.size.x) / 2.f, WINDOW_SIZE - 50.f});
        window.draw(instruction);
    }
}

void drawMenu(sf::RenderWindow& window, const sf::Font& font) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 1));
        line.setPosition({0.f, static_cast<float>(i)});
        float ratio = static_cast<float>(i) / WINDOW_SIZE;
        std::uint8_t color = 230 - static_cast<std::uint8_t>(ratio * 50);
        line.setFillColor(sf::Color(color, color, color + 10));
        window.draw(line);
    }

    sf::Text titleShadow(font);
    titleShadow.setString("JUEGO DEL GATO");
    titleShadow.setCharacterSize(75);
    titleShadow.setStyle(sf::Text::Bold);
    titleShadow.setFillColor(sf::Color(0, 0, 0, 60));
    sf::FloatRect titleBounds = titleShadow.getLocalBounds();
    titleShadow.setPosition({(WINDOW_SIZE - titleBounds.size.x) / 2.f + 4, 84.f});
    window.draw(titleShadow);

    sf::Text title(font);
    title.setString("JUEGO DEL GATO");
    title.setCharacterSize(75);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(60, 60, 80));
    title.setPosition({(WINDOW_SIZE - titleBounds.size.x) / 2.f, 80.f});
    window.draw(title);

    sf::Text xDecor(font);
    xDecor.setString("X");
    xDecor.setCharacterSize(100);
    xDecor.setStyle(sf::Text::Bold);
    xDecor.setFillColor(sf::Color(255, 80, 80, 150));
    xDecor.setPosition({80.f, 200.f});
    xDecor.setRotation(sf::degrees(-15.f));
    window.draw(xDecor);

    sf::Text oDecor(font);
    oDecor.setString("O");
    oDecor.setCharacterSize(100);
    oDecor.setStyle(sf::Text::Bold);
    oDecor.setFillColor(sf::Color(80, 150, 255, 150));
    oDecor.setPosition({520.f, 200.f});
    oDecor.setRotation(sf::degrees(15.f));
    window.draw(oDecor);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    
    bool isHoveringPlay = (mousePos.x >= 200 && mousePos.x <= 500 && 
                           mousePos.y >= 280 && mousePos.y <= 360);
    sf::RectangleShape buttonShadow(sf::Vector2f(300, 80));
    buttonShadow.setFillColor(sf::Color(0, 0, 0, 60));
    buttonShadow.setPosition({205.f, 285.f});
    window.draw(buttonShadow);
    sf::RectangleShape playButton(sf::Vector2f(300, 80));
    playButton.setFillColor(isHoveringPlay ? sf::Color(70, 220, 70) : sf::Color(50, 200, 50));
    playButton.setPosition({200.f, 280.f});
    playButton.setOutlineThickness(4);
    playButton.setOutlineColor(sf::Color(30, 150, 30));
    window.draw(playButton);

    sf::Text playText(font);
    playText.setString("JUGAR");
    playText.setCharacterSize(50);
    playText.setStyle(sf::Text::Bold);
    playText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = playText.getLocalBounds();
    playText.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        280.f + (80.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(playText);

    bool isHoveringSettings = (mousePos.x >= 200 && mousePos.x <= 500 && 
                               mousePos.y >= 380 && mousePos.y <= 460);
    sf::RectangleShape settingsShadow(sf::Vector2f(300, 80));
    settingsShadow.setFillColor(sf::Color(0, 0, 0, 60));
    settingsShadow.setPosition({205.f, 385.f});
    window.draw(settingsShadow);
    sf::RectangleShape settingsButton(sf::Vector2f(300, 80));
    settingsButton.setFillColor(isHoveringSettings ? sf::Color(100, 150, 220) : sf::Color(80, 130, 200));
    settingsButton.setPosition({200.f, 380.f});
    settingsButton.setOutlineThickness(4);
    settingsButton.setOutlineColor(sf::Color(50, 90, 150));
    window.draw(settingsButton);

    sf::Text settingsText(font);
    settingsText.setString("AUDIO");
    settingsText.setCharacterSize(45);
    settingsText.setStyle(sf::Text::Bold);
    settingsText.setFillColor(sf::Color::White);
    textBounds = settingsText.getLocalBounds();
    settingsText.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        380.f + (80.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(settingsText);

    if ((isHoveringPlay || isHoveringSettings) && !wasHovering) {
        audio.playHover();
    }
    wasHovering = isHoveringPlay || isHoveringSettings;

    sf::Text instructions(font);
    instructions.setString("Clic derecho para volver al menu");
    instructions.setCharacterSize(22);
    instructions.setFillColor(sf::Color(100, 100, 100));
    sf::FloatRect instBounds = instructions.getLocalBounds();
    instructions.setPosition({(WINDOW_SIZE - instBounds.size.x) / 2.f, 550.f});
    window.draw(instructions);
}

void drawModeSelect(sf::RenderWindow& window, const sf::Font& font) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 1));
        line.setPosition({0.f, static_cast<float>(i)});
        float ratio = static_cast<float>(i) / WINDOW_SIZE;
        std::uint8_t color = 230 - static_cast<std::uint8_t>(ratio * 50);
        line.setFillColor(sf::Color(color, color, color + 10));
        window.draw(line);
    }

    sf::Text title(font);
    title.setString("SELECCIONA MODO");
    title.setCharacterSize(60);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(60, 60, 80));
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition({(WINDOW_SIZE - titleBounds.size.x) / 2.f, 80.f});
    window.draw(title);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    
    // Botón 1 vs 1
    bool isHovering1v1 = (mousePos.x >= 200 && mousePos.x <= 500 && 
                          mousePos.y >= 220 && mousePos.y <= 300);
    sf::RectangleShape button1v1Shadow(sf::Vector2f(300, 80));
    button1v1Shadow.setFillColor(sf::Color(0, 0, 0, 60));
    button1v1Shadow.setPosition({205.f, 225.f});
    window.draw(button1v1Shadow);
    sf::RectangleShape button1v1(sf::Vector2f(300, 80));
    button1v1.setFillColor(isHovering1v1 ? sf::Color(70, 220, 70) : sf::Color(50, 200, 50));
    button1v1.setPosition({200.f, 220.f});
    button1v1.setOutlineThickness(4);
    button1v1.setOutlineColor(sf::Color(30, 150, 30));
    window.draw(button1v1);

    sf::Text text1v1(font);
    text1v1.setString("1 vs 1");
    text1v1.setCharacterSize(50);
    text1v1.setStyle(sf::Text::Bold);
    text1v1.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = text1v1.getLocalBounds();
    text1v1.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        220.f + (80.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(text1v1);

    // Botón vs IA
    bool isHoveringIA = (mousePos.x >= 200 && mousePos.x <= 500 && 
                         mousePos.y >= 330 && mousePos.y <= 410);
    sf::RectangleShape buttonIAShadow(sf::Vector2f(300, 80));
    buttonIAShadow.setFillColor(sf::Color(0, 0, 0, 60));
    buttonIAShadow.setPosition({205.f, 335.f});
    window.draw(buttonIAShadow);
    sf::RectangleShape buttonIA(sf::Vector2f(300, 80));
    buttonIA.setFillColor(isHoveringIA ? sf::Color(220, 150, 70) : sf::Color(200, 130, 50));
    buttonIA.setPosition({200.f, 330.f});
    buttonIA.setOutlineThickness(4);
    buttonIA.setOutlineColor(sf::Color(150, 90, 30));
    window.draw(buttonIA);

    sf::Text textIA(font);
    textIA.setString("vs IA");
    textIA.setCharacterSize(50);
    textIA.setStyle(sf::Text::Bold);
    textIA.setFillColor(sf::Color::White);
    textBounds = textIA.getLocalBounds();
    textIA.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        330.f + (80.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(textIA);

    // Botón Volver
    bool isHoveringBack = (mousePos.x >= 250 && mousePos.x <= 450 && 
                           mousePos.y >= 480 && mousePos.y <= 550);
    sf::RectangleShape backShadow(sf::Vector2f(200, 70));
    backShadow.setFillColor(sf::Color(0, 0, 0, 60));
    backShadow.setPosition({255.f, 485.f});
    window.draw(backShadow);
    sf::RectangleShape backButton(sf::Vector2f(200, 70));
    backButton.setFillColor(isHoveringBack ? sf::Color(100, 150, 220) : sf::Color(80, 130, 200));
    backButton.setPosition({250.f, 480.f});
    backButton.setOutlineThickness(3);
    backButton.setOutlineColor(sf::Color(50, 90, 150));
    window.draw(backButton);

    sf::Text backText(font);
    backText.setString("VOLVER");
    backText.setCharacterSize(40);
    backText.setStyle(sf::Text::Bold);
    backText.setFillColor(sf::Color::White);
    sf::FloatRect backBounds = backText.getLocalBounds();
    backText.setPosition({
        250.f + (200.f - backBounds.size.x) / 2.f - backBounds.position.x,
        480.f + (70.f - backBounds.size.y) / 2.f - backBounds.position.y
    });
    window.draw(backText);

    if ((isHovering1v1 || isHoveringIA || isHoveringBack) && !wasHovering) {
        audio.playHover();
    }
    wasHovering = isHovering1v1 || isHoveringIA || isHoveringBack;
}

void drawDifficultySelect(sf::RenderWindow& window, const sf::Font& font) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 1));
        line.setPosition({0.f, static_cast<float>(i)});
        float ratio = static_cast<float>(i) / WINDOW_SIZE;
        std::uint8_t color = 230 - static_cast<std::uint8_t>(ratio * 50);
        line.setFillColor(sf::Color(color, color, color + 10));
        window.draw(line);
    }

    sf::Text title(font);
    title.setString("DIFICULTAD");
    title.setCharacterSize(60);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(60, 60, 80));
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition({(WINDOW_SIZE - titleBounds.size.x) / 2.f, 80.f});
    window.draw(title);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    // Botón Fácil
    bool isHoveringEasy = (mousePos.x >= 200 && mousePos.x <= 500 && 
                           mousePos.y >= 200 && mousePos.y <= 270);
    sf::RectangleShape easyButtonShadow(sf::Vector2f(300, 70));
    easyButtonShadow.setFillColor(sf::Color(0, 0, 0, 60));
    easyButtonShadow.setPosition({205.f, 205.f});
    window.draw(easyButtonShadow);
    sf::RectangleShape easyButton(sf::Vector2f(300, 70));
    easyButton.setFillColor(isHoveringEasy ? sf::Color(120, 220, 120) : sf::Color(100, 200, 100));
    easyButton.setPosition({200.f, 200.f});
    easyButton.setOutlineThickness(3);
    easyButton.setOutlineColor(sf::Color(60, 160, 60));
    window.draw(easyButton);

    sf::Text easyText(font);
    easyText.setString("FACIL");
    easyText.setCharacterSize(45);
    easyText.setStyle(sf::Text::Bold);
    easyText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = easyText.getLocalBounds();
    easyText.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        200.f + (70.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(easyText);

    // Botón Medio
    bool isHoveringMedium = (mousePos.x >= 200 && mousePos.x <= 500 && 
                             mousePos.y >= 290 && mousePos.y <= 360);
    sf::RectangleShape mediumButtonShadow(sf::Vector2f(300, 70));
    mediumButtonShadow.setFillColor(sf::Color(0, 0, 0, 60));
    mediumButtonShadow.setPosition({205.f, 295.f});
    window.draw(mediumButtonShadow);
    sf::RectangleShape mediumButton(sf::Vector2f(300, 70));
    mediumButton.setFillColor(isHoveringMedium ? sf::Color(240, 200, 80) : sf::Color(220, 180, 60));
    mediumButton.setPosition({200.f, 290.f});
    mediumButton.setOutlineThickness(3);
    mediumButton.setOutlineColor(sf::Color(180, 140, 40));
    window.draw(mediumButton);

    sf::Text mediumText(font);
    mediumText.setString("MEDIO");
    mediumText.setCharacterSize(45);
    mediumText.setStyle(sf::Text::Bold);
    mediumText.setFillColor(sf::Color::White);
    textBounds = mediumText.getLocalBounds();
    mediumText.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        290.f + (70.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(mediumText);

    // Botón Difícil
    bool isHoveringHard = (mousePos.x >= 200 && mousePos.x <= 500 && 
                           mousePos.y >= 380 && mousePos.y <= 450);
    sf::RectangleShape hardButtonShadow(sf::Vector2f(300, 70));
    hardButtonShadow.setFillColor(sf::Color(0, 0, 0, 60));
    hardButtonShadow.setPosition({205.f, 385.f});
    window.draw(hardButtonShadow);
    sf::RectangleShape hardButton(sf::Vector2f(300, 70));
    hardButton.setFillColor(isHoveringHard ? sf::Color(240, 90, 90) : sf::Color(220, 70, 70));
    hardButton.setPosition({200.f, 380.f});
    hardButton.setOutlineThickness(3);
    hardButton.setOutlineColor(sf::Color(180, 40, 40));
    window.draw(hardButton);

    sf::Text hardText(font);
    hardText.setString("DIFICIL");
    hardText.setCharacterSize(45);
    hardText.setStyle(sf::Text::Bold);
    hardText.setFillColor(sf::Color::White);
    textBounds = hardText.getLocalBounds();
    hardText.setPosition({
        200.f + (300.f - textBounds.size.x) / 2.f - textBounds.position.x,
        380.f + (70.f - textBounds.size.y) / 2.f - textBounds.position.y
    });
    window.draw(hardText);

    // Botón Volver
    bool isHoveringBack = (mousePos.x >= 250 && mousePos.x <= 450 && 
                           mousePos.y >= 500 && mousePos.y <= 570);
    sf::RectangleShape backShadow(sf::Vector2f(200, 70));
    backShadow.setFillColor(sf::Color(0, 0, 0, 60));
    backShadow.setPosition({255.f, 505.f});
    window.draw(backShadow);
    sf::RectangleShape backButton(sf::Vector2f(200, 70));
    backButton.setFillColor(isHoveringBack ? sf::Color(100, 150, 220) : sf::Color(80, 130, 200));
    backButton.setPosition({250.f, 500.f});
    backButton.setOutlineThickness(3);
    backButton.setOutlineColor(sf::Color(50, 90, 150));
    window.draw(backButton);

    sf::Text backText(font);
    backText.setString("VOLVER");
    backText.setCharacterSize(40);
    backText.setStyle(sf::Text::Bold);
    backText.setFillColor(sf::Color::White);
    sf::FloatRect backBounds = backText.getLocalBounds();
    backText.setPosition({
        250.f + (200.f - backBounds.size.x) / 2.f - backBounds.position.x,
        500.f + (70.f - backBounds.size.y) / 2.f - backBounds.position.y
    });
    window.draw(backText);

    if ((isHoveringEasy || isHoveringMedium || isHoveringHard || isHoveringBack) && !wasHovering) {
        audio.playHover();
    }
    wasHovering = isHoveringEasy || isHoveringMedium || isHoveringHard || isHoveringBack;
}

void drawSettings(sf::RenderWindow& window, const sf::Font& font) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 1));
        line.setPosition({0.f, static_cast<float>(i)});
        float ratio = static_cast<float>(i) / WINDOW_SIZE;
        std::uint8_t color = 230 - static_cast<std::uint8_t>(ratio * 50);
        line.setFillColor(sf::Color(color, color, color + 10));
        window.draw(line);
    }

    sf::Text title(font);
    title.setString("CONFIGURACION DE AUDIO");
    title.setCharacterSize(50);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(60, 60, 80));
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition({(WINDOW_SIZE - titleBounds.size.x) / 2.f, 50.f});
    window.draw(title);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    sf::Text musicLabel(font);
    musicLabel.setString("Musica");
    musicLabel.setCharacterSize(35);
    musicLabel.setStyle(sf::Text::Bold);
    musicLabel.setFillColor(sf::Color(60, 60, 80));
    musicLabel.setPosition({100.f, 150.f});
    window.draw(musicLabel);

    sf::RectangleShape musicBar(sf::Vector2f(300, 20));
    musicBar.setPosition({100.f, 200.f});
    musicBar.setFillColor(sf::Color(180, 180, 180));
    musicBar.setOutlineThickness(2);
    musicBar.setOutlineColor(sf::Color(100, 100, 100));
    window.draw(musicBar);
    float musicFill = (audio.musicVolume / 100.f) * 300.f;
    sf::RectangleShape musicFillBar(sf::Vector2f(musicFill, 20));
    musicFillBar.setPosition({100.f, 200.f});
    musicFillBar.setFillColor(audio.musicMuted ? sf::Color(150, 150, 150) : sf::Color(80, 150, 255));
    window.draw(musicFillBar);

    sf::Text musicValue(font);
    musicValue.setString(std::to_string(static_cast<int>(audio.musicVolume)) + "%");
    musicValue.setCharacterSize(28);
    musicValue.setFillColor(sf::Color(60, 60, 80));
    musicValue.setPosition({420.f, 193.f});
    window.draw(musicValue);

    bool isHoveringMusicMute = (mousePos.x >= 500 && mousePos.x <= 600 && 
                                mousePos.y >= 190 && mousePos.y <= 230);
    sf::RectangleShape musicMuteBtn(sf::Vector2f(100, 40));
    musicMuteBtn.setPosition({500.f, 190.f});
    musicMuteBtn.setFillColor(isHoveringMusicMute ? sf::Color(220, 80, 80) : sf::Color(200, 60, 60));
    musicMuteBtn.setOutlineThickness(2);
    musicMuteBtn.setOutlineColor(sf::Color(150, 30, 30));
    window.draw(musicMuteBtn);

    sf::Text musicMuteText(font);
    musicMuteText.setString(audio.musicMuted ? "UNMUTE" : "MUTE");
    musicMuteText.setCharacterSize(25);
    musicMuteText.setStyle(sf::Text::Bold);
    musicMuteText.setFillColor(sf::Color::White);
    sf::FloatRect muteBounds = musicMuteText.getLocalBounds();
    musicMuteText.setPosition({
        500.f + (100.f - muteBounds.size.x) / 2.f - muteBounds.position.x,
        190.f + (40.f - muteBounds.size.y) / 2.f - muteBounds.position.y
    });
    window.draw(musicMuteText);

    sf::Text sfxLabel(font);
    sfxLabel.setString("Efectos de Sonido");
    sfxLabel.setCharacterSize(35);
    sfxLabel.setStyle(sf::Text::Bold);
    sfxLabel.setFillColor(sf::Color(60, 60, 80));
    sfxLabel.setPosition({100.f, 280.f});
    window.draw(sfxLabel);

    sf::RectangleShape sfxBar(sf::Vector2f(300, 20));
    sfxBar.setPosition({100.f, 330.f});
    sfxBar.setFillColor(sf::Color(180, 180, 180));
    sfxBar.setOutlineThickness(2);
    sfxBar.setOutlineColor(sf::Color(100, 100, 100));
    window.draw(sfxBar);
    float sfxFill = (audio.sfxVolume / 100.f) * 300.f;
    sf::RectangleShape sfxFillBar(sf::Vector2f(sfxFill, 20));
    sfxFillBar.setPosition({100.f, 330.f});
    sfxFillBar.setFillColor(audio.sfxMuted ? sf::Color(150, 150, 150) : sf::Color(255, 150, 80));
    window.draw(sfxFillBar);

    sf::Text sfxValue(font);
    sfxValue.setString(std::to_string(static_cast<int>(audio.sfxVolume)) + "%");
    sfxValue.setCharacterSize(28);
    sfxValue.setFillColor(sf::Color(60, 60, 80));
    sfxValue.setPosition({420.f, 323.f});
    window.draw(sfxValue);

    bool isHoveringSfxMute = (mousePos.x >= 500 && mousePos.x <= 600 && 
                             mousePos.y >= 320 && mousePos.y <= 360);
    sf::RectangleShape sfxMuteBtn(sf::Vector2f(100, 40));
    sfxMuteBtn.setPosition({500.f, 320.f});
    sfxMuteBtn.setFillColor(isHoveringSfxMute ? sf::Color(220, 80, 80) : sf::Color(200, 60, 60));
    sfxMuteBtn.setOutlineThickness(2);
    sfxMuteBtn.setOutlineColor(sf::Color(150, 30, 30));
    window.draw(sfxMuteBtn);

    sf::Text sfxMuteText(font);
    sfxMuteText.setString(audio.sfxMuted ? "UNMUTE" : "MUTE");
    sfxMuteText.setCharacterSize(25);
    sfxMuteText.setStyle(sf::Text::Bold);
    sfxMuteText.setFillColor(sf::Color::White);
    sf::FloatRect sfxMuteBounds = sfxMuteText.getLocalBounds();
    sfxMuteText.setPosition({
        500.f + (100.f - sfxMuteBounds.size.x) / 2.f - sfxMuteBounds.position.x,
        320.f + (40.f - sfxMuteBounds.size.y) / 2.f - sfxMuteBounds.position.y
    });
    window.draw(sfxMuteText);

    bool isHoveringBack = (mousePos.x >= 250 && mousePos.x <= 450 && 
                           mousePos.y >= 450 && mousePos.y <= 520);
    sf::RectangleShape backShadow(sf::Vector2f(200, 70));
    backShadow.setFillColor(sf::Color(0, 0, 0, 60));
    backShadow.setPosition({255.f, 455.f});
    window.draw(backShadow);
    sf::RectangleShape backButton(sf::Vector2f(200, 70));
    backButton.setFillColor(isHoveringBack ? sf::Color(100, 150, 220) : sf::Color(80, 130, 200));
    backButton.setPosition({250.f, 450.f});
    backButton.setOutlineThickness(3);
    backButton.setOutlineColor(sf::Color(50, 90, 150));
    window.draw(backButton);

    sf::Text backText(font);
    backText.setString("VOLVER");
    backText.setCharacterSize(40);
    backText.setStyle(sf::Text::Bold);
    backText.setFillColor(sf::Color::White);
    sf::FloatRect backBounds = backText.getLocalBounds();
    backText.setPosition({
        250.f + (200.f - backBounds.size.x) / 2.f - backBounds.position.x,
        450.f + (70.f - backBounds.size.y) / 2.f - backBounds.position.y
    });
    window.draw(backText);

    if ((isHoveringMusicMute || isHoveringSfxMute || isHoveringBack) && !wasHovering) {
        audio.playHover();
    }
    wasHovering = isHoveringMusicMute || isHoveringSfxMute || isHoveringBack;

    sf::Text instructions(font);
    instructions.setString("Arrastra en las barras para ajustar el volumen");
    instructions.setCharacterSize(20);
    instructions.setFillColor(sf::Color(100, 100, 100));
    sf::FloatRect instBounds = instructions.getLocalBounds();
    instructions.setPosition({(WINDOW_SIZE - instBounds.size.x) / 2.f, 580.f});
    window.draw(instructions);
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_SIZE, WINDOW_SIZE}), "Juego del Gato");
    window.setFramerateLimit(60);
    
    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        if (!font.openFromFile("C:/Windows/Fonts/calibri.ttf") &&
            !font.openFromFile("C:/Windows/Fonts/verdana.ttf") &&
            !font.openFromFile("arial.ttf")) {
            printf("No se pudo cargar ninguna fuente.\n");
            return -1;
        }
    }

    bool audioLoaded = audio.loadAudio();
    if (audioLoaded) {
        printf("Audio cargado correctamente.\n");
        audio.playMusic();
    } else {
        printf("El juego funcionara sin sonido.\n");
    }

    resetBoard();

    bool isDraggingMusic = false;
    bool isDraggingSfx = false;

    while (window.isOpen()) {
        
        // Lógica de movimiento de CPU
        if (currentState == GameState::Game && !gameOver && vsIA && currentPlayer == 'O' && !waitingForCPU) {
            waitingForCPU = true;
            cpuMoveClock.restart();
        }

        if (waitingForCPU && cpuMoveClock.getElapsedTime().asSeconds() > 0.5f) {
            cpuMakeMove(cpuDifficulty);
            audio.playMove();
            
            if (checkWinner()) {
                gameOver = true;
                currentState = GameState::GameOver;
                animationClock.restart();
                if (winnerText == "Empate!") {
                    audio.playDraw();
                } else {
                    audio.playWin();
                }
            } else {
                currentPlayer = 'X';
            }
            
            waitingForCPU = false;
        }
        
        while (std::optional<sf::Event> event = window.pollEvent()) { 
            
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>();
            if (mousePressed) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    int mx = mousePressed->position.x;
                    int my = mousePressed->position.y;
                    
                    if (currentState == GameState::Game && !gameOver && !waitingForCPU) {
                        // Solo permitir movimiento si es el turno del jugador
                        if (!vsIA || currentPlayer == 'X') {
                            int x = (mx - MARGIN) / CELL_SIZE;
                            int y = (my - MARGIN) / CELL_SIZE;
                            
                            if (x >= 0 && x < 3 && y >= 0 && y < 3 && board[y][x] == ' ') {
                                board[y][x] = currentPlayer;
                                audio.playMove();
                                
                                if (checkWinner()) {
                                    gameOver = true;
                                    currentState = GameState::GameOver;
                                    animationClock.restart();
                                    if (winnerText == "Empate!") {
                                        audio.playDraw();
                                    } else {
                                        audio.playWin();
                                    }
                                } else {
                                    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                                }
                            }
                        }
                    } else if (currentState == GameState::Menu) {
                        if (mx >= 200 && mx <= 500 && my >= 280 && my <= 360) {
                            audio.playClick(); 
                            currentState = GameState::ModeSelect;
                        }
                        else if (mx >= 200 && mx <= 500 && my >= 380 && my <= 460) {
                            audio.playClick(); 
                            currentState = GameState::Settings;
                        }
                    } else if (currentState == GameState::ModeSelect) {
                        if (mx >= 200 && mx <= 500 && my >= 220 && my <= 300) {
                            audio.playClick();
                            vsIA = false;
                            resetBoard();
                            currentState = GameState::Game;
                        }
                        else if (mx >= 200 && mx <= 500 && my >= 330 && my <= 410) {
                            audio.playClick();
                            currentState = GameState::DifficultySelect;
                        }
                        else if (mx >= 250 && mx <= 450 && my >= 480 && my <= 550) {
                            audio.playClick();
                            currentState = GameState::Menu;
                        }
                    } else if (currentState == GameState::DifficultySelect) {
                        if (mx >= 200 && mx <= 500 && my >= 200 && my <= 270) {
                            audio.playClick();
                            cpuDifficulty = Difficulty::Easy;
                            vsIA = true;
                            resetBoard();
                            currentState = GameState::Game;
                        }
                        else if (mx >= 200 && mx <= 500 && my >= 290 && my <= 360) {
                            audio.playClick();
                            cpuDifficulty = Difficulty::Medium;
                            vsIA = true;
                            resetBoard();
                            currentState = GameState::Game;
                        }
                        else if (mx >= 200 && mx <= 500 && my >= 380 && my <= 450) {
                            audio.playClick();
                            cpuDifficulty = Difficulty::Hard;
                            vsIA = true;
                            resetBoard();
                            currentState = GameState::Game;
                        }
                        else if (mx >= 250 && mx <= 450 && my >= 500 && my <= 570) {
                            audio.playClick();
                            currentState = GameState::ModeSelect;
                        }
                    } else if (currentState == GameState::GameOver) {
                        audio.playClick(); 
                        currentState = GameState::Menu;
                    } else if (currentState == GameState::Settings) {
                        if (mx >= 250 && mx <= 450 && my >= 450 && my <= 520) {
                            audio.playClick(); 
                            currentState = GameState::Menu;
                        }
                        else if (mx >= 500 && mx <= 600 && my >= 190 && my <= 230) {
                            audio.playClick(); 
                            audio.toggleMusicMute();
                        }
                        else if (mx >= 500 && mx <= 600 && my >= 320 && my <= 360) {
                            audio.playClick(); 
                            audio.toggleSfxMute();
                        }
                        else if (mx >= 100 && mx <= 400 && my >= 200 && my <= 220) {
                            isDraggingMusic = true;
                        }
                        else if (mx >= 100 && mx <= 400 && my >= 330 && my <= 350) {
                            isDraggingSfx = true;
                        }
                    }
                }
                
                if (mousePressed->button == sf::Mouse::Button::Right) {
                    if (currentState == GameState::Game || currentState == GameState::GameOver || 
                        currentState == GameState::Settings || currentState == GameState::ModeSelect ||
                        currentState == GameState::DifficultySelect) {
                        audio.playClick();
                        currentState = GameState::Menu;
                    }
                }
            }

            const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>();
            if (mouseReleased) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    isDraggingMusic = false;
                    isDraggingSfx = false;
                }
            }
        }

        if (currentState == GameState::Settings) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            
            if (isDraggingMusic) {
                float newVolume = ((mousePos.x - 100.f) / 300.f) * 100.f;
                newVolume = std::fmax(0.f, std::fmin(100.f, newVolume));
                audio.musicVolume = newVolume;
                audio.updateVolumes();
            }
            
            if (isDraggingSfx) {
                float newVolume = ((mousePos.x - 100.f) / 300.f) * 100.f;
                newVolume = std::fmax(0.f, std::fmin(100.f, newVolume));
                audio.sfxVolume = newVolume;
                audio.updateVolumes();
            }
        }

        window.clear(sf::Color::White);

        if (currentState == GameState::Menu) {
            drawMenu(window, font);
        } else if (currentState == GameState::ModeSelect) {
            drawModeSelect(window, font);
        } else if (currentState == GameState::DifficultySelect) {
            drawDifficultySelect(window, font);
        } else if (currentState == GameState::Game || currentState == GameState::GameOver) {
            drawGame(window, font);
        } else if (currentState == GameState::Settings) {
            drawSettings(window, font);
        }
        
        window.display();
    }
    
    return 0;
}