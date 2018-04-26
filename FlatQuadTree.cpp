#include "FlatQuadTree.h"
#include <iostream>
#include <sstream>
#include <list>

FlatQuadTree::FlatQuadTree() :
	m_size(1024),
	m_min_size(4)
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
	int iter_counter = 0;
	std::vector<HitPoint2D> result;

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

	std::list<QuadContext> stack(0);
	// index, sub_size, start.x, start.y
	stack.emplace_back(0, m_size/2, 0, start.x, start.y);
	QuadContext& root_context = stack.back();
	root_context.abs_x = start.x;
	root_context.abs_y = start.y;
	root_context.t_max_min = 0;

	root_context.sub_index = root_context.getCurrentSub(root_context.x, root_context.y);

	// Compute how much (in units of t) we can move along the ray
	// before reaching the cell's width and height
	root_context.t_dx = std::abs(root_context.scale * inv_ray_x);
	root_context.t_dy = std::abs(root_context.scale * inv_ray_y);

	// Compute the value of t for first intersection in x and y
	root_context.t_max_x = (dir_x*root_context.scale - root_context.x) * inv_ray_x;
	root_context.t_max_y = (dir_y*root_context.scale - root_context.y) * inv_ray_y;

	// Probable condition: hit or stack.is_empty()
	while (true)
	{
		++iter_counter;
		//printStack(stack);
		// Current context (location, index, sub_index, ...)
		QuadContext& context = stack.back();
		const QuadElement& current_elem = m_elements[context.index];
		int current_x = context.x;
		int current_y = context.y;
		int sub_index = context.sub_index;
		int current_size = context.scale;

		//std::cout << "In " << context.index << std::endl;

		// If current sub empty -> move to next one
		if (current_elem.subs[sub_index] == -1 || context.advance)
		{
			if (context.advance)
			{
				//std::cout << "Sub " << context.sub_index << " in " << context.index << " already explored, skipping " << std::endl;
				context.advance = false;
			}
			else
			{
				//std::cout << "Sub " << context.sub_index << " in " << context.index << " is empty, skipping " << std::endl;
			}

			int sub_y_coord = context.sub_index / 2;
			int sub_x_coord = context.sub_index - 2*sub_y_coord;

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

			//std::cout << "Hit, new rel coords: " << context.x + context.t_max_min*ray_vector.x << " " << context.y + context.t_max_min*ray_vector.y << std::endl;
			//std::cout << "New t_max coords: " << context.t_max_x << " " << context.t_max_y << std::endl;

			float hit_abs_x = context.abs_x + context.t_max_min*ray_vector.x;
			float hit_abs_y = context.abs_y + context.t_max_min*ray_vector.y;

			//std::cout << "Hit, new abs coords: " << hit_abs_x << " " << hit_abs_y << std::endl;
			result.emplace_back(hit_abs_x, hit_abs_y, false);

			if (sub_x_coord > -1 && sub_x_coord < 2 && sub_y_coord > -1 && sub_y_coord < 2)
			{
				context.sub_index = sub_x_coord + 2*sub_y_coord;
			}
			else
			{
				//std::cout << "Exiting " << context.index << std::endl;
				stack.pop_back();

				if (stack.empty())
					break;
				else
					stack.back().advance = true;
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

				result.emplace_back(hit_abs_x, hit_abs_y, true);
				break;
			}
			else
			{
				//std::cout << "Sub " << context.sub_index << " in " << context.index << " has sub, entering " << current_elem.subs[sub_index] << std::endl;

				// Adding sub context to the stack
				int new_index = current_elem.subs[sub_index];

				stack.emplace_back();
				QuadContext& new_context = stack.back();
				new_context.index = new_index;
				new_context.scale = current_size / 2;

				new_context.x = context.current_x;
				new_context.y = context.current_y;

				// Translating relative coords into sub context
				if (sub_index == 1)
				{
					//std::cout << "Translating x part: " << context.current_x << " " << context.current_y << std::endl;
					new_context.x -= current_size;
				}
				else if (sub_index == 2)
				{
					//std::cout << "Translating y part: " << context.current_x << " " << context.current_y << std::endl;
					new_context.y -= current_size;
				}
				else if (sub_index == 3)
				{
					//std::cout << "Translating both part: " << context.current_x << " " << context.current_y << std::endl;
					new_context.x -= current_size;
					new_context.y -= current_size;
				}

				new_context.current_x = new_context.x;
				new_context.current_y = new_context.y;

				//std::cout << "Rel coords: " << new_context.x << " " << new_context.y << std::endl;

				// Initializing sub raycast parameters
				new_context.abs_x = context.abs_x + context.t_max_min*ray_vector.x;
				new_context.abs_y = context.abs_y + context.t_max_min*ray_vector.y;
				//std::cout << "Abs coords: " << new_context.abs_x << " " << new_context.abs_y << std::endl;

				new_context.sub_index = new_context.getCurrentSub(new_context.x, new_context.y);
				//std::cout << "New sub: " << new_context.sub_index << " scale: " << new_context.scale << std::endl;

				// Compute how much (in units of t) we can move along the ray
				// before reaching the cell's width and height
				new_context.t_dx = context.t_dx * 0.5f;
				new_context.t_dy = context.t_dy * 0.5f;

				int sub_y_coord = new_context.sub_index / 2;
				int sub_x_coord = new_context.sub_index - (sub_y_coord * 2);

				// Compute the value of t for first intersection in x and y
				new_context.t_max_x = ((dir_x + sub_x_coord)*new_context.scale - new_context.x) * inv_ray_x;
				new_context.t_max_y = ((dir_y + sub_y_coord)*new_context.scale - new_context.y) * inv_ray_y;

				//std::cout << "T_max coords: " << new_context.t_max_x << " " << new_context.t_max_y << std::endl;
			}
		}

		//std::cout << std::endl;
	}

	std::cout << "Iteration count: " << iter_counter << std::endl;

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
		
		/*if (current_x < current_size)
		{
			if (current_y < current_size)
			{
				sub_index = 0;
			}
			else
			{
				current_y -= current_size;
				sub_index = 2;
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
				sub_index = 3;
			}
		}*/

		if (m_elements[current_index].subs[sub_index] == -1)
		{
			int new_index = m_elements.size();
			//std::cout << "Creating sub " << sub_index << " for " << current_index << " (ID: " << new_index << ")" << std::endl;
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

	//std::cout << "===================== ADD =====================" << std::endl;

	// Debug
	/*std::cout << "Done, new size: " << m_elements.size() << std::endl;
	int grid_size = (m_size / m_min_size)*(m_size / m_min_size);
	float space_used = m_elements.size() / float(grid_size);
	std::cout << "Spaced used: " << int(space_used * 100) << "%" << std::endl;*/
}

