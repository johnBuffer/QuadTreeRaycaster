#include "FlatQuadTree.h"
#include <iostream>
#include <sstream>
#include <list>
#include <bitset>


FlatQuadTree::FlatQuadTree() :
	m_size(10),
	m_min_size(5)
{
	m_elements.emplace_back(0);
	int new_index = m_elements.size();
	m_elements[0].subs = new_index;
	for (int i(4); i--;)
		m_elements.emplace_back();

	m_font.loadFromFile("font.ttf");
}

void FlatQuadTree::print()
{
	print_element(0, "");
}

void FlatQuadTree::draw(sf::RenderTarget* render_target) const
{
	draw_element(render_target, 0, 0, 0, 1<<m_size);
}

HitPoint2D FlatQuadTree::castRay(const glm::vec2& start, const glm::vec2& ray_vector)
{
	//std::cout << "\n========== START ==========" << std::endl;
	sf::Clock clock;
	clock.restart();
	int iter_counter = 0;

	HitPoint2D result;

	if (m_elements.empty())
		return result;

	// We assume we have a ray vector:
	// vec = start + t*v
	
	// Initialization of global parameters
	float inv_ray_x = 1 / ray_vector.x;
	float inv_ray_y = 1 / ray_vector.y;
	int step_x = ray_vector.x > 0 ? 1 : -1;
	int step_y = ray_vector.y > 0 ? 1 : -1;
	int dir_x = step_x > 0 ? 1 : 0;
	int dir_y = step_y > 0 ? 1 : 0;
	float t_dx = std::abs((1 << (m_size - 1)) * inv_ray_x);
	float t_dy = std::abs((1 << (m_size - 1)) * inv_ray_y);

	int current_stack_index = 0;
	QuadContext stack[10];
	// index, sub_size, start.x, start.y
	stack[0] = QuadContext(0, m_size-1, 0, start.x, start.y);
	stack[0].initialize(dir_x, dir_y, t_dx, t_dy, inv_ray_x, inv_ray_y);

	// Probable condition: hit or stack.is_empty()
	/*while (true)
	{
		++iter_counter;
		// Current context (location, index, sub_index, ...)
		QuadContext& context = stack[current_stack_index];
		const QuadElement& current_elem = m_elements[context.index];
		int current_scale = context.scale;
		int current_size = 1 << current_scale;
		int current_x = context.x;
		int current_y = context.y;
		int sub_index = context.sub_index;
		int sub_y_coord = sub_index>>1;
		int sub_x_coord = sub_index - (sub_y_coord<<1);

		// If current sub empty -> move to next one
		if (current_elem.subs[sub_index] == -1 || context.advance)
		{
			if (context.advance) { context.advance = false; }

			context.t_max_min = 0;
			if (context.t_max_x < context.t_max_y)
			{
				context.t_max_min = context.t_max_x;
				sub_x_coord += step_x;
				context.t_max_x += context.t_dx;
			}
			else
			{
				context.t_max_min = context.t_max_y;
				sub_y_coord += step_y;
				context.t_max_y += context.t_dy;
			}

			context.current_x = context.x + context.t_max_min*ray_vector.x;
			context.current_y = context.y + context.t_max_min*ray_vector.y;

			if (sub_x_coord > -1 && sub_x_coord < 2 && sub_y_coord > -1 && sub_y_coord < 2)
			{
				context.sub_index = sub_x_coord + (sub_y_coord<<1);
			}
			else
			{
				--current_stack_index;
				if (current_stack_index<0)
					break;
				
				stack[current_stack_index].advance = true;
			}
		}
		// If not, go deeper
		else
		{
			const QuadElement& sub_element = m_elements[current_elem.subs[sub_index]];
			if (sub_element.is_leaf && !sub_element.is_empty)
			{
				int hit_abs_x = context.abs_x + context.t_max_min*ray_vector.x;
				int hit_abs_y = context.abs_y + context.t_max_min*ray_vector.y;

				result.coords.x = hit_abs_x;
				result.coords.y = hit_abs_y;
				result.hit = true;
				break;
			}
			else
			{
				// Adding sub context to the stack
				++current_stack_index;
				int new_index = current_elem.subs[sub_index];

				new(&stack[current_stack_index]) QuadContext(new_index, current_scale-1, 0, context.current_x, context.current_y);
				QuadContext& new_context = stack[current_stack_index];

				// Translating relative coords into sub context
				new_context.x -= sub_x_coord * current_size;
				new_context.y -= sub_y_coord * current_size;

				// Initializing sub raycast 
				new_context.initialize(dir_x, dir_y, context.t_dx * 0.5f, context.t_dy * 0.5f, inv_ray_x, inv_ray_y);
				new_context.abs_x = context.abs_x + context.t_max_min*ray_vector.x;
				new_context.abs_y = context.abs_y + context.t_max_min*ray_vector.y;
			}
		}
	}*/

	float cast_time = clock.getElapsedTime().asMicroseconds();
	std::cout << "Iteration count: " << iter_counter << " in " << cast_time << "us" << std::endl;

	return result;
}

