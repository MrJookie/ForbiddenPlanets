#include <SFML/Graphics.hpp>

#include "GameState.hpp"

GameState::GameState(Application* game) :
timesZoomed(0),
screenMovementSpeed(1024)
{
	this->game = game;

	map.loadFromFile("assets/planet001.tmx");

	onResize();
	gridView.setCenter(0, 0); //edit this

	player.setMap(&map);
	player.setPlayerTexture(game->textures.get("player"));

	loadGui();
}


void GameState::loadGui()
{
	//just test of SFGUI

	//game->window.resetGLStates();

	// Create our OpenGL canvas window
	auto window1 = sfg::Window::Create();
	window1->SetTitle("TOPKEK");
	window1->SetPosition(sf::Vector2f(350.f, 80.f));

	// Create our SFML canvas window
	auto window2 = sfg::Window::Create();
	window2->SetTitle("Hue hue hue hue Hue hue hue hue Hue hue hue hue");
	window2->SetPosition(sf::Vector2f(500.f, 200.f));

	auto box = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL);

	auto button1 = sfg::Button::Create();
	auto button2 = sfg::Button::Create();
	auto label = sfg::Label::Create();

	button1->SetLabel("Foo");
	button2->SetLabel("Bar");
	label->SetText("Baz");

	// To add our widgets to the box we use the Pack() method instead of the
	// Add() method. This makes sure the widgets are added and layed out
	// properly in the box.
	box->Pack(button1);
	box->Pack(label);
	box->Pack(button2);

	// Just as with the window we can set the spacing between widgets
	// withing a box.
	box->SetSpacing(5.f);

	// Finally we add our box to the window as it's only child.
	// Notice that we don't have to add the children of a box to it's parent
	// Because all children and grandchildren and .... are automatically
	// considered descendents of the parent.
	//guiwindow->Add(box);

	sf::Image img1;
	img1.loadFromFile("assets/inv1.png");

	sf::Image img2;
	img2.loadFromFile("assets/inv2.png");

	auto book = sfg::Notebook::Create();
	auto inventory = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
	auto inventory2 = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
	auto table = sfg::Table::Create();
	auto table2 = sfg::Table::Create();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto tmp = sfg::Image::Create();
			tmp->SetImage(img1);
			table->Attach(tmp, sf::Rect<sf::Uint32>(i, j, 1, 1), sfg::Table::FILL | sfg::Table::EXPAND, sfg::Table::FILL, sf::Vector2f(5.f, 5.f));
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			auto tmp = sfg::Image::Create();
			tmp->SetImage(img2);
			table2->Attach(tmp, sf::Rect<sf::Uint32>(i, j, 1, 1), sfg::Table::FILL | sfg::Table::EXPAND, sfg::Table::FILL, sf::Vector2f(5.f, 5.f));
		}
	}

	inventory->Pack(table, false);
	inventory2->Pack(table2, false);
	book->AppendPage(inventory, sfg::Label::Create("Zippo"));
	book->AppendPage(inventory2, sfg::Label::Create("JNKR"));

	window1->Add(box);
	window2->Add(book);

	desktop.Add(window1);
	desktop.Add(window2);
}

void GameState::update(const float dt)
{
	windowSize = static_cast<sf::Vector2f>(game->window.getSize());
	this->dt = dt;

	game->window.setView(gridView);
	mousePosition = sf::Mouse::getPosition(game->window);
	mouseWorldPosition = game->window.mapPixelToCoords(sf::Mouse::getPosition(game->window));

	game->window.setTitle("Forbidden Planets Game " + std::to_string(1.f / dt));

	game->guiVertices.setString("VERTS: " + std::to_string(map.numVerticesToDraw()));
	game->guiObjects.setString("OBJECTS: " + std::to_string(map.numObjectsToDraw()));
	game->guiMousePosition.setString("X: " + std::to_string(mousePosition.x) + " Y: " + std::to_string(mousePosition.y));
	game->guiMouseWorldPosition.setString("X: " + std::to_string(static_cast<int>(mouseWorldPosition.x)) + " Y: " + std::to_string(static_cast<int>(mouseWorldPosition.y)));
	game->guiTile.setString("TILE: " + std::to_string(map.getTilePos(mouseWorldPosition).x) + ":" + std::to_string(map.getTilePos(mouseWorldPosition).y));
	game->guiResolution.setString("RESOLUTION: " + std::to_string(game->window.getSize().x) + "x" + std::to_string(game->window.getSize().y));

	//must be before map.update();
	moveScreen();

	map.update(gridView);
	player.update(dt);
	
	desktop.Update(0.f);
}

