
#include <SFML\Graphics.hpp>

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>

#include <vector>
#include <string>
#include <memory>

#include "Chip8.hpp"

struct GameEntry
{
	sf::Text text;
	std::string file;
};

sf::RenderWindow window(sf::VideoMode(640, 480, 8), "Chip8 Emulator");
Chip8 chip8;
sf::Font font;

std::vector<GameEntry> LoadGames();
std::string ChooseGame(Chip8& chip8, sf::RenderWindow& window);

void UpdateInput(Chip8& chip8);

int main()
{
	std::string game = ChooseGame(chip8, window);
	if (game.empty())
		return -1;

	chip8.LoadGame(game);

	sf::Texture texture;
	texture.create(Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT);
	sf::Image image;
	image.create(Chip8::SCREEN_WIDTH, Chip8::SCREEN_HEIGHT);
	sf::Sprite screen;
	screen.setTexture(texture);
	screen.setScale(640.0f / Chip8::SCREEN_WIDTH, 480.0f / Chip8::SCREEN_HEIGHT);

	chip8.SetFrameRate(300);

	sf::Clock clock;
	sf::Time elapsedTime;
	sf::Event winEvent = {};
	while (window.isOpen())
	{
		while (window.pollEvent(winEvent))
		{
			switch (winEvent.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		elapsedTime += clock.restart();

		const sf::Time timePerCycle = sf::milliseconds(1000) / (sf::Int64)chip8.GetFrameRate();
		if (elapsedTime > timePerCycle)
		{
			elapsedTime -= timePerCycle;

			UpdateInput(chip8);

			chip8.EmulateCycle();

			if (chip8.mRedraw)
			{
				const uint8_t* vram = chip8.GetVram();
				for (unsigned int y = 0; y < Chip8::SCREEN_HEIGHT; ++y)
				{
					for (unsigned int x = 0; x < Chip8::SCREEN_WIDTH; ++x)
					{
						const sf::Color pixel = (vram[(y * Chip8::SCREEN_WIDTH) + x]) ? sf::Color::White : sf::Color::Black;
						image.setPixel(x, y, pixel);
					}
				}
				texture.update(image);

				window.clear(sf::Color::Magenta);

				window.draw(screen);

				window.display();

				chip8.mRedraw = false;
			}
		}
	}

	window.close();

	return 0;
}

std::string ChooseGame(Chip8& chip8, sf::RenderWindow& window)
{
	std::string game;

	auto gameEntries = LoadGames();
	if (gameEntries.empty())
		return game;

	sf::Clock clock;
	sf::Time elapsedTime = sf::milliseconds(100);
	size_t cur = 0;
	gameEntries[0].text.setFillColor(sf::Color::Green);
	sf::Event winEvent = {};
	bool running = true;
	while (running)
	{
		elapsedTime += clock.restart();
		while (window.pollEvent(winEvent))
		{
			switch (winEvent.type)
			{
			case sf::Event::Closed:
				running = false;
				break;
			}
		}

		if (elapsedTime.asMilliseconds() > 100)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				gameEntries[cur].text.setFillColor(sf::Color::White);
				if (cur == 0)
					cur = std::size(gameEntries) - 1;
				else
					--cur;
				gameEntries[cur].text.setFillColor(sf::Color::Green);
				elapsedTime = sf::Time::Zero;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				gameEntries[cur].text.setFillColor(sf::Color::White);
				++cur;
				if (cur >= std::size(gameEntries))
					cur = 0;
				gameEntries[cur].text.setFillColor(sf::Color::Green);
				elapsedTime = sf::Time::Zero;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
			{
				game = gameEntries[cur].file;
				running = false;
			}
		}

		window.clear();

		for (GameEntry entry : gameEntries)
		{
			window.draw(entry.text);
		}

		window.display();
	}

	return game;
}

std::vector<GameEntry> LoadGames()
{
	std::vector<GameEntry> gameEntries;

	rapidxml::file<> file("GameList.xml");
	std::unique_ptr<rapidxml::xml_document<>> doc(new rapidxml::xml_document<>);
	doc->parse<0>(file.data());

	rapidxml::xml_node<>* root = doc->first_node();
	if (!root)
		return gameEntries;

	if (!font.loadFromFile("FreeSans.ttf"))
		return gameEntries;

	for (rapidxml::xml_node<>* curGame = root->first_node();
		curGame;
		curGame = curGame->next_sibling())
	{
		rapidxml::xml_node<>* nameEl = curGame->first_node();
		rapidxml::xml_node<>* fileEl = nameEl->next_sibling();

		GameEntry entry;
		entry.text.setFont(font);
		entry.text.setCharacterSize(30);
		entry.text.setFillColor(sf::Color::White);
		entry.text.setPosition(200.0f, 50.0f + 150.0f * gameEntries.size());
		entry.text.setString(nameEl->value());

		entry.file = fileEl->value();

		gameEntries.push_back(entry);
	}

	doc->clear();

	return gameEntries;
}

void UpdateInput(Chip8& chip8)
{
	uint8_t* keys = chip8.GetKeys();

	keys[0x1] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) ? 1 : 0;
	keys[0x2] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) ? 1 : 0;
	keys[0x3] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) ? 1 : 0;
	keys[0xC] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) ? 1 : 0;

	keys[0x4] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) ? 1 : 0;
	keys[0x5] = (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) ? 1 : 0;
	keys[0x6] = (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) ? 1 : 0;
	keys[0xD] = (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) ? 1 : 0;

	keys[0x7] = (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ? 1 : 0;
	keys[0x8] = (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) ? 1 : 0;
	keys[0x9] = (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ? 1 : 0;
	keys[0xE] = (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) ? 1 : 0;

	keys[0xA] = (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) ? 1 : 0;
	keys[0x0] = (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) ? 1 : 0;
	keys[0xB] = (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) ? 1 : 0;
	keys[0xF] = (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) ? 1 : 0;
}