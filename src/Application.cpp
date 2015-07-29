#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <stack>
#include <iostream>

#include "Application.hpp"
#include "ResourceManager.hpp"
#include "MenuState.hpp"
#include "GameState.hpp"

Application::Application() :
updateNumFrames(0),
fullscreen(0)
{
	loadResources();

	guiFPS.setFont(fonts.get("guiFont"));
	guiFPS.setPosition(5.f, 5.f);
	guiFPS.setCharacterSize(12u);

	guiVertices.setFont(fonts.get("guiFont"));
	guiVertices.setPosition(5.f, 20.f);
	guiVertices.setCharacterSize(12u);

	guiObjects.setFont(fonts.get("guiFont"));
	guiObjects.setPosition(5.f, 35.f);
	guiObjects.setCharacterSize(12u);

	guiMousePosition.setFont(fonts.get("guiFont"));
	guiMousePosition.setPosition(5.f, 50.f);
	guiMousePosition.setCharacterSize(12u);

	guiMouseWorldPosition.setFont(fonts.get("guiFont"));
	guiMouseWorldPosition.setPosition(5.f, 65.f);
	guiMouseWorldPosition.setCharacterSize(12u);

	guiTile.setFont(fonts.get("guiFont"));
	guiTile.setPosition(5.f, 80.f);
	guiTile.setCharacterSize(12u);

	guiResolution.setFont(fonts.get("guiFont"));
	guiResolution.setPosition(5.f, 95.f);
	guiResolution.setCharacterSize(12u);

	createWindow(800, 600, false);
}

Application::~Application()
{
	while (!states.empty())
		popState();
}

void Application::createWindow(sf::Uint16 width, sf::Uint16 height, bool fullscreen)
{
	sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();

	sf::ContextSettings contextSettings;
	//contextSettings.antialiasingLevel = 8;

	//move this later to menu settings, so user can set his own video mode
	//std::vector<sf::VideoMode> fullscreenVideoModes;
	//fullscreenVideoModes = sf::VideoMode::getFullscreenModes();
	
	if (fullscreen)
		window.create(sf::VideoMode(videoMode), "Forbidden Planets", sf::Style::Fullscreen);
	else
		window.create(sf::VideoMode(width, height), "Forbidden Planets");

    window.setFramerateLimit(10000);
	window.setKeyRepeatEnabled(true);
	//window.setVerticalSyncEnabled(false);
}

void Application::loadResources()
{
    textures.load("background", "assets/background.png");
	textures.load("background2", "assets/background2.png");
	textures.load("player", "assets/player.png");
	fonts.load("guiFont", "assets/Sansation.ttf");
}

void Application::pushState(State* state)
{
    states.push(state);
}

void Application::popState()
{
    delete states.top();
    states.pop();
}

void Application::changeState(State* state)
{
    if(!states.empty())
        popState();
    pushState(state);
}

State* Application::peekState()
{
    if(states.empty()) 
		return NULL;
    return states.top();
}

void Application::run()
{
	//pushState(new MenuState(this));
	pushState(new GameState(this));
	
	sfg::SFGUI sfgui;

    sf::Clock clock;

    while(window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

		if (peekState() == NULL) 
			continue;

		updateTime += elapsed;
		updateNumFrames += 1;
		if (updateTime >= sf::seconds(1.0f))
		{
			guiFPS.setString("FPS: " + std::to_string(updateNumFrames));

			updateTime -= sf::seconds(1.0f);
			updateNumFrames = 0;
		}

        peekState()->handleInput();
		peekState()->update(dt);

        // window.clear();

        peekState()->draw(dt);
		window.draw(guiFPS);
		
		window.draw(guiVertices);
		window.draw(guiObjects);
		window.draw(guiMousePosition);
		window.draw(guiMouseWorldPosition);
		window.draw(guiTile);
		window.draw(guiResolution);
		
		
		sfgui.Display(window);

        window.display();

		//window.setTitle("Forbidden Planets " + std::to_string(1.f / dt));
    }
}
