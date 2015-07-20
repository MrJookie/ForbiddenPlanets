#include "TileMap.hpp"

using namespace rapidxml;

bool TileMap::loadFromFile(std::string fileName)
{
	rapidxml::file<> xmlFile(fileName.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(xmlFile.data());

	xml_node<>* map;
	map = doc.first_node("map");

	width = atoi(map->first_attribute("width")->value());
	height = atoi(map->first_attribute("height")->value());
	tilewidth = atoi(map->first_attribute("tilewidth")->value());
	tileheight = atoi(map->first_attribute("tileheight")->value());

	xml_node<>* tilesetElement;
	tilesetElement = map->first_node("tileset");
	while (tilesetElement)
	{
		Tileset tileset;
		tileset.firstgid = atoi(tilesetElement->first_attribute("firstgid")->value());
		tileset.name = tilesetElement->first_attribute("name")->value();
		tileset.tilewidth = atoi(tilesetElement->first_attribute("tilewidth")->value());
		tileset.tileheight = atoi(tilesetElement->first_attribute("tileheight")->value());

		xml_node<>* imageElement;
		imageElement = tilesetElement->first_node("image");
		tileset.source = imageElement->first_attribute("source")->value();
		tileset.sourcewidth = atoi(imageElement->first_attribute("width")->value());
		tileset.sourceheight = atoi(imageElement->first_attribute("height")->value());

		//todo: load to ResourceManager
		if (!tileset.tilesheet.loadFromFile("assets/" + tileset.source))
		{
			return false;
		}

		//tileset.tilesheet.setSmooth(true);

		tilesets.push_back(tileset);

		/*
		std::cout << tileset.firstgid << std::endl;
		std::cout << tileset.name << std::endl;
		std::cout << tileset.tilewidth << std::endl;
		std::cout << tileset.tileheight << std::endl;
		std::cout << tileset.source << std::endl;
		std::cout << tileset.sourcewidth << std::endl;
		std::cout << tileset.sourceheight << std::endl;
		std::cout << "-----" << std::endl;
		*/

		tilesetElement = tilesetElement->next_sibling("tileset");
	}

	//tilesets[0].tilesheet.setSmooth(true);
	
	//process tiles pixels into temp 2D array GID->color
	sf::Image image = tilesets[0].tilesheet.copyToImage();
	
	std::vector<std::vector<sf::Color>> gidToColors;
	gidToColors.resize(image.getSize().x / tilewidth * image.getSize().y / tileheight);
	
	std::size_t colorGID = 0;
	for(std::size_t tileY = 0; tileY < image.getSize().y / tileheight; ++tileY)
	{
		for(std::size_t tileX = 0; tileX < image.getSize().x / tilewidth; ++tileX)
		{
			for(std::size_t pixelY = 0; pixelY < tileheight; ++pixelY)
			{
				for(std::size_t pixelX = 0; pixelX < tilewidth; ++pixelX)
				{
					sf::Color pixelColor = image.getPixel(pixelX + (tilewidth * tileX), pixelY + (tileheight * tileY));
					
					gidToColors[colorGID].push_back(pixelColor);
					
					//std::cout << "pixelY: " << pixelY << " pixelX: " << pixelX << " --------------------------------------" << std::endl;
				}
			}
			
			colorGID++;
		}
	}
	
	//Color{x,x,x,0} = transparent
	
	int gidd = 0;
	std::cout << gidd << std::endl;
	std::cout << "color r: " << static_cast<int>(gidToColors[gidd][0].r) << std::endl;
	std::cout << "color g: " << static_cast<int>(gidToColors[gidd][0].g) << std::endl;
	std::cout << "color b: " << static_cast<int>(gidToColors[gidd][0].b) << std::endl;
	std::cout << "color a: " << static_cast<int>(gidToColors[gidd][0].a) << std::endl;
	
	std::vector<std::vector<sf::Color>*> mapTexture;

	int x = 0;
	int y = 0;

	//obtain TILED GID from flipped tiles
	const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
	const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
	const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;
	
	int layerID = 0;

	xml_node<>* layerElement;
	layerElement = map->first_node("layer");
	while (layerElement)
	{
		Layer layer;
		layer.name = layerElement->first_attribute("name") ? layerElement->first_attribute("name")->value() : "";
		layer.visible = layerElement->first_attribute("visible") ? false : true;
		layer.opacity = layerElement->first_attribute("opacity") ? atof(layerElement->first_attribute("opacity")->value()) : 1;
		layer.vertices.setPrimitiveType(sf::Quads);
		layer.vertices.resize(width * height * 4);
		layer.verticesToDraw.setPrimitiveType(sf::Quads);

		//resize 2D tile array
		layer.tiles.resize(width);
		for (std::size_t i = 0; i < height; ++i)
			layer.tiles[i].resize(height);

		xml_node<>* layerDataElement;
		layerDataElement = layerElement->first_node("data");

		xml_node<>* tileElement;
		tileElement = layerDataElement->first_node("tile");

		while (tileElement)
		{
			unsigned tileGID = atoll(tileElement->first_attribute("gid")->value());

			bool flipped_horizontally = (tileGID & FLIPPED_HORIZONTALLY_FLAG);
			bool flipped_vertically = (tileGID & FLIPPED_VERTICALLY_FLAG);
			bool flipped_diagonally = (tileGID & FLIPPED_DIAGONALLY_FLAG);

			tileGID &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

			layer.tiles[x][y].gid = tileGID;
			layer.tiles[x][y].position.x = x * tilewidth;
			layer.tiles[x][y].position.y = y * tileheight;
			layer.tiles[x][y].solid = (tileGID == 0) ? false : true;

			//-firstgid
			int tu = (tileGID - tilesets[0].firstgid) % (tilesets[0].tilesheet.getSize().x / tilewidth);
			int tv = (tileGID - tilesets[0].firstgid) / (tilesets[0].tilesheet.getSize().x / tilewidth);

			sf::Vertex* quad = &layer.vertices[(x + y * width) * 4];

			quad[0].color = sf::Color(255, 255, 255, 255 * layer.opacity);
			quad[1].color = sf::Color(255, 255, 255, 255 * layer.opacity);
			quad[2].color = sf::Color(255, 255, 255, 255 * layer.opacity);
			quad[3].color = sf::Color(255, 255, 255, 255 * layer.opacity);

			quad[0].position = sf::Vector2f((x + 0) * tilewidth, (y + 0) * tileheight);
			quad[1].position = sf::Vector2f((x + 1) * tilewidth, (y + 0) * tileheight);
			quad[2].position = sf::Vector2f((x + 1) * tilewidth, (y + 1) * tileheight);
			quad[3].position = sf::Vector2f((x + 0) * tilewidth, (y + 1) * tileheight);

			// - 0.0075 to prevent tearing on zoomed view: http://en.sfml-dev.org/forums/index.php?topic=6665.0
			if (flipped_horizontally && flipped_diagonally)
			{
				quad[0].texCoords = sf::Vector2f((tu + 0) * tilewidth, (tv + 1) * tileheight);
				quad[1].texCoords = sf::Vector2f((tu + 0) * tilewidth - 0.0075, (tv + 0) * tileheight);
				quad[2].texCoords = sf::Vector2f((tu + 1) * tilewidth - 0.0075, (tv + 0) * tileheight - 0.0075);
				quad[3].texCoords = sf::Vector2f((tu + 1) * tilewidth, (tv + 1) * tileheight - 0.0075);
			} 
			else if (flipped_vertically && flipped_diagonally)
			{
				quad[0].texCoords = sf::Vector2f((tu + 1) * tilewidth, (tv + 0) * tileheight);
				quad[1].texCoords = sf::Vector2f((tu + 1) * tilewidth - 0.0075, (tv + 1) * tileheight);
				quad[2].texCoords = sf::Vector2f((tu + 0) * tilewidth - 0.0075, (tv + 1) * tileheight - 0.0075);
				quad[3].texCoords = sf::Vector2f((tu + 0) * tilewidth, (tv + 0) * tileheight - 0.0075);
			}
			else if (flipped_horizontally && flipped_vertically)
			{
				quad[0].texCoords = sf::Vector2f((tu + 1) * tilewidth, (tv + 1) * tileheight);
				quad[1].texCoords = sf::Vector2f((tu + 0) * tilewidth - 0.0075, (tv + 1) * tileheight);
				quad[2].texCoords = sf::Vector2f((tu + 0) * tilewidth - 0.0075, (tv + 0) * tileheight - 0.0075);
				quad[3].texCoords = sf::Vector2f((tu + 1) * tilewidth, (tv + 0) * tileheight - 0.0075);
			}
			else  //no rotation
			{
				quad[0].texCoords = sf::Vector2f((tu + 0) * tilewidth, (tv + 0) * tileheight);
				quad[1].texCoords = sf::Vector2f((tu + 1) * tilewidth - 0.0075, (tv + 0) * tileheight);
				quad[2].texCoords = sf::Vector2f((tu + 1) * tilewidth - 0.0075, (tv + 1) * tileheight - 0.0075);
				quad[3].texCoords = sf::Vector2f((tu + 0) * tilewidth, (tv + 1) * tileheight - 0.0075);
			}
			
			
			//copy only first layer's pointer tile to colors
			if(layerID == 0)
			{
				if(tileGID < colorGID)
				{
					mapTexture.push_back(&gidToColors[tileGID]);
				}
			}
			
			x++;
			if (x == width)
			{
				x = 0;
				y++;
				if (y == height)
					y = 0;
			}
			
			tileElement = tileElement->next_sibling("tile");
		}
		layers.push_back(layer);
		
		layerID++;

		layerElement = layerElement->next_sibling("layer");
	}

	xml_node<>* objectgroupElement;
	objectgroupElement = map->first_node("objectgroup");
	while (objectgroupElement)
	{
		bool objectgroupVisible = objectgroupElement->first_attribute("visible") ? false : true;

		xml_node<>* objectElement;
		objectElement = objectgroupElement->first_node("object");
		while (objectElement)
		{
			//add object type?
			unsigned tileGID = atoll(objectElement->first_attribute("gid")->value());

			sf::Vector2f position;
			position.x = atof(objectElement->first_attribute("x")->value());
			position.y = atof(objectElement->first_attribute("y")->value());

			//determine which tile set to use, if gid lies in the current tileset, return pointer
			Tileset* tileset = &tilesets[0];
			std::size_t tilesetIndexUse = 0;
			for (std::size_t i = 0; i < tilesets.size(); ++i)
			{
				if (tileGID >= tilesets[i].firstgid && tileGID < tilesets[i+1].firstgid)
				{
					tileset = &tilesets[i];
					break;
				}
			}

			int tu = (tileGID - (*tileset).firstgid) % ((*tileset).tilesheet.getSize().x / (*tileset).tilewidth);
			int tv = (tileGID - (*tileset).firstgid) / ((*tileset).tilesheet.getSize().x / (*tileset).tilewidth);

			sf::Sprite sprite;
			sprite.setTexture((*tileset).tilesheet);
			sprite.setTextureRect(sf::Rect<int>(tu * (*tileset).tilewidth, tv * (*tileset).tileheight, (*tileset).tilewidth, (*tileset).tileheight));
			//-tileheight to correct tiled sprite position, 0,0 tile stores position 0,tileheight
			sprite.setPosition(position.x, position.y - (*tileset).tileheight);

			if (objectElement->first_attribute("rotation"))
			{
				sprite.setPosition(position.x, position.y);
				sprite.setOrigin(0, (*tileset).tileheight);
				sprite.setRotation(atof(objectElement->first_attribute("rotation")->value()));
			}
			
			Object object;
			object.name = objectElement->first_attribute("name") ? objectElement->first_attribute("name")->value() : "";
			object.visible = (objectElement->first_attribute("visible") || !objectgroupVisible) ? false : true;
			object.sprite = sprite;

			objects.push_back(object);

			objectElement = objectElement->next_sibling("object");
		}

		objectgroupElement = objectgroupElement->next_sibling("objectgroup");
	}

	//stress MicroPather
	//allocate 640x640 states
	pather = new micropather::MicroPather(this, width * height, 8, false);

	return true;
}

sf::Vector2i TileMap::getTileSize()
{
	return (sf::Vector2i(tilewidth, tileheight));
}

void TileMap::editTile(int x, int y)
{
	//layers[0].tiles[x][y].name = "";
	/* //remake
	layers[0].vertices[(x + y * width) * 4 + 0].position = sf::Vector2f(0, 0);
	layers[0].vertices[(x + y * width) * 4 + 1].position = sf::Vector2f(0, 0);
	layers[0].vertices[(x + y * width) * 4 + 2].position = sf::Vector2f(0, 0);
	layers[0].vertices[(x + y * width) * 4 + 3].position = sf::Vector2f(0, 0);
	*/
}

Tile& TileMap::getTileAt(sf::Vector2f position)
{
	std::size_t x = std::floor(position.x / tilewidth);
	std::size_t y = std::floor(position.y / tileheight);

	for (auto& layer : layers)
	{
		if (layer.visible)
		{
			if (layer.tiles[x][y].solid)
			{
				return layer.tiles[x][y];
			}
		}
	}

	return layers[0].tiles[x][y];
}

sf::Vector2i TileMap::getTilePos(sf::Vector2f position)
{
	return sf::Vector2i(std::floor(position.x / tilewidth), std::floor(position.y / tileheight));
}

//only VertexArray vertices, not counted Object(sprite) vertices
std::size_t TileMap::numVerticesToDraw()
{
	std::size_t verts = 0;
	for (const auto& layer : layers)
	{
		verts += layer.verticesToDraw.getVertexCount();
	}

	return verts;
}

std::size_t TileMap::numObjectsToDraw()
{
	return objectsToDraw.size();
}

void TileMap::update(const sf::View& view)
{
	//prepare onscreen tiles

	int startX = std::floor((view.getCenter().x - view.getSize().x / 2) / tilewidth);
	int endX = std::ceil((view.getCenter().x + view.getSize().x / 2) / tilewidth);
	int startY = std::floor((view.getCenter().y - view.getSize().y / 2) / tileheight);
	int endY = std::ceil((view.getCenter().y + view.getSize().y / 2) / tileheight);

	//std::cout << "startX: " << startX << " endX: " << endX << " startY: " << startY << " endY: " << endY << std::endl;

	if (startX > width)
		startX = 0; //width
	if (startX < 0)
		startX = 0;

	if (startY > height)
		startY = 0; //height
	if (startY < 0)
		startY = 0;

	if (endY > height)
		endY = height;
	if (endY < 0)
		endY = 0;

	if (endX > width)
		endX = width;
	if (endX < 0)
		endX = 0;

	for (auto& layer : layers)
	{
		if (layer.visible)
		{
			layer.verticesToDraw.clear();

			for (int x = startX; x < endX; ++x)
			{
				for (int y = startY; y < endY; ++y)
				{
					if (layer.tiles[x][y].solid)
					{
						layer.verticesToDraw.append(layer.vertices[(x + y * width) * 4 + 0]);
						layer.verticesToDraw.append(layer.vertices[(x + y * width) * 4 + 1]);
						layer.verticesToDraw.append(layer.vertices[(x + y * width) * 4 + 2]);
						layer.verticesToDraw.append(layer.vertices[(x + y * width) * 4 + 3]);
					}
				}
			}
		}
	}

	//prepare onscreen sprites 

	sf::FloatRect screenRect(
		sf::Vector2f(view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2),
		sf::Vector2f(view.getSize().x, view.getSize().y)
	);

	objectsToDraw.clear();
	for (auto& object : objects)
	{
		if (object.visible)
		{
			if (screenRect.intersects(object.sprite.getGlobalBounds()))
			{
				objectsToDraw.push_back(&object);
			}
		}
	}
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& layer : layers)
	{
		states.transform *= getTransform();
		states.texture = &tilesets[0].tilesheet;

		target.draw(layer.verticesToDraw, states);
	}

	for (const auto& object : objectsToDraw)
	{
		target.draw((*object).sprite, states);
	}

	
	/*
	sf::Texture texture;
	texture.loadFromFile("assets/2500.png");
	texture.setSmooth(false);

	sf::Sprite sprite;
	sprite.setTexture(texture);
	sprite.setTextureRect(sf::IntRect(0, 0, 512, 512));

	target.draw(sprite);
	*/
}

