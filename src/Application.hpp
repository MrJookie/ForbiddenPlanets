#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include <SFML/Graphics.hpp>

#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widget.hpp>

#include <stack>

#include "ResourceManager.hpp"

class State;

class Application
{
public:
	Application();
	~Application();

	std::stack<State*> states;

	sf::RenderWindow window;
	ResourceManager<sf::Texture> textures;
	ResourceManager<sf::Font> fonts;

	//todo: remove
	sf::Sprite background;

	void pushState(State* state);
	void popState();
	void changeState(State* state);
	State* peekState();

	void createWindow(sf::Uint16 width, sf::Uint16 height, bool fullscreen);
	void run();
	bool fullscreen;

	sf::Text guiVertices;
	sf::Text guiObjects;
	sf::Text guiMousePosition;
	sf::Text guiMouseWorldPosition;
	sf::Text guiTile;
	sf::Text guiResolution;

private:
	void loadResources();

	sf::Text guiFPS;
	sf::Time updateTime;
	std::size_t updateNumFrames;
};

#endif /* APPLICATION_HPP_ */
