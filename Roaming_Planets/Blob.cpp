#include "Blob.h"

#include <math.h>

Blob::Blob(
sf::Vector2<float> init_pos,
sf::Vector2<float> init_vel,
float init_mass) :
position(init_pos),
velocity(init_vel),
mass(init_mass) {}

float Blob::total_mass = 0;

float Blob::getMass() {
	return mass;
}

void Blob::addMass(float add_mass) {
	mass += add_mass;
	total_mass += add_mass;
}

void Blob::nudge(const float& elapsed) {
	this->position += velocity * elapsed;
}

Blob::~Blob() {
	total_mass -= mass;
};

Moon::Moon(
	sf::Vector2<float> init_pos,
	sf::Vector2<float> init_vel,
	float init_mass)
	:
	Blob(init_pos, init_vel, init_mass) {
	total_mass += init_mass;
	baseCircle.setOutlineColor(sf::Color::Blue);
	baseCircle.setFillColor(sf::Color::Magenta);
}

float Moon::getRadius() {
	return std::cbrt(mass);
}

float Moon::osmolarity() {
	return mass;
}

void Moon::draw(
	sf::RenderWindow& window,
	float scale,
	sf::Vector2i offset
) {
	if (bigger) { baseCircle.setFillColor(sf::Color::Red); }
	else { baseCircle.setFillColor(sf::Color::Green); }
	baseCircle.setRadius(this->getRadius());
	baseCircle.setPosition(this->position);
	window.draw(center(baseCircle, scale, offset));
}

Moon::~Moon() {}

Player::Player(
	sf::Vector2<float> init_pos,
	sf::Vector2<float> init_vel,
	float init_mass,
	const std::string& playerName)
	:
	Blob(init_pos, init_vel, init_mass),
	name(playerName) {
	total_mass += init_mass;
	heart.setFillColor(sf::Color::Magenta);
	heart.setRadius(getRadius() / 2);
	players[playerName] = this;
}

//initialization of static map
std::map<std::string, Player*> Player::players;
std::map<std::string, Player*>::iterator Player::currentPlayer;

float Player::getRadius() {
	return std::cbrt(this->getMass());
}

float Player::osmolarity() {
	return this->getMass();
}

Blob* Player::shoot(sf::Vector2f crosshair) {
	const float fraction = 0.0625f;
	//proportional to the escape velocity
	const float momentum = 256 * this->getMass() / this->getRadius();
	Blob* projectile = new Moon(
		this->position,
		this->velocity,
		//bypasses the totalMass counter
		this->getMass() * fraction
	);
	//distributing the mass
	float total_buffer = this->getMass();
	this->addMass(-this->getMass() * fraction);
	//calculating the normalized vector towards the target
	sf::Vector2f norm_vec = sf::Vector2f(crosshair) - this->position;
	float dist = sqrt(norm_vec.x * norm_vec.x + norm_vec.y * norm_vec.y);
	norm_vec /= dist;
	//distributing the momentum
	projectile->velocity += norm_vec * momentum / projectile->getMass();
	this->velocity -= norm_vec * momentum / this->getMass();
	//mitosis!
	float separation = this->getRadius() + projectile->getRadius();
	projectile->position += this->getMass() * separation * norm_vec / total_buffer;
	this->position -= projectile->getMass() * separation * norm_vec / total_buffer;
	return projectile;
}

void Player::draw(
	sf::RenderWindow& window,
	float scale,
	sf::Vector2i offset
) {
	if (bigger) {
		baseCircle.setFillColor(sf::Color::Red);
	}
	else { baseCircle.setFillColor(sf::Color::Green); }
	baseCircle.setRadius(this->getRadius());
	heart.setRadius((cos(pacer) + 2) / 4 * this->getRadius());
	baseCircle.setPosition(this->position);
	heart.setPosition(this->position);
	window.draw(center(baseCircle, scale, offset));
	window.draw(center(heart, scale, offset));
}

Player::~Player() {
	players[name] = nullptr;
}

Black_hole::Black_hole(
	sf::Vector2<float> init_pos,
	sf::Vector2<float> init_vel,
	float init_mass)
	:
	Blob(init_pos, init_vel, init_mass) {
	baseCircle.setOutlineColor(sf::Color::Red);
	baseCircle.setFillColor(sf::Color::Black);
	baseCircle.setOutlineThickness(5);
}

void Black_hole::addMass(float add_mass) {
	mass += add_mass;
}

float Black_hole::getRadius() {
	return 0;
}

float Black_hole::osmolarity() {
	return this->getMass();
}

//anchors the damn thing in place
void Black_hole::nudge(const float &elapsed) {}

void Black_hole::draw(
	sf::RenderWindow& window,
	float scale,
	sf::Vector2i offset
) {
	baseCircle.setRadius(cbrt(this->getMass()) / 4);
	baseCircle.setPosition(this->position);
	window.draw(center(baseCircle, scale, offset));
}

Black_hole::~Black_hole() {}

void interact(Blob* blob_1, Blob* blob_2, const float& elapsed) {
	//same effect can be achieved by scaling the whole level
	const float G = 1;
	sf::Vector2<float> rel_pos = blob_2->position - blob_1->position;
	float dist_sq = rel_pos.x * rel_pos.x + rel_pos.y * rel_pos.y;
	float dist = sqrt(dist_sq);
	if (dist >= (blob_1->getRadius() + blob_2->getRadius())) {
		sf::Vector2<float> norm_vec = rel_pos / dist;
		blob_1->velocity += elapsed * G * norm_vec * blob_2->getMass() / dist_sq;
		blob_2->velocity -= elapsed * G * norm_vec * blob_1->getMass() / dist_sq;
	}
	else {
		float transfer = (blob_2->osmolarity() - blob_1->osmolarity()) * elapsed;
		//TODO clean this mess
		blob_1->addMass(-transfer);
		blob_2->addMass(transfer);
		sf::Vector2f momentum = (blob_2->velocity - blob_1->velocity) * transfer;
		if (blob_1->getMass() < 0) {
			blob_2->addMass(blob_1->getMass());
			throw false;
		}
		if (blob_2->getMass() < 0) {
			blob_1->addMass(blob_2->getMass());
			throw true;
		}
		if (transfer < 0) {
			blob_1->velocity -= momentum / blob_1->getMass();
		}
		else {
			blob_2->velocity -= momentum / blob_2->getMass();
		}
	}
}

inline sf::CircleShape center(
	sf::CircleShape circle,
	float scale,
	sf::Vector2i offset
) {
	circle.move(
		-circle.getRadius(),
		-circle.getRadius()
	);
	circle.setPosition(circle.getPosition() * scale);
	circle.move(sf::Vector2f(offset));
	circle.setRadius(circle.getRadius() * scale);
	return circle;
}