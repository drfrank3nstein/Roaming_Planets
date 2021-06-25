#pragma once

#include <string>
#include <list>
#include <fstream>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "Blob.h"

class Arena {
private:
	virtual std::pair<sf::Vector2i, sf::Vector2i> getBoundaries() = 0;
	sf::Texture stars;
	sf::Sprite background;
protected:
	static const float margins;
	sf::Font font;
	void align_background(const sf::RenderWindow& window);
	float scale = 1;
	sf::Vector2i offset;
	sf::Shape* walls;
	sf::Text message;
	std::list<Blob*> objects;
public:
	Arena();
	static Arena* create(std::istream& istrm);
	bool load(std::istream &istrm);
	void randomize(
		float max_mass, float max_momentum, int number,
		sf::Vector2i bounds
	);
	void randomize(
		float max_mass, float max_momentum, int number,
		sf::Vector2i tl_bounds,
		sf::Vector2i br_bounds
	);
	void step(float elapsed_s, int warp);
	virtual bool bounce(Blob* blob) = 0;
	virtual void align(const sf::RenderWindow& window) = 0;
	//colours the objects that can threaten the current player
	void paint(const float elapsed_s, const int warp, std::string player);
	void draw(sf::RenderWindow &window);
	void shoot(std::string player, sf::Vector2i mousePos);
	~Arena();
};

class Box : public Arena {
private:
	std::pair<sf::Vector2i, sf::Vector2i> getBoundaries();
protected:
	const sf::Vector2i bounds;
public:
	Box(sf::Vector2i bounds);
	bool bounce(Blob* blob);
	void align(const sf::RenderWindow& window);
};

class Ring : public Arena {
private:
	std::pair<sf::Vector2i, sf::Vector2i> getBoundaries();
protected:
	const int radius;
public:
	Ring(int radius);
	bool bounce(Blob* blob);
	void align(const sf::RenderWindow& window);
};