#include <algorithm>
#include <random>
#include <time.h>
#include <math.h>

#include "Arena.h"

const float Arena::margins(0.25f);
Arena::Arena() {
	font.loadFromFile("Metropolis-Bold.otf");
	message.setFont(font);
	stars.loadFromFile("stars.jpg");
	background.setTexture(stars);
	background.setPosition(0, 0);
}

//switch for strings
enum csv_code {
	box,
	ring,
	moon,
	player,
	nebula,
	skyscape,
	black_hole,
};
csv_code code(std::string const& input) {
	if (input == "box") { return box; }
	if (input == "ring") { return ring; }
	if (input == "moon") { return moon; }
	if (input == "player") { return player; }
	if (input == "nebula") { return nebula; }
	if (input == "skyscape") { return skyscape; }
	if (input == "black_hole") { return black_hole; }
};

Arena* Arena::create(std::istream& istrm) {
	std::string meta, temp;
	std::vector<std::string> info;
	std::getline(istrm, meta);
	std::stringstream meta_s(meta);
	while (std::getline(meta_s, temp, ',')) {
		info.push_back(temp);
	}
	Arena* arena;
	switch (code(info[0])) {
	case box:
		arena = new Box(sf::Vector2i{
			std::stoi(info[1]),
			std::stoi(info[2])
			});
		break;
	case ring:
		arena = new Ring(std::stoi(info[1]));
		break;
	default:
		return nullptr;
	}
	if (arena->load(istrm)) {
		return nullptr;
	}
	return arena;
}

bool Arena::load(std::istream &istrm) {
	std::string line;
	while (std::getline(istrm, line)) {
		std::stringstream line_s(line);
		std::vector<std::string> info;
		std::string temp;
		while (std::getline(line_s, temp, ',')) {
			info.push_back(temp);
		}
		Blob* blob;
		switch (code(info[0])) {
		case moon:
			blob = new Moon(
				sf::Vector2<float>(std::stof(info[1]), std::stof(info[2])),
				sf::Vector2<float>(std::stof(info[3]), std::stof(info[4])),
				std::stof(info[5])
			);
			break;
		case player:
			blob = new Player(
				sf::Vector2<float>(std::stof(info[1]), std::stof(info[2])),
				sf::Vector2<float>(std::stof(info[3]), std::stof(info[4])),
				std::stof(info[5]), info[6]
			);
			break;
		case nebula:
			this->randomize(
				std::stof(info[1]), std::stof(info[2]), std::stof(info[3]),
				sf::Vector2i(std::stof(info[4]), std::stof(info[5])),
				sf::Vector2i(std::stof(info[6]), std::stof(info[7]))
			);
			continue; //no need to push back
		case skyscape:
			this->randomize(
				std::stof(info[1]), std::stof(info[2]), std::stof(info[3]),
				this->getBoundaries().first,
				this->getBoundaries().second
			);
			continue; //no need to push_back
		case black_hole:
			blob = new Black_hole(
				sf::Vector2<float>(std::stof(info[1]), std::stof(info[2])),
				sf::Vector2<float>(std::stof(info[3]), std::stof(info[4])),
				std::stof(info[5])
			);
			break;
		}
		objects.push_back(blob);
	}
	Player::currentPlayer = Player::players.begin();
	return 0;
}

void Arena::randomize(
	float max_mass, float max_momentum, int number,
	sf::Vector2i bounds
) {
	randomize(
		max_mass, max_momentum, number,
		sf::Vector2i{ 0, 0 },
		bounds
	);
}

