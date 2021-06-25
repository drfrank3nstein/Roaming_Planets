#pragma once

#include <string>
#include <map>
#include <iostream>

#include <SFML/Graphics.hpp>
class Blob
{
protected:
	float mass;
public:
	sf::CircleShape baseCircle;
	sf::Vector2<float> position;
	sf::Vector2<float> velocity;
	bool bigger = true;
	Blob(
		sf::Vector2<float> init_pos,
		sf::Vector2<float> init_vel,
		float init_mass
	);
	static float total_mass;
	float getMass();
	virtual void addMass(float add_mass);
	virtual float getRadius() = 0;
	virtual float osmolarity() = 0;
	virtual void nudge(const float &elapsed);
	virtual void draw(
		sf::RenderWindow &window,
		float scale,
		sf::Vector2i offset
	) = 0;
	virtual ~Blob();
};

class Moon : public Blob {
public:
	Moon(
		sf::Vector2<float> init_pos,
		sf::Vector2<float> init_vel,
		float init_mass
	);
	float getRadius();
	float osmolarity();
	void draw(
		sf::RenderWindow &window,
		float scale,
		sf::Vector2i offset
	);
	~Moon();
};

class Player : public Blob {
public:
	Player(
		sf::Vector2f init_pos,
		sf::Vector2f init_vel,
		float init_mass,
		const std::string &playerName
		);
	static std::map<std::string, Player*> players;
	static std::map<std::string, Player*>::iterator currentPlayer;
	std::string name;
	float pacer = 0;
	sf::CircleShape heart;
	float getRadius();
	float osmolarity();
	//returns the projectile that has been shot
	Blob* shoot(sf::Vector2f crosshair);
	void draw(
		sf::RenderWindow &window,
		float scale,
		sf::Vector2i offset
	);
	~Player();
};

class Black_hole : public Blob {
public:
	Black_hole(
		sf::Vector2f init_pos,
		sf::Vector2f init_vel,
		float init_mass
	);
	void addMass(float add_mass);
	float getRadius();
	float osmolarity();
	void nudge(const float& elapsed);
	void draw(
		sf::RenderWindow& window,
		float scale,
		sf::Vector2i offset
	);
	~Black_hole();
};

void interact(
	Blob* blob_1,
	Blob* blob_2,
	const float& elapsed
);

inline sf::CircleShape center(
	sf::CircleShape circle,
	float scale,
	sf::Vector2i offset
);