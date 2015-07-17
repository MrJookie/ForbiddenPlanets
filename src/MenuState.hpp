#ifndef MENUSTATE_HPP_
#define MENUSTATE_HPP_

#include <SFML/Graphics.hpp>

#include "State.hpp"

class MenuState : public State
{
public:
	MenuState(Application* game);

private:
	void draw(const float dt);
	void update(const float dt);
	void handleInput();
	void loadGame();
	void onResize();
	
	sf::View menuView;
};

#endif /* MENUSTATE_HPP_ */
