#ifndef GAMESTATE_HPP_
#define GAMESTATE_HPP_

#include <SFML/Graphics.hpp>

#include <iostream>
#include <ctime>

#ifdef WINDOWS
#include <direct.h> //mkdir
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif


#include "State.hpp"
#include "TileMap.hpp"
#include "Player.hpp"

class GameState : public State
{
public:
	GameState(Application* game);
	~GameState();

private:
	void draw(const float dt);
	void update(const float dt);
	void handleInput();
	void loadGui();
	void onResize();
	void onMouseWheelMoved(sf::Event event);
	void onMouseButtonPressed(sf::Event event);
	void moveScreen();
	void makeScreenshot();

	sf::View gridView;
	sf::View guiView;
	
	sf::Vector2f windowSize;
	sf::Vector2i mousePosition;
	sf::Vector2f mouseWorldPosition;

	int timesZoomed;
	float dt;
	sf::Uint16 screenMovementSpeed;

	TileMap map;
	Player player;
	
	sfg::Desktop desktop;
};

#endif /* GAMESTATE_HPP_ */
