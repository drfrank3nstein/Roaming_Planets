#include <string>
#include <iostream>

#include <SFML/Graphics.hpp>

#include "Arena.h"

int main()
{
    std::cout << "enter save file: ";
    std::string fileName;
    std::cin >> fileName;
    fileName = "saves/" + fileName + ".csv";
    std::fstream file(fileName, std::ios::in);
    if (!file.is_open()) {
        std::cout << "could not open the file" << std::endl;
        return 1;
    }
    
    Arena* arena = Arena::create(file);
    file.close();

    sf::Clock clock;
    int warp = 7;
    
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    sf::RenderWindow window(modes.front(), "Roaming Planets", sf::Style::Resize);
    arena->align(window);
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    
    while (window.isOpen()) {
        window.clear(sf::Color::Black);
        sf::Time elapsed = clock.restart();
        arena->step(elapsed.asSeconds(), warp);
        arena->paint(elapsed.asSeconds(), warp, (*Player::currentPlayer).first);
        arena->draw(window);
        window.display();
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonReleased:
                switch (event.mouseButton.button) {
                case sf::Mouse::Button::Left:
                    arena->shoot(
                        (*Player::currentPlayer).first,
                        sf::Mouse::getPosition(window)
                    );
                    break;
                }
                break;
            case sf::Event::KeyReleased:
                switch (event.key.code) {
                case sf::Keyboard::Space:
                    if (++Player::currentPlayer == Player::players.end()) {
                        Player::currentPlayer = Player::players.begin();
                    }
                    break;
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                }
            case sf::Event::MouseWheelScrolled:
                warp += event.mouseWheelScroll.delta;
                if (warp < 0) { warp = 0; }
                if (warp > 31) { warp = 31; }
                break;
            case sf::Event::Resized:
                window.setView(
                    sf::View(
                        sf::FloatRect(
                            0, 0, 
                            event.size.width, event.size.height 
                        )
                    )
                );
                arena->align(window);
            }
        }
    }
}