void GameState::moveScreen()
{
	sf::Vector2f windowSize = static_cast<sf::Vector2f>(game->window.getSize());
	sf::Vector2i mousePosition = sf::Mouse::getPosition(game->window);

	//std::ceil?
	if (game->window.hasFocus())
	{
		//corner screen movement
		if ((mousePosition.x >= 0 && mousePosition.x < 20) && (mousePosition.y >= 0 && mousePosition.y < 20))
		{
			gridView.move(sf::Vector2f(-screenMovementSpeed * dt, -screenMovementSpeed * dt));
		}
		else if ((mousePosition.x > (windowSize.x - 20) && mousePosition.x <= windowSize.x) && (mousePosition.y >= 0 && mousePosition.y < 20))
		{
			gridView.move(sf::Vector2f(screenMovementSpeed * dt, -screenMovementSpeed * dt));
		}
		else if ((mousePosition.x > (windowSize.x - 20) && mousePosition.x <= windowSize.x) && (mousePosition.y > (windowSize.y - 20) && mousePosition.y <= windowSize.y))
		{
			gridView.move(sf::Vector2f(screenMovementSpeed * dt, screenMovementSpeed * dt));
		}
		else if ((mousePosition.x >= 0 && mousePosition.x < 20) && (mousePosition.y >(windowSize.y - 20) && mousePosition.y <= windowSize.y))
		{
			gridView.move(sf::Vector2f(-screenMovementSpeed * dt, screenMovementSpeed * dt));
		}
		//x/y screen movement
		else if ((mousePosition.x >= 0 && mousePosition.x < 20) && (mousePosition.y >= 0 && mousePosition.y < windowSize.y))
		{
			gridView.move(sf::Vector2f(-screenMovementSpeed * dt, 0));
		}
		else if ((mousePosition.x > (windowSize.x - 20) && mousePosition.x <= windowSize.x) && (mousePosition.y >= 0 && mousePosition.y < windowSize.y))
		{
			gridView.move(sf::Vector2f(screenMovementSpeed * dt, 0));
		}
		else if (mousePosition.y >= 0 && mousePosition.y < 20 && (mousePosition.x >= 0 && mousePosition.x < windowSize.x))
		{
			gridView.move(sf::Vector2f(0, -screenMovementSpeed * dt));
		}
		else if ((mousePosition.y > (windowSize.y - 20) && mousePosition.y <= windowSize.y) && (mousePosition.x >= 0 && mousePosition.x < windowSize.x))
		{
			gridView.move(sf::Vector2f(0, screenMovementSpeed * dt));
		}
	}
}

void GameState::handleInput()
{
    sf::Event event;

	while (game->window.pollEvent(event))
    {
		desktop.HandleEvent(event);

        switch(event.type)
        {
            case sf::Event::Closed:
            {
				
                game->window.close();
                break;
            }
            case sf::Event::Resized:
            {
				//gridView.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
				//guiView.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
				/*
				game->background.setPosition(game->window.mapPixelToCoords(sf::Vector2i(0, 0)));
				*/
				onResize();
                break;
            }
			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Escape) game->window.close();
				if (event.key.code == sf::Keyboard::F) { //changed resolution/fullscreen mode
					sf::Vector2u size = game->window.getSize();
					game->window.close();
					game->createWindow(size.x, size.y, game->fullscreen ? false : true);
					game->fullscreen ? game->fullscreen = false : game->fullscreen = true;
					onResize();
				}
				if (event.key.code == sf::Keyboard::F5) makeScreenshot();
				if (event.key.code == sf::Keyboard::Space)
				{
					//player.movePlayer();
				}
				break;
			}
			case sf::Event::MouseWheelMoved:
			{
				onMouseWheelMoved(event);
				break;
			}
			case sf::Event::MouseButtonPressed:
			{
				onMouseButtonPressed(event);
				break;
			}

			default:
				break;
        }
    }
}

