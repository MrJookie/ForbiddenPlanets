
#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"
#include "SFML/Graphics.hpp"
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using rapidxml::xml_node;

class Tileset
{
public:
	std::size_t firstgid;
	std::string source;
	std::size_t tilewidth;
	std::size_t tileheight;
	sf::Texture tilesheet;
};

class Tile
{
public:
	unsigned int gid;
	int flags;
	bool solid;
};
bool operator<(const Tile& a, const Tile& b) { return a.flags < b.flags; }

class Layer
{
public:
	std::string name;
	bool visible;
	float opacity;
	std::vector<std::vector<Tile>> tiles;
};

class ObjectExport
{
public:
	int name;
	std::size_t gid;
	bool visible;
	//float x,y;
	sf::Vector2f pos;
	float rotation;
};

class StringPack {
	int cnt;
	std::vector<std::string> strings;
	char* buf;
	int lastbuf;
	public:
		StringPack() : cnt(0), buf(0),lastbuf(0) {}
		~StringPack() { if(buf) delete[] buf; }
		int put( std::string str ) {
			strings.push_back(str);
			int ret = cnt;
			cnt += str.size() + 1;
			return ret;
		}
		const char* c_str() {
			if(cnt == lastbuf && buf) return buf;
			int i=0;
			if(buf) delete[] buf;
			buf = new char[ cnt ];
			for(auto &s : strings) {
				for(auto &c : s) {
					buf[i++] = c;
				}
				buf[i++] = 0;
			}
			lastbuf = cnt;
			return (const char*)buf;
		}
		int size() { return cnt; }
};


bool loadFromFile(std::string fileName)
{
	int width,height;
	std::vector<ObjectExport> objects;
	std::vector<Tileset> tilesets;
	rapidxml::file<> xmlFile(fileName.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(xmlFile.data());

	xml_node<>* map = doc.first_node("map");

	width = atoi(map->first_attribute("width")->value());
	height = atoi(map->first_attribute("height")->value());

	xml_node<>* tilesetElement = map->first_node("tileset");
	while (tilesetElement)
	{
		Tileset tileset;
		tileset.firstgid = atoi(tilesetElement->first_attribute("firstgid")->value());
		tileset.tilewidth = atoi(tilesetElement->first_attribute("tilewidth")->value());
		tileset.tileheight = atoi(tilesetElement->first_attribute("tileheight")->value());
		
		xml_node<>* imageElement;
		imageElement = tilesetElement->first_node("image");
		tileset.source = imageElement->first_attribute("source")->value();
		
		if (!tileset.tilesheet.loadFromFile("tilesheets/" + tileset.source))
			return false;
	
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
	std::vector<Layer> layers;
	xml_node<>* layerElement = map->first_node("layer");
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

		xml_node<>* layerDataElement = layerElement->first_node("data");
		xml_node<>* tileElement = layerDataElement->first_node("tile");
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
				mapIndices.push_back( (ml[1].solid ? 0x80000000 : 0) | indexCounter );
				indexCounter++;
			} else {
				mapIndices.push_back( (ml[1].solid ? 0x80000000 : 0) | mapMultiLayerToIndex[ ml ] );
			}
		}
	}
	const int tilesheet_width = 2048;
	const int n_tiles_per_width = 64;
	int ntiles = mapMultiLayerToIndex.size();
	sf::Image tilesheet;
	tilesheet.create( tilesheet_width, ((ntiles / n_tiles_per_width)+1)*cTilesize );
	
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
			ofs[i] = sf::Vector2u( ((gid - tilesets[cts].firstgid) % (tilesheets[cts].getSize().x / cTilesize)) * cTilesize,
								   ((gid - tilesets[cts].firstgid) / (tilesheets[cts].getSize().x / cTilesize)) * cTilesize );
			
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
					q = tilesheets[ts[i]].getPixel( ofs[i].x + xc, ofs[i].y + yc );
					// merge 2 layers
					float o = layers[i].opacity * float(q.a)/255.0;
					float k = 1.0-o;
					p.r = p.r * k + q.r * o;
					p.g = p.g * k + q.g * o;
					p.b = p.b * k + q.b * o;
				}
			}
			// write pixel to new single tilesheet
			tilesheet.setPixel( xofs + x, yofs + y, p );
		}
	}
	delete[] ts;
	delete[] ofs;
	delete[] tilesheets;
	std::vector<bool> tilesetUsed(tilesets.size(), false);
	std::vector<std::vector<ObjectExport>> tilesetObjects(tilesets.size());
	StringPack strings;
	
	xml_node<>* objectgroupElement = map->first_node("objectgroup");
	while (objectgroupElement)
	{
		bool objectgroupVisible = objectgroupElement->first_attribute("visible") ? false : true;

		xml_node<>* objectElement;
		objectElement = objectgroupElement->first_node("object");
		while (objectElement)
		{
			//add object type?
			unsigned tileGID = atoll(objectElement->first_attribute("gid")->value());
			unsigned &gid = tileGID; // an alias
			
			float x = atof(objectElement->first_attribute("x")->value());
			float y = atof(objectElement->first_attribute("y")->value());

			int tileset = 0;
			for (int j = 0; j < tilesets.size(); ++j) {
				if (gid >= tilesets[j].firstgid && ( (j + 1 > tilesets.size()) || (gid < tilesets[j+1].firstgid))) {
					tileset = j;
					break;
				}
			}
			tilesetUsed[tileset] = true;
			
			float rotation = 0;

			if (objectElement->first_attribute("rotation")) {
				rotation = atof(objectElement->first_attribute("rotation")->value());
			}
			
			ObjectExport object;
			const char* objname = (objectElement->first_attribute("name") ? objectElement->first_attribute("name")->value() : "");
			object.name = strings.put(objname);
			object.visible = (objectElement->first_attribute("visible") || !objectgroupVisible) ? false : true;
			object.gid = gid - tilesets[tileset].firstgid;
			object.pos = sf::Vector2f( x, y );
			object.rotation = rotation;

			tilesetObjects[tileset].push_back( object );

			objectElement = objectElement->next_sibling("object");
		}

		objectgroupElement = objectgroupElement->next_sibling("objectgroup");
	}
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
	std::ofstream outfile( fileName.replace( fileName.size() - 3, 3, "map"), std::ofstream::binary );
	#define PUT_UINT( uint ) \
		tmp = uint; \
		outfile.write( (const char*)&tmp, sizeof(unsigned int) );
	unsigned int tmp;
	PUT_UINT( width );
	PUT_UINT( height );
	PUT_UINT( mapIndices.size() );
	outfile.write( (const char*)&mapIndices[0], sizeof(unsigned int) * mapIndices.size() );
	PUT_UINT( ntiles );
	PUT_UINT( tilesheet.getSize().x );
	PUT_UINT( tilesheet.getSize().y; );
	outfile.write( (const char*)tilesheet.getPixelsPtr(), tilesheet.getSize().x * sizeof(unsigned int) * tilesheet.getSize().y * sizeof( unsigned int ) );
	int ntilesets = 0;
	for( int i = 0; i < tilesets.size(); i++ )
		if(tilesetUsed[i]) ntilesets++;
	PUT_UINT( ntilesets );
	for( int i = 0; i < tilesets.size(); i++) {
		if(!tilesetUsed[i]) continue;
		// tileset source, width, height
		PUT_UINT( strings.put( tilesets[i].source )  );
		PUT_UINT( tilesets[i].tilewidth );
		PUT_UINT( tilesets[i].tileheight );
		// length of objects with current tileset
		PUT_UINT( tilesetObjects[i].size() );
		// objects from current tileset
		outfile.write( (const char*)&tilesetObjects[i][0], sizeof(ObjectExport) * tilesetObjects[i].size() );
	}
	// write all strings
	outfile.write( strings.c_str(), strings.size() );
	outfile.close();
}

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