bool TileMap::startPathfinding(sf::Vector2i startPosition, sf::Vector2i finishPosition, micropather::MPVector<void*>& movementPath)
{
	//std::cout << "start: " << startPosition.x << ", " << startPosition.y << std::endl;
	//std::cout << "finish: " << finishPosition.x << ", " << finishPosition.y << std::endl;

	float totalCost = 0;
	int result = pather->Solve(pathXYToNode(startPosition.x, startPosition.y), pathXYToNode(finishPosition.x, finishPosition.y), &movementPath, &totalCost);

	for (int i = 0; i < movementPath.size(); ++i)
	{
		int x;
		int y;

		pathNodeToXY(movementPath[i], &x, &y);
		//if (i != 0 && i != path.size() - 1)
		{
			//layers[0].vertices[(x + y * width) * 4 + 0].color = sf::Color(0, 255, 0, 255);
			//layers[0].vertices[(x + y * width) * 4 + 1].color = sf::Color(0, 255, 0, 255);
			//layers[0].vertices[(x + y * width) * 4 + 2].color = sf::Color(0, 255, 0, 255);
			//layers[0].vertices[(x + y * width) * 4 + 3].color = sf::Color(0, 255, 0, 255);
		}
	}

	if (result == micropather::MicroPather::SOLVED)
	{
		//std::cout << "totalCost: " << totalCost << std::endl;
		//std::cout << "pathSize: " << movementPath.size() << std::endl;

		return true;
	}

	//if MicroPather cache is enabled
	//pather->Reset();

	return false;
}

