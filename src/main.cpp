#include <iostream>
#include "FlatQuadTree.h"

int main()
{
	int win_width = 512 * 2;
	int win_height = 512 * 2;

	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "QR");

	FlatQuadTree qt;
	/*for (int x(0); x < 1024; ++x)
	{
		for (int y(0); y < 1024; ++y)
		{
			if (y < 304 || y > 310)
				qt.addElement(x, y);
		}
	}*/

	for (int i(1000); i--;)
		qt.addElement(rand() % 1024, rand() % 1024);

	bool mouseButtonPressed = false;

	bool first_point_set = false;

	glm::vec2 start_point(0, 0);
	glm::vec2 end_point(0, 0);

	std::vector<HitPoint2D> hit_points;

	//std::cout << (1 + (1 << 1)) << std::endl;

	while (window.isOpen())
	{
		sf::Vector2i local_position = sf::Mouse::getPosition(window);
		glm::vec2 mouse_position(local_position.x, local_position.y);

		glm::vec2 ray;
		if (first_point_set)
		{
			//sf::Mouse::setPosition(sf::Vector2i(start_point.x + 2, start_point.y + 2), window);
			end_point.x = local_position.x;
			end_point.y = local_position.y;

			ray = end_point - start_point;

			if (glm::length(ray) > 50.0f && std::abs(ray.x)>0.01 && std::abs(ray.y)>0.01)
			{
				ray = glm::normalize(ray);
				hit_points = qt.castRay(start_point, ray);
			}
		}

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
					mouseButtonPressed = false;
					
					if (!first_point_set)
					{
						start_point.x = local_position.x;
						start_point.y = local_position.y;

						first_point_set = true;
					}
					else
					{
						first_point_set = false;
						end_point.x = local_position.x;
						end_point.y = local_position.y;

						ray = glm::normalize(end_point - start_point);
						hit_points = qt.castRay(start_point, ray);
						
						std::cout << "Hit points found: " << hit_points.size() << std::endl;
					}

				}
				else
				{
					mouseButtonPressed = true;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				mouseButtonPressed = false;
			}
		}

		if (mouseButtonPressed)
		{
			qt.addElement(local_position.x, local_position.y);
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
		for (HitPoint2D& pt : hit_points)
		{
			if (pt.hit)
				hit_marker.setFillColor(sf::Color::Red);
			else
				hit_marker.setFillColor(sf::Color::Cyan);

			hit_marker.setPosition(pt.coords.x, pt.coords.y);
			window.draw(hit_marker);
		}
		
		window.display();
	}

	return 0;
}