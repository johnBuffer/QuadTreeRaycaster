#include "FlatQuadTree.h"
#include <iostream>
#include <sstream>
#include <list>

FlatQuadTree::FlatQuadTree() :
	m_size(1024),
	m_min_size(8)
{
	m_elements.emplace_back(0);

	m_font.loadFromFile("font.ttf");
}

void FlatQuadTree::print()
{
	print_element(0, "");
}

void FlatQuadTree::draw(sf::RenderTarget* render_target) const
{
	draw_element(render_target, 0, 0, 0, m_size);
}

std::vector<HitPoint2D> FlatQuadTree::castRay(const glm::vec2& start, const glm::vec2& ray_vector)
{
	sf::Clock clock;
	clock.restart();
	int iter_counter = 0;
	std::vector<HitPoint2D> result;

	if (m_elements.empty())
		return result;
	
	// Initialization of global parameters
	float inv_ray_x = 1 / ray_vector.x;
	float inv_ray_y = 1 / ray_vector.y;
	int step_x = ray_vector.x > 0 ? 1 : -1;
	int step_y = ray_vector.y > 0 ? 1 : -1;
	int dir_x = step_x > 0 ? 1 : 0;
	int dir_y = step_y > 0 ? 1 : 0;
	float t_dx = std::abs(m_size / 2 * inv_ray_x);
	float t_dy = std::abs(m_size / 2 * inv_ray_y);

	std::vector<QuadContext> stack;
	stack.reserve(32);
	// index, sub_size, start.x, start.y
	stack.emplace_back(0, m_size/2, 0, start.x, start.y);
	stack.back().initialize(dir_x, dir_y, t_dx, t_dy, inv_ray_x, inv_ray_y);

	// Probable condition: hit or stack.is_empty()
	while (true) {
		++iter_counter;
		// Current context (location, index, sub_index, ...)
		QuadContext& context = stack.back();
		const QuadElement& current_elem = m_elements[context.index];
		int current_size = context.scale;
		int current_x = context.x;
		int current_y = context.y;
		int sub_index = context.sub_index;
		int sub_y_coord = sub_index>>1;
		int sub_x_coord = sub_index - (sub_y_coord<<1);

		if (current_elem.subs[sub_index] == -1 || context.advance) {
			if (context.advance) { context.advance = false; }

			context.t_max_min = 0;
			if (context.t_max_x < context.t_max_y) {
				context.t_max_min = context.t_max_x;
				sub_x_coord += step_x;
				context.t_max_x += context.t_dx;
			}
			else {
				context.t_max_min = context.t_max_y;
				sub_y_coord += step_y;
				context.t_max_y += context.t_dy;
			}

			context.current_x = context.x + context.t_max_min*ray_vector.x;
			context.current_y = context.y + context.t_max_min*ray_vector.y;

			if (sub_x_coord > -1 && sub_x_coord < 2 && sub_y_coord > -1 && sub_y_coord < 2) {
				context.sub_index = sub_x_coord + (sub_y_coord<<1);
			}
			else {
				stack.pop_back();
				if (stack.empty())
					break;
				else
					stack.back().advance = true;
			}
		}
		// If not, go deeper
		else {
			const QuadElement& sub_element = m_elements[current_elem.subs[sub_index]];
			if (sub_element.is_leaf && !sub_element.is_empty) {
				int hit_abs_x = context.abs_x + context.t_max_min*ray_vector.x;
				int hit_abs_y = context.abs_y + context.t_max_min*ray_vector.y;

				result.emplace_back(hit_abs_x, hit_abs_y, true);
				break;
			}
			else {
				int new_index = current_elem.subs[sub_index];
				QuadContext new_context(new_index, current_size >> 1, 0, context.current_x, context.current_y);
				// Translating relative coords into sub context
				new_context.x -= sub_x_coord * current_size;
				new_context.y -= sub_y_coord * current_size;
				// Initializing sub raycast 
				new_context.initialize(dir_x, dir_y, context.t_dx * 0.5f, context.t_dy * 0.5f, inv_ray_x, inv_ray_y);
				new_context.abs_x = context.abs_x + context.t_max_min*ray_vector.x;
				new_context.abs_y = context.abs_y + context.t_max_min*ray_vector.y;
				stack.emplace_back(new_context);
			}
		}
	}

	float cast_time = clock.getElapsedTime().asMicroseconds();
	std::cout << "Iteration count: " << iter_counter << " in " << cast_time << "us" << std::endl;

	return result;
}

void FlatQuadTree::addElement(int x, int y)
{
	int current_x = x;
	int current_y = y;
	int current_size = m_size;

	int current_index = 0;

	while (current_size >= m_min_size)
	{
		current_size /= 2;

		int sub_x = current_x / current_size;
		int sub_y = current_y / current_size;
		int sub_index = sub_x + 2*sub_y;

		if (sub_x > 0)
			current_x -= current_size;
		if (sub_y > 0)
			current_y -= current_size;

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

	//std::cout << std::endl;

	//std::cout << "\n===================== ADD =====================" << std::endl;

	// Debug
	/*std::cout << "Done, new size: " << m_elements.size() << std::endl;
	int grid_size = (m_size / m_min_size)*(m_size / m_min_size);
	float space_used = m_elements.size() / float(grid_size);
	std::cout << "Spaced used: " << int(space_used * 100) << "%" << std::endl;*/
}

void FlatQuadTree::printStack(const std::vector<QuadContext>& stack) const
{
	for (const QuadContext& qc : stack)
	{
		std::cout << "Index: " << qc.index << " (size: " << qc.scale << "), Sub_index: " << qc.sub_index << std::endl;
	}
}

void FlatQuadTree::print_element(int index, const std::string& indent) const
{
	const QuadElement& elem = m_elements[index];

	std::cout << indent << "ID: " << index << std::endl;
	for (int i(0); i < 4; ++i)
	{
		int sub_idx = elem.subs[i];
		if (sub_idx != -1)
		{
			std::cout << indent << "sub " << i << ": " << sub_idx << std::endl;
			print_element(sub_idx, indent + "    ");
		}
	}
}

void FlatQuadTree::draw_element(sf::RenderTarget* render_target, int index, float x_start, float y_start, float size) const
{
	if (index == -1)
		return;

	sf::Text text;
	text.setFont(m_font);
	text.setColor(sf::Color::Red);
	text.setCharacterSize(14);

	std::stringstream sx;
	sx << index;
	text.setString(sx.str());
	text.setPosition(x_start + size / 2.0f, y_start + size / 2.0f);

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

		//render_target->draw(va);

		draw_element(render_target, elem.subs[0], x_start, y_start, sub_size);
		draw_element(render_target, elem.subs[1], x_start + sub_size, y_start, sub_size);
		draw_element(render_target, elem.subs[2], x_start, y_start + sub_size, sub_size);
		draw_element(render_target, elem.subs[3], x_start + sub_size, y_start + sub_size, sub_size);
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

	//render_target->draw(text);
}
