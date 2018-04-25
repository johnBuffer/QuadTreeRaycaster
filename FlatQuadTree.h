#pragma once

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>

struct HitPoint2D
{
	HitPoint2D() : coords(0.0, 0.0), hit(false) {}
	HitPoint2D(float x, float y, bool has_hit) :
		coords(x, y),
		hit(has_hit)
	{}

	glm::vec2 coords;
	bool hit;
};

// One of the FlatQuadTree element
struct QuadElement
{
	QuadElement() : parent(-1), is_leaf(true), is_empty(true)
	{
		for (int i(0); i<4; ++i)
		{
			subs[i] = -1;
		}
	}

	QuadElement(int idx) : QuadElement()
	{
		index = idx;
	}

	// Current index in vector
	int index;
	// Index of parent
	int parent;
	// Indexes of subs
	std::array<int, 4> subs;
	// Is it a leaf ?
	bool is_leaf;
	// Is it empty ?
	bool is_empty;
};

struct QuadContext
{
	QuadContext() = default;

	QuadContext(int idx, int scl, int sb_idx, int px, int py) :
		index(idx),
		scale(scl),
		sub_index(sb_idx),
		x(px),
		y(py)
	{}

	int getCurrentSub(int px, int py)
	{
		int sub_index = 0;
		int size = scale >> 1;
		if (px < scale)
		{
			if (py < size)
				sub_index = 0;
			else
				sub_index = 2;
		}
		else
		{
			px -= size;
			if (py < size)
				sub_index = 1;
			else
				sub_index = 3;
		}

		return sub_index;
	}
	
	int parent_index;
	int index;
	int scale;
	int sub_index;
	int x, y;
	int abs_x, abs_y;

	float t_max_x, t_max_y;
	float t_dx, t_dy;
};

class FlatQuadTree
{
public:
	FlatQuadTree();
	~FlatQuadTree() = default;

	void draw(sf::RenderTarget* render_target) const;

	std::vector<HitPoint2D> castRay(const glm::vec2& start, const glm::vec2& ray_vector);

	void addElement(int x, int y);

private:
	int m_min_size;
	int m_size;
	std::vector<QuadElement> m_elements;

	void printStack(const std::vector<QuadContext>& stack) const;

	std::vector<QuadContext> getCurrentContext(int x, int y) const;
	QuadContext updateContext(int x, int y, QuadContext& current_context);

	void draw_element(sf::RenderTarget* render_target, int index, float x_start, float y_start, float size) const;
};
