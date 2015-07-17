#ifndef STATE_HPP_
#define STATE_HPP_

#include "Application.hpp"

class State
{
 public:
	Application* game;

    virtual void draw(const float dt) = 0;
    virtual void update(const float dt) = 0;
    virtual void handleInput() = 0;
};

#endif /* STATE_HPP_ */
