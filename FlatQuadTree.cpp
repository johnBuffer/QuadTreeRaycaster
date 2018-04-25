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

std::vector<HitPoint2D> FlatQuadTree::castRay(const glm::vec2& start, const glm::vec2& ray_vector)
{
	std::vector<HitPoint2D> result;

	if (m_elements.empty())
		return result;

	// Initialization
	// We assume we have a ray vector:
	// vec = start + t*v
	float inv_ray_x = 1 / ray_vector.x;
	float inv_ray_y = 1 / ray_vector.y;

	std::vector<QuadContext> stack(0);
	stack.emplace_back(0, m_size, 0, start.x, start.y);
	QuadContext& root_context = stack.back();
	root_context.abs_x = start.x;
	root_context.abs_y = start.y;

	int step_x = ray_vector.x > 0 ? 1 : -1;
	int step_y = ray_vector.y > 0 ? 1 : -1;

	// Compute the value of t for first intersection in x and y
	int dir_x = step_x > 0 ? 1 : 0;
	int dir_y = step_y > 0 ? 1 : 0;

	// Compute how much (in units of t) we can move along the ray
	// before reaching the cell's width and height
	root_context.t_dx = std::abs(float(m_size/2) * inv_ray_x);
	root_context.t_dy = std::abs(float(m_size/2) * inv_ray_y);

	root_context.t_max_x = ((dir_x)*m_size/2 - root_context.x) * inv_ray_x;
	root_context.t_max_y = ((dir_y)*m_size/2 - root_context.y) * inv_ray_y;

	// Probable condition: hit or stack.is_empty()
	while (true)
	{
		// Current context (location, index, sub_index, ...)
		QuadContext& context = stack.back();
		int current_x = context.x;
		int current_y = context.y;

		// Check for nested voxels
		QuadContext sub_context = updateContext(current_x, current_y, context);
		int current_size = sub_context.scale;

		/*std::cout << "Parent index: " << context.index << std::endl;
		std::cout << "Parent sub: " << context.sub_index << std::endl;
		std::cout << "Current index: " << sub_context.index << std::endl;
		std::cout << "At sub: " << sub_context.sub_index << " pos: [" << sub_context.x << ", " << sub_context.y << "]" << std::endl;*/

		printStack(stack);

		// Empty sub ?
		if (sub_context.index == context.index)
		{
			std::cout << context.index << " sub_index " << context.sub_index << " is empty, moving..." << std::endl;

			// Move one the next context's sub
			float t_max_x = ((dir_x)*current_size - sub_context.x) * inv_ray_x;
			float t_max_y = ((dir_y)*current_size - sub_context.y) * inv_ray_y;

			int min_type = 0;
			float t_max_min = 0;
			if (t_max_x < t_max_y)
			{
				min_type = step_x;
				t_max_min = t_max_x;
				context.t_max_x += context.t_dx;
			}
			else
			{
				min_type = 2 * step_y;
				t_max_min = t_max_y;
				context.t_max_y += context.t_dy;
			}

			// Move on to the next sub
			int next_sub = context.sub_index + min_type;
			std::cout << "Empty, next sub: " << next_sub << " (min type: " << min_type << ")" << std::endl;

			context.abs_x += t_max_min * ray_vector.x;
			context.abs_y += t_max_min * ray_vector.y;

			result.emplace_back(context.abs_x, context.abs_y, true);

			if (next_sub > -1 && next_sub < 4)
			{
				context.sub_index = next_sub;
				if (min_type == step_x)
				{
					context.x = 0;
					context.y = current_y + t_max_min * ray_vector.y + 2;
				}
				else
				{
					context.x = current_x + t_max_min * ray_vector.x + 2;
					context.y = 0;
				}
			}
			else
			{
				std::cout << "Exiting " << context.index << "..." << std::endl;
				stack.pop_back();
				
				QuadContext& previous_context = stack.back();
				int min_type = 0;
				float t_max_min = 0;
				if (previous_context.t_max_x < previous_context.t_max_y)
				{
					min_type = step_x;
					t_max_min = previous_context.t_max_x;
					previous_context.t_max_x += previous_context.t_dx;
				}
				else
				{
					min_type = 2 * step_y;
					t_max_min = previous_context.t_max_y;
					previous_context.t_max_y += previous_context.t_dy;
				}

				context.x += t_max_min * ray_vector.x;
				context.y += t_max_min * ray_vector.y;
			}
		}
		// Not empty sub
		else
		{
			// Enter the sub voxel
			stack.emplace_back(sub_context);
			QuadContext& new_context = stack.back();

			new_context.t_dx = context.t_dx*0.5f;
			new_context.t_dy = context.t_dy*0.5f;

			new_context.t_max_x = ((dir_x)*current_size/2 - new_context.x) * inv_ray_x;
			new_context.t_max_y = ((dir_y)*current_size/2 - new_context.y) * inv_ray_y;
		}

		std::cout << std::endl;
	}

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
		}

		if (m_elements[current_index].subs[sub_index] == -1)
		{
			std::cout << "Creating sub " << sub_index << " for " << current_index << std::endl;
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

	std::cout << std::endl;

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
		std::cout << "Index: " << qc.index << "(size: " << qc.scale << "), Sub_index: " << qc.sub_index << std::endl;
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
}
