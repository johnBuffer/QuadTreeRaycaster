#pragma once

#include <list>
#include <array>
#include <vector>
#include <iostream>
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
			subs[i] = -1;
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
	int subs[4];
	// Is it a leaf ?
	bool is_leaf;
	// Is it empty ?
	bool is_empty;
};

struct QuadContext
{
	QuadContext() = default;

	QuadContext(int idx, int scl, int sb_idx, int px, int py) :
		advance(false),
		index(idx),
		scale(scl),
		sub_index(sb_idx),
		x(px),
		y(py),
		current_x(px),
		current_y(py)
	{}

	void initialize(int dir_x, int dir_y, int t_delta_x, int t_delta_y, float inv_ray_x, float inv_ray_y)
	{
		// Absolute hit's X and Y -> this init is just usefull for first context ( coord == abs_coord )
		abs_x = x;
		abs_y = y;

		current_x = x;
		current_y = y;

		// Compute sub_index
		int sub_x_coord = x / (scale+1);
		int sub_y_coord = y / (scale+1);
		sub_index = sub_x_coord + (sub_y_coord<<1);

		// Compute how much (in units of t) we can move along the ray
		// before reaching the cell's width and height
		t_dx = t_delta_x;
		t_dy = t_delta_y;

		// Compute the value of t for first intersection in x and y
		t_max_min = 0;

		// Compute the value of t for first intersection in x and y
		//std::cout << "Computing t_max, t_dx: " << t_dx << " t_dy: " << t_dy << " inv_ray_x " << inv_ray_x << " inv_ray_y " << inv_ray_y << std::endl;
		t_max_x = ((dir_x + sub_x_coord)*scale - x) * inv_ray_x;
		t_max_y = ((dir_y + sub_y_coord)*scale - y) * inv_ray_y;
	}

	int getCurrentSub(int px, int py)
	{
		return px/scale + 2*(py/scale);
	}
	
	bool advance;
	int index;
	int scale;
	int sub_index;
	int x, y, current_x, current_y;
	int abs_x, abs_y;

	float t_max_x, t_max_y, t_max_min;
	float t_dx, t_dy;
};

class FlatQuadTree
{
public:
	FlatQuadTree();
	~FlatQuadTree() = default;

	void print();
	void draw(sf::RenderTarget* render_target) const;

	HitPoint2D castRay(const glm::vec2& start, const glm::vec2& ray_vector);

	void addElement(int x, int y);

private:
	int m_min_size;
	int m_size;
	std::vector<QuadElement> m_elements;

	sf::Font m_font;

	void printStack(const std::vector<QuadContext>& stack) const;

	void print_element(int index, const std::string& indent) const;
	void draw_element(sf::RenderTarget* render_target, int index, float x_start, float y_start, float size) const;
};
