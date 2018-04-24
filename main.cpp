#include <iostream>
#include "FlatQuadTree.h"

int main()
{
	int win_width = 512*2;
	int win_height = 512*2;

	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "QR");

	FlatQuadTree qt;

	bool mouseButtonPressed = false;

	glm::vec2 start_point(0.0f, 0.0f);

	while (window.isOpen())
	{
		sf::Vector2i local_position = sf::Mouse::getPosition(window);
		glm::vec2 mouse_position(local_position.x, local_position.y);

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
					qt.addElement(local_position.x, local_position.y);
				}
				else
				{
					start_point = glm::vec2(local_position.x, local_position.y);
				}
			}
		}

		glm::vec2 ray = glm::normalize(mouse_position - start_point);
		HitPoint2D hit_point = qt.castRay(start_point, ray);

		sf::VertexArray ray_va(sf::Lines, 2);
		ray_va[0].color = sf::Color::Green;
		ray_va[1].color = sf::Color::Green;
		
		ray_va[0].position = sf::Vector2f(start_point.x, start_point.y);
		if (hit_point.hit)
		{
			ray_va[1].position = sf::Vector2f(hit_point.coords.x, hit_point.coords.y);
		}
		else
		{
			ray_va[1].position = sf::Vector2f(mouse_position.x, mouse_position.y);
		}

		window.clear();

		window.draw(ray_va);
		qt.draw(&window);
		
		window.display();
	}

	return 0;
}