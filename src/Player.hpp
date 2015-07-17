#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <SFML/Graphics.hpp>

#include "ResourceManager.hpp"

#include "TileMap.hpp"

//move player movement to GameState.cpp
//why?: remove redundancy of TileMap*, ResourceManager, dt passing to updatePlayerMovement and calling from GameState.cpp....
//updatePlayerMovement function should take Pointer and move actually that entity!
//leave only player info/stats here

class Player : public sf::Drawable, public sf::Transformable
{
public:
	Player();

	void setMap(TileMap* map);
	void update(const float dt);
	void movePlayer(sf::Vector2f moveToPosition);

	void setPlayerTexture(sf::Texture& pTexture);

	
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	TileMap* map;

	micropather::MPVector<void*> movementPath;
	void updatePlayerMovement(const float dt);

	bool pathFound;
	int currentMovementIndex;
	sf::VertexArray pathLine;

	float lerp(float x, float y, float z);

	//remove pTexture?
	sf::Sprite pSprite;
	sf::RectangleShape pTileRect;
};

#endif /* PLAYER_HPP_ */
