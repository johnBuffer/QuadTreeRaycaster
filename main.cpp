#include <iostream>
#include "FlatQuadTree.h"

int main()
{
	int win_width = 512*2;
	int win_height = 512*2;

	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "QR");

	FlatQuadTree qt;
	qt.addElement(20, 50);

	bool mouseButtonPressed = false;

	glm::vec2 start_point(20.0f, 100.0f);
	glm::vec2 end_point(400.0f, 250.0f);

	std::vector<HitPoint2D> hit_points;
	glm::vec2 ray = glm::normalize(end_point - start_point);

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

					hit_points = qt.castRay(start_point, ray);
					std::cout << "Hit points found: " << hit_points.size() << std::endl;
				}
				else
				{

				}
			}
		}

		sf::VertexArray ray_va(sf::Lines, 2);
		ray_va[0].color = sf::Color::Green;
		ray_va[1].color = sf::Color::Green;
		ray_va[0].position = sf::Vector2f(start_point.x, start_point.y);
		ray_va[1].position = sf::Vector2f(end_point.x, end_point.y);

		window.clear();

		window.draw(ray_va);
		qt.draw(&window);

		sf::RectangleShape hit_marker(sf::Vector2f(7.0f, 7.0f));
		hit_marker.setOrigin(3.0f, 3.0f);
		hit_marker.setFillColor(sf::Color::Red);
		for (HitPoint2D& pt : hit_points)
		{
			hit_marker.setPosition(pt.coords.x, pt.coords.y);
			window.draw(hit_marker);
		}
		
		window.display();
	}

	return 0;
}