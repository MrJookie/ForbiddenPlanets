#ifndef TILEMAP_HPP_
#define TILEMAP_HPP_
#define GL_GLEXT_PROTOTYPES
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <thread>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"
#include "Micropather/Micropather.h"

class Tileset
{
public:
	std::size_t firstgid;
	std::string name;
	std::size_t tilewidth;
	std::size_t tileheight;

	std::string source;
	std::size_t sourcewidth;
	std::size_t sourceheight;

	sf::Texture tilesheet;
};

class Tile
{
public:
	unsigned int gid;
	bool solid;
	sf::Vector2f position; //maybe remove?
	int flags;
};


//only one big tilesheet for different layers allowed, but many tilesheets with different sizes are allowed for objects, because they will be written as sf::Sprites, not sf::VertexArray
class Layer
{
public:
	std::string name;
	bool visible;
	float opacity;
	std::vector<std::vector<Tile>> tiles;
	sf::VertexArray vertices;
	sf::VertexArray verticesToDraw;
	//set tiles[x][y].solid = false; then it wont be drawn
};

class Object
{
public:
	std::string name;
	std::size_t gid;
	//std::string type;
	sf::Rect<int> rect;
	std::map<std::string, std::string> properties;
	sf::Sprite sprite;
	bool visible;
};

class TileMap : public sf::Drawable, public micropather::Graph
{
public:
	bool loadFromFile(std::string fileName);
	std::size_t numVerticesToDraw();
	std::size_t numObjectsToDraw();
	sf::Vector2i getTileSize();
	void editTile(int x, int y);
	bool objectAtTile(int x, int y);
	sf::Vector2i getTilePos(sf::Vector2f position);
	Tile& getTileAt(sf::Vector2f position);
	bool tileWalkable(int nx, int ny);
	void pathNodeToXY(void* node, int* x, int* y);
	void* pathXYToNode(int x, int y);

	micropather::MicroPather* pather;

	void update(const sf::View& view);
	bool startPathfinding(sf::Vector2i startPosition, sf::Vector2i finishPosition, micropather::MPVector<void*>& movementPath);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	//MicroPather
	virtual float LeastCostEstimate(void* stateStart, void* stateEnd);
	virtual void AdjacentCost(void* state, MP_VECTOR<micropather::StateCost>* adjacent);
	virtual void PrintStateInfo(void* state);
	std::vector<Object*> objectsToDraw;
	void initOpenGL();
private:
	std::size_t width;
	std::size_t height;
	std::size_t tilewidth;
	std::size_t tileheight;

	std::vector<Tileset> tilesets;
	std::vector<Layer> layers;
	std::vector<Object> objects;
	
	
	GLuint indexTexture;
	GLuint tilesTexture;
	GLuint shaders;
	GLuint vertexArrayObject;
	GLuint vert;
	GLuint texcoord;
	GLuint vbo[2];
	sf::Image m_tilesheet;
	std::vector<unsigned int> m_mapIndices;
	int m_ntiles;
};

#endif /* TILEMAP_HPP_ */