void FlatQuadTree::printStack(const std::list<QuadContext>& stack) const
{
	for (const QuadContext& qc : stack)
	{
		std::cout << "Index: " << qc.index << " (size: " << qc.scale << "), Sub_index: " << qc.sub_index << std::endl;
	}
}

std::vector<QuadContext> FlatQuadTree::getCurrentContext(int x, int y) const
{
	std::vector<QuadContext> stack;

	int parent_index = -1;
	int index = 0;
	int size = m_size;
	int sub_index = 0;
	int px = x;
	int py = y;

	while (!m_elements[index].is_leaf)
	{
		size >>= 1;

		const QuadElement& elem = m_elements[index];

		if (px < size)
		{
			if (py < size)
			{
				sub_index = 0;
			}
			else
			{
				py -= size;
				sub_index = 2;
			}
		}
		else
		{
			px -= size;
			if (py < size)
			{
				sub_index = 1;
			}
			else
			{
				py -= size;
				sub_index = 3;
			}
		}

		if (elem.subs[sub_index] == -1)
		{
			break;
		}
		else
		{
			stack.emplace_back(index, size, sub_index, px, py);
			index = elem.subs[sub_index];
		}
	}

	stack.emplace_back(index, size, sub_index, px, py);
	stack.back().parent_index = parent_index;
	parent_index = index;

	return stack;
}

QuadContext FlatQuadTree::updateContext(int x, int y, QuadContext& current_context)
{
	QuadContext sub_context;
	int size = current_context.scale>>1;
	int px = x;
	int py = y;

	int sub_index = current_context.getCurrentSub(x, y);
	if (sub_index == 1)
		px -= size;
	else if (sub_index == 2)
		py -= size;
	else if (sub_index == 3)
	{
		px -= size;
		py -= size;
	}

	// Updating current context
	current_context.sub_index = sub_index;
	const QuadElement& current_elem = m_elements[current_context.index];
	int sub_elem_index = current_elem.subs[sub_index];
	if (sub_elem_index != -1)
	{
		sub_context.index = sub_elem_index;
		sub_context.parent_index = current_context.index;
		sub_context.sub_index = -1;
		sub_context.abs_x = current_context.abs_x;
		sub_context.abs_y = current_context.abs_y;
	}
	else
	{
		sub_context = current_context;
		sub_context.sub_index = sub_index;
	}

	sub_context.scale = size;
	sub_context.x = px;
	sub_context.y = py;

	return sub_context;
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

		render_target->draw(va);

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
