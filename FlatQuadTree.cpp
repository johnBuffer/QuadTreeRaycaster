#include "FlatQuadTree.h"
#include <iostream>

FlatQuadTree::FlatQuadTree() :
	m_size(1024),
	m_min_size(16)
{
	m_elements.emplace_back(0);
}

void FlatQuadTree::draw(sf::RenderTarget* render_target) const
{
	draw_element(render_target, 0, 0, 0, m_size);
}

HitPoint2D FlatQuadTree::castRay(const glm::vec2& start, const glm::vec2& ray)
{
	return HitPoint2D();
}

void FlatQuadTree::addElement(int x, int y)
{
	int current_x = x;
	int current_y = y;
	int current_size = m_size;

	int current_index = 0;

	while (current_size >= m_min_size)
	{
		current_size >>= 1;

		int sub_index = -1;
		if (current_x < current_size)
		{
			if (current_y < current_size)
			{
				sub_index = 0;
			}
			else
			{
				current_y -= current_size;
				sub_index = 3;
			}
		}
		else
		{
			current_x -= current_size;
			if (current_y < current_size)
			{
				sub_index = 1;
			}
			else
			{
				current_y -= current_size;
				sub_index = 2;
			}
		}

		if (m_elements[current_index].subs[sub_index] == -1)
		{
			int new_index = m_elements.size();
			m_elements[current_index].subs[sub_index] = new_index;
			m_elements.emplace_back(new_index);
			m_elements.back().parent = current_index;
		}

		QuadElement& elem = m_elements[current_index];
		elem.is_leaf = false;
		elem.is_empty = false;

		current_index = elem.subs[sub_index];
	}

	m_elements[current_index].is_empty = false;

	// Debug
	std::cout << "Done, new size: " << m_elements.size() << std::endl;
	int grid_size = (m_size / m_min_size)*(m_size / m_min_size);
	float space_used = m_elements.size() / float(grid_size);
	std::cout << "Spaced used: " << int(space_used * 100) << "%" << std::endl;
}

void FlatQuadTree::draw_element(sf::RenderTarget* render_target, int index, float x_start, float y_start, float size) const
{
	if (index == -1)
		return;

	const QuadElement& elem = m_elements[index];
	if (!elem.is_leaf)
	{
		sf::VertexArray va(sf::Lines, 4);
		for (int i(0); i<4; ++i)
			va[i].color = sf::Color::White;

		float sub_size = size / 2.0f;
	
		va[0].position = sf::Vector2f(x_start + sub_size, y_start);
		va[1].position = sf::Vector2f(x_start + sub_size, y_start + size);

		va[2].position = sf::Vector2f(x_start       , y_start + sub_size);
		va[3].position = sf::Vector2f(x_start + size, y_start + sub_size);

		render_target->draw(va);

		draw_element(render_target, elem.subs[0], x_start, y_start, sub_size);
		draw_element(render_target, elem.subs[1], x_start + sub_size, y_start, sub_size);
		draw_element(render_target, elem.subs[2], x_start + sub_size, y_start + sub_size, sub_size);
		draw_element(render_target, elem.subs[3], x_start, y_start + sub_size, sub_size);
	}
	else// if (!elem.is_empty)
	{
		sf::VertexArray va(sf::Quads, 4);
		for (int i(0); i<4; ++i)
			va[i].color = sf::Color::Green;

		va[0].position = sf::Vector2f(x_start       , y_start);
		va[1].position = sf::Vector2f(x_start + size, y_start);

		va[2].position = sf::Vector2f(x_start + size, y_start + size);
		va[3].position = sf::Vector2f(x_start       , y_start + size);

		render_target->draw(va);
	}
}
