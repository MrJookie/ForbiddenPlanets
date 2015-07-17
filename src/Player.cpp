#include "Player.hpp"

Player::Player()
{
	//pTexture.loadFromFile("assets/player.png");
	//pTexture.setSmooth(true);
	//pSprite.setTexture(pTexture);

	pTileRect.setSize(sf::Vector2f(30.0f, 30.0f));
	pTileRect.setFillColor(sf::Color(0, 0, 0, 0));
	pTileRect.setOutlineColor(sf::Color(0, 255, 0, 255));
	pTileRect.setOutlineThickness(1.0f);

	pSprite.setOrigin(20 / 2, 20 / 2);
	pSprite.setPosition(67 * 32 + 16, 66 * 32 + 16);

	pathFound = false;
	currentMovementIndex = 1;
}

void Player::setMap(TileMap* map)
{
	this->map = map;
}

void Player::setPlayerTexture(sf::Texture& pTexture)
{
	pTexture.setSmooth(true);
	pSprite.setTexture(pTexture);
}

void Player::update(const float dt)
{
	//pSprite.setPosition(8 * map->getTileSize().x, 8 * map->getTileSize().y);

	pTileRect.setPosition(pSprite.getPosition().x + 1, pSprite.getPosition().y + 1);

	updatePlayerMovement(dt);
}

float Player::lerp(float x, float y, float t) {
	return ((1.0f - t) * x) + (t * y);
}

void Player::updatePlayerMovement(const float dt)
{
	if (pathFound)
	{
		int x, y;
		map->pathNodeToXY(movementPath[currentMovementIndex], &x, &y);

		sf::Vector2f delta;
		sf::Vector2f direction;
		sf::Vector2f playerPosition(pSprite.getPosition().x, pSprite.getPosition().y);
		sf::Vector2f targetPosition(x * 32 + 16, y * 32 + 16);

		/*
		int x2, y2;
		map->pathNodeToXY(movementPath[currentMovementIndex + 1], &x2, &y2);
		std::cout << "x: " << x << " y: " << y << " x2: " << x2 << " y2: " << y2 << std::endl;

		//move diagonally right down
		if ((currentMovementIndex + 1 < movementPath.size()) || (x2 > x && y2 > y))
		{
			targetPosition.x = x * 32 + 32;
			targetPosition.y = y * 32 + 16;
		}
		*/

		delta.x = targetPosition.x - playerPosition.x;
		delta.y = targetPosition.y - playerPosition.y;

		const float tolerance = 1.0f;
		const float distance = sqrt((delta.x * delta.x) + (delta.y * delta.y));
		const float displacement = 60 * dt;

		const float angle = atan2(delta.y, delta.x) * (180 / 3.14159265);
		const float rotationDiff = (((static_cast<int>(angle - pSprite.getRotation()) % 360) + 540) % 360) - 180;
		const float lerpAngle = lerp(pSprite.getRotation(), pSprite.getRotation() + rotationDiff, dt * 7);
		pSprite.setRotation(lerpAngle);

		direction.x = delta.x / distance;
		direction.y = delta.y / distance;
		
		//edit lines drawing
		pathLine.clear();
		pathLine.setPrimitiveType(sf::LinesStrip);
		pathLine.resize(movementPath.size());

		int a = 0;
		for (int i = 0; i < movementPath.size(); ++i)
		{
			/*
			if (i < currentMovementIndex)
				continue;

			if (i == currentMovementIndex)
			{
				pathLine[a].position = sf::Vector2f(playerPosition.x, playerPosition.y);
			}
			else
			{
				int x, y;
				map->pathNodeToXY(movementPath[i], &x, &y);
				pathLine[a].position = sf::Vector2f(x * map->getTileSize().x + map->getTileSize().x / 2, y * map->getTileSize().y + map->getTileSize().y / 2);
			}
			*/

			int x, y;
			map->pathNodeToXY(movementPath[i], &x, &y);
			pathLine[i].position = sf::Vector2f(x * map->getTileSize().x + map->getTileSize().x / 2, y * map->getTileSize().y + map->getTileSize().y / 2);

			a++;
		}

		if (distance <= tolerance || distance <= displacement)
		{
			playerPosition = targetPosition;
			//std::cout << "milestone reached!" << std::endl;

			if (++currentMovementIndex >= movementPath.size())
			{
				pathFound = false;
				currentMovementIndex = 1;
				movementPath.clear();
				pathLine.clear();

				//std::cout << "finish reached!" << std::endl;
			}
		}
		else
		{
			playerPosition.x += direction.x * displacement;
			playerPosition.y += direction.y * displacement;

			pSprite.setPosition(playerPosition);
		}
	}

}

void Player::movePlayer(sf::Vector2f moveToPosition)
{
	//if exact tile not walkable, try to check 12 adjacent tiles and if one of them is walkable, set new moveToPosition?
	if (map->tileWalkable(map->getTilePos(moveToPosition).x, map->getTilePos(moveToPosition).y))
	{
		pathFound = map->startPathfinding(map->getTilePos(pSprite.getPosition()), map->getTilePos(moveToPosition), movementPath);

		if (pathFound)
		{
			currentMovementIndex = 1;
		}
	}
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//target.draw(pTileRect);

	target.draw(pathLine);
	target.draw(pSprite);
}