void loadbinary(std::string fileName) {
	
	/*
		FILE STRUCTURE:
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
	int width, height, nindices;
	GET_UINT( width );
	GET_UINT( height );
	GET_UINT( nindices );
	std::vector<unsigned int> mapIndices(nindices);
	infile.read( (char*)&mapIndices[0], sizeof(unsigned int) * nindices );
	// load tilesheet
	int ts_x, ts_y, ntiles;
	GET_UINT( ntiles );
	GET_UINT( ts_x );
	GET_UINT( ts_y );
	sf::Image tilesheet;
	int img_memsize = ts_x * sizeof(unsigned int) * ts_y * sizeof(unsigned int);
	char* data = new char[ img_memsize  ];
	infile.read( data, img_memsize );
	//cout << ntiles << " " << ts_x << " " << ts_y << endl;
	tilesheet.create( ts_x, ts_y, (unsigned char*)data );
	delete[] data;
	// load tilesets with objects
	int ntilesets;
	GET_UINT( ntilesets );
	std::vector<Tileset> tilesets;
	tilesets.resize( ntilesets );
	std::vector<Object> objects;
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
	/*
	cout << "Loaded strings: " << endl;
	cout << "--------------------------------" << endl;
	for(int i = 0; i < len; i++)
		cout << strings[i];
	cout <<endl;
	cout << "--------------------------------" << endl;
	*/
	unsigned int  cTilesize = 32;
	// load tileset tilesheets (use firstgid for strings offset)
	for(int i = 0; i < ntilesets; i++) {
		std::string fn( &strings[ tilesets[i].firstgid ] );
		cout << "Loading " << fn << endl;
		if(!tilesets[i].tilesheet.loadFromFile( fn )) {
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
	// init gl
}


int main(int argc, char* argv[]) {
	
	// only one parameter and that is map_name.tmx, and it will output to map_name.map
	if(argc == 2) {
		if(!loadFromFile( argv[1] )) {
			return 1;
		}
	} else if(argc == 3) {
		loadbinary( argv[2] );
	}
	return 0;
}
