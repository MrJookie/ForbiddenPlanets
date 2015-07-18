#include <iostream>

#include "MenuState.hpp"

int main()
{
	try {
		Application game;
		game.run();
	}
	catch (std::exception& e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
	}

    return 0;
}