void FlatQuadTree::addElement(int x, int y)
{
	int current_x = x;
	int current_y = y;
	int current_size = 1<<m_size;
	int current_index = 0;

	int last_index = -1;
	int last_sub;

	while (current_size >= (1<<m_min_size))
	{
		//std::cout << "Adding in " << current_index << std::endl;
		QuadElement& current_element = m_elements[current_index];
		current_size /= 2;

		int sub_x = current_x / current_size;
		int sub_y = current_y / current_size;
		int sub_index = sub_x + 2*sub_y;

		current_x -= current_size * sub_x;
		current_y -= current_size * sub_y;

		unsigned short current_mask = current_element.subs_mask << (sub_index);

		//std::cout << "Current sub mask " << std::bitset<16>(current_element.subs_mask) << " gives " << std::bitset<16>(current_mask) << " " << sub_index << std::endl;
		//current_mask <<= sub_index;
		//std::cout << "Condition mask " << std::bitset<16>(0x8000) << " -> " << std::bitset<16>(current_mask & 0x8000) << std::endl;

		if (!(current_mask & 0x8000))
		{
			// Index for new element's subs
			int new_index = m_elements.size();
			//std::cout << "Creating sub " << sub_index << " for " << current_index << " (ID: " << new_index << ")" << std::endl;
			m_elements[current_element.subs + sub_index].subs = new_index;

			// Update current sub status
			current_element.subs_mask |= (0x8000 >> sub_index);

			// Update parent's leaf status
			if (last_index != -1)
			{
				QuadElement& last_element = m_elements[last_index];
				last_element.subs_mask &= ~(0x80 >> last_sub);

				if (!last_index)
				{
					std::cout << "Root's leaf mask " << std::bitset<16>(~(0x80 >> last_sub)) << " " << last_sub << std::endl;
					//std::cout << "Root's mask " << std::bitset<16>(last_element.subs_mask) << std::endl;
				}
			}

			// Allocate new subs
			for (int i(4); i--;)
			{
				m_elements.emplace_back();
			}

			//std::cout << "New tree size " << m_elements.size() << std::endl;
		}
		else
		{
			//std::cout << "Sub " << sub_index << " already existing in " << current_index << std::endl;
		}

		last_index = current_index;
		last_sub = sub_index;
		
		current_index = m_elements[current_index].subs + sub_index;

		//std::cout << std::endl;
	}

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
		unsigned int current_mask = elem.subs_mask << i;

		if (current_mask & 0x8000)
		{
			std::cout << indent << "sub " << i << ": " << elem.subs+i << std::endl;
			print_element(elem.subs + i, indent + "    ");
		}
	}
}

void FlatQuadTree::draw_element(sf::RenderTarget* render_target, int index, float x_start, float y_start, float size) const
{
	float sub_size = size / 2.0f;

	sf::Text text;
	text.setFont(m_font);
	text.setColor(sf::Color::Red);
	text.setCharacterSize(14);

	std::stringstream sx;
	sx << index;
	text.setString(sx.str());
	text.setPosition(x_start + sub_size, y_start + sub_size);

	const QuadElement& elem = m_elements[index];
	
	sf::VertexArray va(sf::Lines, 4);
	for (int i(0); i<4; ++i)
		va[i].color = sf::Color::White;

	
	va[0].position = sf::Vector2f(x_start + sub_size, y_start);
	va[1].position = sf::Vector2f(x_start + sub_size, y_start + size);

	va[2].position = sf::Vector2f(x_start       , y_start + sub_size);
	va[3].position = sf::Vector2f(x_start + size, y_start + sub_size);

	render_target->draw(va);

	for (int x(0); x < 2; ++x)
	{
		for (int y(0); y < 2; ++y)
		{
			int sub_index = x + 2 * y;
			if ((elem.subs_mask << sub_index) & 0x8000)
			{
				if (!((elem.subs_mask << sub_index) & 0x80))
					draw_element(render_target, elem.subs+sub_index, x_start+sub_size * x, y_start + sub_size * y, sub_size);
				else
				{
					sf::VertexArray va_leaf(sf::Quads, 4);
					for (int i(0); i<4; ++i)
						va_leaf[i].color = sf::Color::Green;

					va_leaf[0].position = sf::Vector2f(x_start + size / 2 * x, y_start + size / 2 * y);
					va_leaf[1].position = sf::Vector2f(x_start + size / 2 * (x+1), y_start + size / 2 * y);

					va_leaf[2].position = sf::Vector2f(x_start + size / 2 * (x + 1), y_start + size / 2 * (y+1));
					va_leaf[3].position = sf::Vector2f(x_start + size / 2 * x, y_start + size / 2 * (y+1));

					render_target->draw(va_leaf);
				}
			}
		}
	}

	//render_target->draw(text);
}