void GameState::makeScreenshot()
{
	sf::Image screenshot = game->window.capture();

	time_t epoch_time;
	struct tm *tm_p;
	epoch_time = time(NULL);
	tm_p = localtime(&epoch_time);
	if (tm_p)
	{
		std::string screenshotName = "Forbidden_Planets_" + std::to_string(tm_p->tm_year + 1900) + "-" + std::to_string(tm_p->tm_mon) + "-" + std::to_string(tm_p->tm_mday) + "_" + std::to_string(tm_p->tm_hour) + "-" + std::to_string(tm_p->tm_min) + "-" + std::to_string(tm_p->tm_sec);

		//#if defined(_WIN32)
		//	_mkdir("screenshots");
		//#else 
		//	mkdir("screenshots", 0777);
		//#endif

		if (screenshot.saveToFile("screenshots/" + screenshotName + ".png"))
		{
			std::cout << "Screenshot " + screenshotName + ".png saved." << std::endl;
		}
	}
}

void GameState::onResize()
{
	sf::Vector2f windowSize = static_cast<sf::Vector2f>(game->window.getSize());

	if (windowSize.x < 800)
		windowSize.x = 800;
	if (windowSize.y < 600)
		windowSize.y = 600;

	game->window.setSize(static_cast<sf::Vector2u>(windowSize));

	for (int i = 0; i < timesZoomed; ++i)
		gridView.zoom(1 / 1.1f);

	gridView.setCenter(gridView.getCenter());
	gridView.setSize(windowSize);

	guiView.setCenter(windowSize / 2.f);
	guiView.setSize(windowSize);

	gridView.zoom(1.5f);
}

void GameState::onMouseWheelMoved(sf::Event event)
{
	//game->window.setView(gridView);
	//std::cout << game->window.mapPixelToCoords({ event.mouseWheel.x, event.mouseWheel.y }).y << std::endl;

	if (event.mouseWheel.delta > 0)
	{
		if (timesZoomed > 0)
		{
			gridView.zoom(1.1f);
			timesZoomed--;
		}
	}
	else if (event.mouseWheel.delta < 0)
	{
		if (timesZoomed < 10)
		{
			gridView.zoom(1 / 1.1f);
			timesZoomed++;
		}
	}

	//std::cout << gridView.getSize().x << std::endl;
	//std::cout << gridView.getSize().y << std::endl;
}

void GameState::onMouseButtonPressed(sf::Event event)
{
	game->window.setView(gridView);
	sf::Vector2f position(game->window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y }));

	if (event.mouseButton.button == sf::Mouse::Left)
	{
		//pSprite.setPosition(map.getTilePos(position).x * 32, map.getTilePos(position).y * 32);

		//std::cout << map.tileOccupiedByObject(sf::Vector2i(map.getTilePos(position).x * 32, map.getTilePos(position).y * 32)) << std::endl;

		//pathfindingStart.x = map.getTilePos(pSprite.getPosition()).x;
		//pathfindingStart.y = map.getTilePos(pSprite.getPosition()).y;
	} 
	else if (event.mouseButton.button == sf::Mouse::Right)
	{
		//pSprite.setPosition(map.getTilePos(position).x * 32, map.getTilePos(position).y * 32);

		//pathfindingStart.x = map.getTilePos(pSprite.getPosition()).x;
		//pathfindingStart.y = map.getTilePos(pSprite.getPosition()).y;

		//pathfindingFinish.x = map.getTilePos(position).x;
		//pathfindingFinish.y = map.getTilePos(position).y;

		player.movePlayer(position);
	}
}

void GameState::draw(const float dt)
{
	game->window.clear();
	game->window.setView(gridView);

	game->window.draw(map);
	game->window.draw(player);

	game->window.setView(guiView);
}
