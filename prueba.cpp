#include <SFML/Graphics.hpp>
int main() {
sf::RenderWindow window(sf::VideoMode(800, 600), "Prueba SFML");
sf::CircleShape circulo(50);
circulo.setFillColor(sf::Color::Green);
circulo.setPosition(375, 275);
while (window.isOpen()) {
sf::Event event;
while (window.pollEvent(event)) {
if (event.type == sf::Event::Closed)
window.close();
}
window.clear(sf::Color::Black);
window.draw(circulo);
window.display();
}
return 0;
}