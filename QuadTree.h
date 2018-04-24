#pragma once

#include <array>
#include <glm/glm.hpp>

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

class QuadTree
{
public:
	QuadTree();

	HitPoint2D castRay(const glm::vec2& start, const glm::vec2& ray);

private:
	std::array<QuadTree, 4> m_subs;
};