bool TileMap::objectAtTile(int x, int y)
{
	sf::FloatRect tileRect;
	tileRect.left = x * tilewidth;
	tileRect.width = tilewidth;
	tileRect.top = y * tilewidth;
	tileRect.height = tileheight;

	for (auto& object : objects)
	{
		if (tileRect.intersects(object.sprite.getGlobalBounds()))
			return true;
	}

	return false;
}

bool TileMap::tileWalkable(int nx, int ny)
{
	if (nx >= 0 && nx < width && ny >= 0 && ny < height)
	{
		if (!layers[1].tiles[nx][ny].solid && !objectAtTile(nx, ny))
			return true;
	}

	return false;
}

void TileMap::pathNodeToXY(void* node, int* x, int* y)
{
	intptr_t index = (intptr_t)node;
	*y = index / width;
	*x = index - *y * width;
}

void* TileMap::pathXYToNode(int x, int y)
{
	return (void*)(x + y * width);
}

float TileMap::LeastCostEstimate(void* nodeStart, void* nodeEnd)
{
	//http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html

	sf::Vector2i startTile;
	sf::Vector2i endTile;

	pathNodeToXY(nodeStart, &startTile.x, &startTile.y);
	pathNodeToXY(nodeEnd, &endTile.x, &endTile.y);

	int dx = startTile.x - endTile.x;
	int dy = startTile.y - endTile.y;

	//Euclidean distance
	//return static_cast<float>(sqrt(static_cast<double>(dx*dx) + static_cast<double>(dy*dy)));

	//Manhattan distance
	return static_cast<float>(abs(dx) + abs(dy));
}

void TileMap::AdjacentCost(void* node, micropather::MPVector<micropather::StateCost>* neighbors)
{
	int x, y;
	const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const float cost[8] = { 1.0, 1.41, 1.0, 1.41, 1.0, 1.41, 1.0, 1.41 };

	pathNodeToXY(node, &x, &y);

	for (int i = 0; i < 8; ++i) {
		int nx = x + dx[i];
		int ny = y + dy[i];

		if (tileWalkable(nx, ny)) {
			switch (i)
			{
			case 1:
				//std::cout << "diagonal down right" << std::endl;
				if (tileWalkable(nx - 1, ny) && tileWalkable(nx, ny - 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 3:
				//std::cout << "diagonal down left" << std::endl;
				if (tileWalkable(nx + 1, ny) && tileWalkable(nx, ny - 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 5:
				//std::cout << "diagonal up left" << std::endl;
				if (tileWalkable(nx + 1, ny) && tileWalkable(nx, ny + 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 7:
				//std::cout << "diagonal up right" << std::endl;
				if (tileWalkable(nx - 1, ny) && tileWalkable(nx, ny + 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			default:
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;
			}
		}
	}
}

void TileMap::PrintStateInfo(void* node)
{
}
