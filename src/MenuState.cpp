#include <SFML/Graphics.hpp>

#include "State.hpp"
#include "MenuState.hpp"
#include "GameState.hpp"

/*

!!SKIPPED FOR NOW!!

NEEDS A LOT OF IMPLEMENTATIONS TO BE COPIED FROM GameState.cpp!

*/

MenuState::MenuState(Application* game)
{
	this->game = game;

	game->background.setTexture(game->textures.get("background"));

	sf::Vector2f pos = sf::Vector2f(game->window.getSize());
	menuView.setSize(pos);

	pos *= 0.5f;
	menuView.setCenter(pos);
}

void MenuState::loadGame()
{
	game->pushState(new GameState(this->game));
}

void MenuState::update(const float dt)
{
	game->window.setTitle("Forbidden Planets Menu " + std::to_string(1.f / dt));
}

void MenuState::handleInput()
{
    sf::Event event;

    while(game->window.pollEvent(event))
    {
        switch(event.type)
        {
            case sf::Event::Closed:
            {
                game->window.close();
                break;
            }
            case sf::Event::Resized:
            {
                //menuView.setSize(event.size.width, event.size.height);
                game->background.setPosition(game->window.mapPixelToCoords(sf::Vector2i(0, 0)));
                game->background.setScale(
                    float(event.size.width) / float(game->background.getTexture()->getSize().x),
                    float(event.size.height) / float(game->background.getTexture()->getSize().y));

				onResize();
                break;
            }
            case sf::Event::KeyPressed:
            {
                if(event.key.code == sf::Keyboard::Escape) game->window.close();
				else if (event.key.code == sf::Keyboard::Space) loadGame();
                break;
            }
            default: 
				break;
        }
    }
}


void MenuState::onResize()
{
	sf::Vector2f size = static_cast<sf::Vector2f>(game->window.getSize());

	if (size.x < 800)
		size.x = 800;
	if (size.y < 600)
		size.y = 600;

	game->window.setSize(static_cast<sf::Vector2u>(size));

	menuView.setCenter(size / 2.f);
	menuView.setSize(size);
}

void MenuState::draw(const float dt)
{
	game->window.clear();
	game->window.setView(menuView);

	game->window.draw(game->background);
}