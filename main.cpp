#include <iostream>
#include "FlatQuadTree.h"

int main()
{
	int win_width = 512*2;
	int win_height = 512*2;

	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "QR");

	FlatQuadTree qt;

	bool mouseButtonPressed = false;

	while (window.isOpen())
	{
		sf::Vector2i localPosition = sf::Mouse::getPosition(window);

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					qt.addElement(localPosition.x, localPosition.y);
					mouseButtonPressed = false;
				}
				else
					mouseButtonPressed = true;
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				mouseButtonPressed = false;
			}
		}

		window.clear();

		qt.draw(&window);
		
		window.display();
	}

	return 0;
}