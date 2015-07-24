#include "TileMap.hpp"
#include <map>
#include <set>
#include <iostream>
#include <fstream>
using namespace std;
using namespace rapidxml;

bool operator<(const Tile& a, const Tile& b) { return a.flags < b.flags; }

// function from shaders.cpp
GLuint loadShaders(const char* vertexShaderFile, const char* fragmentShaderFile);


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

		tilesets.push_back(tileset);
		tilesetElement = tilesetElement->next_sibling("tileset");
	}


	int x = 0;
	int y = 0;

	//obtain TILED GID from flipped tiles
	const unsigned FLIPPED_HORIZONTALLY_FLAG = 	0x80000000;
	const unsigned FLIPPED_VERTICALLY_FLAG = 	0x40000000;
	const unsigned FLIPPED_DIAGONALLY_FLAG = 	0x20000000;
	// tilesize must be 32 for map rendering
	const int cTilesize = 32;
	
	int layerID = 0;

	xml_node<>* layerElement;
	layerElement = map->first_node("layer");
	while (layerElement)
	{
		Layer layer;
		layer.name = layerElement->first_attribute("name") ? layerElement->first_attribute("name")->value() : "";
		layer.visible = layerElement->first_attribute("visible") ? false : true;
		layer.opacity = layerElement->first_attribute("opacity") ? atof(layerElement->first_attribute("opacity")->value()) : 1;


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
			// gid with flags is good for uniquelly identifying tile (rotated tile is new tile)
			layer.tiles[x][y].flags = tileGID;
			layer.tiles[x][y].gid = tileGID & ~( FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG );
			layer.tiles[x][y].solid = (tileGID != 0);
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
	
	// loading all tiles into tilesheet and indices for use in shader
	unsigned int indexCounter = 0;
	std::map<std::vector<Tile>, unsigned int> mapMultiLayerToIndex;
	std::vector<unsigned int> mapIndices;
	mapIndices.reserve(width * height);
	std::vector<Tile> ml;
	ml.resize(layers.size());
	for(int y=0; y < height; y++) {
		for(int x=0; x < width; x++) {
			// build tile id from using all layers
			for(int i = 0; i < layers.size(); i++) {
				ml[i] = ( layers[i].tiles[x][y] );
			}
			
			// if not already queued for processing, add it otherwise just add already used index
			if( mapMultiLayerToIndex.find( ml ) == mapMultiLayerToIndex.end() ) {
				mapMultiLayerToIndex.insert( std::make_pair( ml, indexCounter ) );
				mapIndices.push_back( (ml[1].solid ? 0x80000000U : 0) | indexCounter );
				indexCounter++;
			} else {
				mapIndices.push_back( (ml[1].solid ? 0x80000000U : 0) | mapMultiLayerToIndex[ ml ] );
			}
		}
	}
	const int tilesheet_width = 2048;
	const int n_tiles_per_width = 64;
	int ntiles = mapMultiLayerToIndex.size();
	m_tilesheet.create( tilesheet_width, ((ntiles / n_tiles_per_width)+1)*cTilesize );
	
	// copy all tilesets from texture to image for reading pixels
	sf::Image *tilesheets = new sf::Image[ tilesets.size() ];
	for(int i=0; i < tilesets.size(); i++) {
		tilesheets[i] = tilesets[i].tilesheet.copyToImage();
	}
	
	// tileset selector
	int *ts = new int[ layers.size() ];
	// offset for each tiles from layers
	sf::Vector2u *ofs = new sf::Vector2u[ layers.size() ];
	
	// filling up tiles
	for(auto& iter : mapMultiLayerToIndex) {
		for(int i = 0; i < layers.size(); i++) {
			int gid = iter.first[i].gid;
			
			// gid equal 0 is not drawn at all
			if( gid == 0 ) continue;
			
			// check which tileset is this tile in
			ts[i] = 0;
			
			for (int j = 0; j < tilesets.size(); ++j) {
				if (gid >= tilesets[j].firstgid && ( (j + 1 > tilesets.size()) || (gid < tilesets[j+1].firstgid))) {
					ts[i] = j;
					break;
				}
			}
			
			int cts = ts[i];
			// find offset for getting tile from tilesheet
			ofs[i] = sf::Vector2u( ((gid - tilesets[cts].firstgid) % (tilesheets[cts].getSize().x / tilewidth)) * tilewidth,
								   ((gid - tilesets[cts].firstgid) / (tilesheets[cts].getSize().x / tilewidth)) * tilewidth );
			
		}
		// 
		int xofs = (iter.second % n_tiles_per_width) * cTilesize;
		int yofs = iter.second / n_tiles_per_width * cTilesize;
		float xc,yc;
		for(int y = 0; y < cTilesize; y++)
		for(int x = 0; x < cTilesize; x++) {
			
			sf::Color p(0,0,0,255);
			for(int i = 0; i < layers.size(); i++) {
				
				if( iter.first[i].gid > 0 ) {
					sf::Color q;
					xc = x;
					yc = y;
					
					// image flipping
					if(iter.first[i].flags & FLIPPED_VERTICALLY_FLAG)
						yc = 31 - yc;
					if(iter.first[i].flags & FLIPPED_HORIZONTALLY_FLAG)
						xc = 31 - xc;
					if(iter.first[i].flags & FLIPPED_DIAGONALLY_FLAG) {
						int tmp = yc;
						yc = xc;
						xc = tmp;
					}
					
					// merge 2 layers
					q = tilesheets[ts[i]].getPixel( ofs[i].x + xc, ofs[i].y + yc );
					float o = layers[i].opacity * float(q.a)/255.0;
					float k = 1.0-o;
					p.r = p.r * k + q.r * o;
					p.g = p.g * k + q.g * o;
					p.b = p.b * k + q.b * o;
					
				}
			}
			// write pixel to new single tilesheet
			m_tilesheet.setPixel( xofs + x, yofs + y, p );
		}
	}
	m_ntiles = ntiles;
	m_mapIndices = mapIndices;
	delete[] ts;
	delete[] ofs;
	delete[] tilesheets;

	// ---- OpenGL part
	initOpenGL();

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

void TileMap::loadFromBinary(std::string fileName) {
	
	/*
		FILE STRUCTURE:
			width
			height
			indices.size
			indices
			ntiles
			tilesheet.x
			tilesheet.y
			tilesheet.data
			tilesets.size
			--- loop (n tilesets) ---
			tileset.source char index
			tileset.tilewidth
			tileset.tileheight
			objects.size
			objects.data
			-------------------------
			strings
	*/
	std::ifstream infile( fileName, std::ifstream::binary );
	#define GET_UINT( uint ) infile.read( (char*)&uint, sizeof(unsigned int) );
	// load indices
	int nindices;
	GET_UINT( width );
	GET_UINT( height );
	GET_UINT( nindices );
	m_mapIndices.resize( nindices );
	infile.read( (char*)&m_mapIndices[0], sizeof(unsigned int) * nindices );
	// load tilesheet
	int ts_x, ts_y;
	GET_UINT( m_ntiles );
	GET_UINT( ts_x );
	GET_UINT( ts_y );
	//sf::Image tilesheet;
	int img_memsize = ts_x * sizeof(unsigned int) * ts_y * sizeof(unsigned int);
	char* img_data = new char[ img_memsize  ];
	infile.read( img_data, img_memsize );
	//cout << ntiles << " " << ts_x << " " << ts_y << endl;
	m_tilesheet.create( ts_x, ts_y, (unsigned char*)img_data );
	delete[] img_data;
	
	// load tilesets with objects
	int ntilesets;
	GET_UINT( ntilesets );
	tilesets.resize( ntilesets );
	//cout << "loading " << ntilesets << " tilesets" << endl;
	std::vector<std::vector<ObjectExport>> tilesetObjects(ntilesets);
	
	for(int i = 0; i < ntilesets; i++) {
		GET_UINT( tilesets[i].firstgid );
		GET_UINT( tilesets[i].tilewidth );
		GET_UINT( tilesets[i].tileheight );
		int objsize;
		GET_UINT( objsize );
		//cout << "objsize: " << objsize << endl;
		tilesetObjects[i].resize(objsize);
		infile.read( (char*)&tilesetObjects[i][0], sizeof(ObjectExport) * objsize );
	}
	
	
	// load strings
	int curpos = infile.tellg();
	//cout << curpos << endl;
	if(curpos < 0) return;
	infile.seekg( 0, infile.end );
	int endpos = infile.tellg();
	int len = endpos - curpos;
	infile.seekg( curpos );
	char* strings = new char[ len ];
	infile.read( strings, len );
	infile.close();
	unsigned int  cTilesize = 32;
	// load tileset tilesheets (use firstgid for strings offset)
	for(int i = 0; i < ntilesets; i++) {
		std::string fn( &strings[ tilesets[i].firstgid ] );
		//cout << "Loading " << fn << endl;
		if(!tilesets[i].tilesheet.loadFromFile( "assets/" + fn )) {
			return;
		}
		for(const auto& objexp : tilesetObjects[i]) {
			// TODO: write rest of object calculation and preparation for rendering
			int tu = (objexp.gid) % (tilesets[i].tilesheet.getSize().x / tilesets[i].tilewidth);
			int tv = (objexp.gid) / (tilesets[i].tilesheet.getSize().x / tilesets[i].tilewidth);
			
			sf::Sprite sprite;
			sprite.setTexture(tilesets[i].tilesheet);
			sprite.setTextureRect(sf::Rect<int>(tu * tilesets[i].tilewidth, tv * tilesets[i].tileheight, tilesets[i].tilewidth, tilesets[i].tileheight));
			//-tileheight to correct tiled sprite position, 0,0 tile stores position 0,tileheight
			sprite.setPosition(objexp.pos.x, objexp.pos.y - tilesets[i].tileheight);
			if( objexp.rotation != 0 ) {
				sprite.setPosition(objexp.pos.x, objexp.pos.y);
				sprite.setOrigin(0, tilesets[i].tileheight);
				sprite.setRotation(objexp.rotation);
			}
			
			Object object;
			object.name = &strings[ objexp.name ];
			object.visible = objexp.visible;
			object.sprite = sprite;

			objects.push_back(object);
		}
	}
	delete[] strings;
	tilewidth = tileheight = 32;
	pather = new micropather::MicroPather(this, width * height, 8, false);
	// init gl
	initOpenGL();
}


sf::Vector2i TileMap::getTileSize()
{
	return (sf::Vector2i(tilewidth, tileheight));
}


sf::Vector2i TileMap::getTilePos(sf::Vector2f position)
{
	return sf::Vector2i(std::floor(position.x / tilewidth), std::floor(position.y / tileheight));
}

std::size_t TileMap::numObjectsToDraw()
{
	return objectsToDraw.size();
}

void TileMap::update(const sf::View& view)
{
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
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf( target.getView().getTransform().getMatrix() );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glUseProgram( shaders );
	glBindVertexArray( vertexArrayObject );
	glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray( 0 );
	glUseProgram( 0 );
	glPopMatrix();
	glPopMatrix();
	
	for (const auto& object : objectsToDraw)
	{
		//states.transform *= getTransform();
		states.texture = &tilesets[0].tilesheet;
		target.draw((*object).sprite, states);
	}

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
		// !layers[1].tiles[nx][ny].solid
		if ( !(m_mapIndices[ nx + ny*width ] & 0x80000000) && !objectAtTile(nx, ny))
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
	//const float cost[8] = { 1.0, 1.41, 1.0, 1.41, 1.0, 1.41, 1.0, 1.41 };
	const float cost[8] = { 1.0, 1, 1.0, 1, 1.0, 1, 1.0, 1 };

	pathNodeToXY(node, &x, &y);

	for (int i = 0; i < 8; ++i) {
		int nx = x + dx[i];
		int ny = y + dy[i];

		if (tileWalkable(nx, ny)) {
			switch (i)
			{
			case 1:
				//std::cout << "diagonal down right" << std::endl;
				if (tileWalkable(nx - 1, ny) || tileWalkable(nx, ny - 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 3:
				//std::cout << "diagonal down left" << std::endl;
				if (tileWalkable(nx + 1, ny) || tileWalkable(nx, ny - 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 5:
				//std::cout << "diagonal up left" << std::endl;
				if (tileWalkable(nx + 1, ny) || tileWalkable(nx, ny + 1))
				{
					micropather::StateCost nodeCost = { pathXYToNode(nx, ny), cost[i] };
					neighbors->push_back(nodeCost);
				}
				break;

			case 7:
				//std::cout << "diagonal up right" << std::endl;
				if (tileWalkable(nx - 1, ny) || tileWalkable(nx, ny + 1))
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


void TileMap::initOpenGL() {
	
	glGenTextures(1, &indexTexture);
	glGenTextures(1, &tilesTexture);
	
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray( vertexArrayObject );
	
	// bind textures to some unit hopefully not used by sfml
	glActiveTexture( GL_TEXTURE5 );
	glBindTexture( GL_TEXTURE_2D, indexTexture);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &m_mapIndices[0] );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture( GL_TEXTURE6 );
	glBindTexture( GL_TEXTURE_2D, tilesTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 2048, ( (m_ntiles / 64) + 1) * 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_tilesheet.getPixelsPtr() );

	glActiveTexture( GL_TEXTURE0 );
	
	// load shaders
	GLuint program = loadShaders( "assets/vertex.vert", "assets/fragment.frag" );
	
	// load attributes and uniforms
	vert = glGetAttribLocation( program, "vert" );
	texcoord = glGetAttribLocation( program, "texcoord" );
	GLuint tex1 = glGetUniformLocation( program, "tex1" );
	GLuint tex2 = glGetUniformLocation( program, "tex2" );
	glUniform1i( tex1, 5 );
	glUniform1i( tex2, 6 );
	
	shaders = program;
	
	// load VBO-s
	float vsize = float(20480);
	float vertx[8] = {
		0,0,
		0,vsize,
		vsize,vsize,
		vsize,0
	};
	
	float texsize = float(width*32);
	float texcoords[8] = {
		0, 0,
		0, texsize,
		texsize,texsize,
		texsize,0
	};
	
	GLuint vbo[2];
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertx), vertx, GL_STATIC_DRAW);
	glVertexAttribPointer(vert, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)(0));
	glEnableVertexAttribArray( vert );
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)(0));
	glEnableVertexAttribArray( texcoord );
	glBindVertexArray( 0 );
}