void Arena::randomize(
	float max_mass, float max_momentum, int number,
	sf::Vector2i tl_bounds,
	sf::Vector2i br_bounds
) {
	srand(std::time(NULL));
	sf::Vector2i range = br_bounds - tl_bounds;
	for (int i = 0; i < number; i++) {
		float mass = fmod(rand(), max_mass);
		sf::Vector2f pos(
			rand() % range.x + tl_bounds.x,
			rand() % range.y + tl_bounds.y
		);
		float max_vel = max_momentum / mass;
		sf::Vector2f vel(
			fmod(rand(), 2*max_vel) - max_vel,
			fmod(rand(), 2*max_vel) - max_vel
		);
		Blob* blob;
		blob = new Moon(pos, vel, mass);
		if (this->bounce(blob)) {
			delete blob;
			i--;
			continue;
		}
		objects.push_back(blob);
	}
}

void Arena::step(float elapsed_s, int warp) {
	elapsed_s *= pow(1.125f, warp);
	//there should be each_with_each in std library
	for (
		auto iter_1 = objects.begin();
		iter_1 != objects.end();
		iter_1++
		) {
		auto iter_2 = iter_1;
		iter_2++;
		while (iter_2 != objects.end()) {
			try {
				interact(*iter_1, *iter_2, elapsed_s);
				iter_2++;
			}
			catch (bool eaten) {
				if (eaten) {
					delete *iter_2;
					iter_2 = objects.erase(iter_2);
				}
				else {
					delete *iter_1;
					iter_1 = objects.erase(iter_1);
					iter_2 = iter_1;
					iter_2++;
				}
			}
		}
	}
	std::for_each(
		objects.begin(),
		objects.end(),
		[elapsed_s](Blob* blob) {
			blob->nudge(elapsed_s);
		}
	);
	std::for_each(
		objects.begin(),
		objects.end(),
		[this](Blob* blob) {
			this->bounce(blob);
		}
	);
}

bool Box::bounce(Blob* blob) {
	bool flag = false;
	float radius = blob->getRadius();
	float
		breach_l = blob->position.x - radius - 0,
		breach_r = blob->position.x + radius - bounds.x,
		breach_t = blob->position.y - radius - 0,
		breach_b = blob->position.y + radius - bounds.y;
	if (breach_l < 0) {
		blob->velocity.x = -blob->velocity.x;
		blob->position.x -= 2 * breach_l;
		flag = true;
	}
	else if (breach_r > 0) {
		blob->velocity.x = -blob->velocity.x;
		blob->position.x -= 2 * breach_r;
		flag = true;
	}
	if (breach_t < 0) {
		blob->velocity.y = -blob->velocity.y;
		blob->position.y -= 2 * breach_t;
		flag = true;
	}
	else if (breach_b > 0) {
		blob->velocity.y = -blob->velocity.y;
		blob->position.y -= 2 * breach_b;
		flag = true;
	}
	return flag;
}

bool Ring::bounce(Blob* blob) {
	float dist = sqrt(
		blob->position.x * blob->position.x +
		blob->position.y * blob->position.y
	);
	float breach = dist + blob->getRadius() - radius;
	if (breach > 0) {
		float m = -(blob->position.x / blob->position.y);
		float m_m = m * m;
		float coeff_1 = (1 - m_m) / (1 + m_m);
		float coeff_2 = (2 * m / (1 + m_m));
		blob->velocity = {
			coeff_1 * blob->velocity.x + coeff_2 * blob->velocity.y,
			coeff_2 * blob->velocity.x - coeff_1 * blob->velocity.y
		};
		blob->position -= 2 * breach * blob->position / dist;
		return true;
	}
	else { return false; }
}

Box::Box(sf::Vector2i bounds) : bounds(bounds) {
	walls = new sf::RectangleShape();
	walls->setFillColor(sf::Color::Black);
	walls->setOutlineColor(sf::Color::Blue);
	walls->setOutlineThickness(5);
}

Ring::Ring(int radius) : radius(radius) {
	walls = new sf::CircleShape();
	walls->setFillColor(sf::Color::Black);
	walls->setOutlineColor(sf::Color::Blue);
	walls->setOutlineThickness(5);
}

std::pair<sf::Vector2i, sf::Vector2i> Box::getBoundaries() {
	return std::pair<sf::Vector2i, sf::Vector2i> {
		{0, 0},
		bounds
	};
}

std::pair<sf::Vector2i, sf::Vector2i> Ring::getBoundaries() {
	return std::pair<sf::Vector2i, sf::Vector2i> {
		{-radius, -radius},
		{ radius, radius }
	};
}

void Arena::align_background(const sf::RenderWindow& window) {
	float texture_scale = fmax(
		(float)window.getSize().x / (float)stars.getSize().x,
		(float)window.getSize().y / (float)stars.getSize().y
	);
	background.setScale(texture_scale, texture_scale);
}

void Box::align(const sf::RenderWindow& window) {
	align_background(window);
	scale = fmin(
		(window.getSize().x * (1 - margins)) / bounds.x,
		(window.getSize().y * (1 - margins)) / bounds.y
	);
	offset.x = (window.getSize().x - (bounds.x * scale)) / 2;
	offset.y = (window.getSize().y - (bounds.y * scale)) / 2;
	auto wall_ptr = static_cast<sf::RectangleShape*>(walls);
	wall_ptr->setPosition(offset.x, offset.y);
	wall_ptr->setSize(sf::Vector2f{ bounds.x * scale,bounds.y * scale });
	message.setPosition(wall_ptr->getPosition() + sf::Vector2f(offset) * margins);
}

void Ring::align(const sf::RenderWindow& window) {
	align_background(window);
	scale = fmin(
		(window.getSize().x * (1 - margins)) / (radius * 2),
		(window.getSize().y * (1 - margins)) / (radius * 2)
	);
	offset.x = window.getSize().x / 2;
	offset.y = window.getSize().y / 2;
	auto wall_ptr = static_cast<sf::CircleShape*>(walls);
	wall_ptr->setPointCount(radius / 4);
	wall_ptr->setRadius(radius);
	wall_ptr->setPosition(0, 0);
	*wall_ptr = center(*wall_ptr, scale, offset);
	message.setPosition(
		wall_ptr->getPosition().x + wall_ptr->getRadius() * margins / 2,
		wall_ptr->getPosition().y + wall_ptr->getRadius()
	);
}

void Arena::paint(const float elapsed_s, const int warp, std::string player) {
	float critical = 0;
	if (Player::players[player] != nullptr) {
		critical = Player::players[player]->osmolarity();
		Player::players[player]->pacer += (elapsed_s / Player::players[player]->getRadius()) * pow(1.125f, warp) * 16;
		if (critical > Blob::total_mass / 2) {
			message.setString("Critical mass achieved");
			message.setFillColor(sf::Color::Green);
		}
		else { message.setString(""); }
	}
	else {
		message.setString("Lifeform \"" + player + "\" terminated");
		message.setFillColor(sf::Color::Red);
	}
	for (Blob* blob : objects) {
		if (blob->osmolarity() > critical) {
			blob->bigger = true;
		}
		else { blob->bigger = false; }
	}
}

void Arena::draw(sf::RenderWindow &window) {
	window.draw(background);
	window.draw(*walls);
	//ensures that smaller blobs are visible
	objects.sort([](Blob* blob_1, Blob* blob_2) {
		return blob_1->getRadius() > blob_2->getRadius();
		});
	std::for_each(
		objects.begin(),
		objects.end(),
		[&window, this](Blob* blob) {
			blob->draw(window, this->scale, this->offset);
		}
	);
	window.draw(message);
}

void Arena::shoot(std::string player, sf::Vector2i mousePos) {
	mousePos -= offset;
	sf::Vector2f crosshair(mousePos);
	crosshair /= scale;
	if (Player::players[player] != nullptr) {
		objects.push_back(Player::players[player]->shoot(crosshair));
	}
}

Arena::~Arena() {
	for (Blob* blob : objects) {
		delete blob;
	}
}