#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

enum class CellType
{
	None,
	Room,
	Hallway,
	Stairs
};

struct Tetrahedron
{
	Tetrahedron() = default;
	Tetrahedron(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

	bool CircumSphereContains(const glm::vec3& pos);
	bool ContainsVertex(const glm::vec3& v);
	bool operator==(const Tetrahedron& t) const;

	glm::vec3 a, b, c, d;
	glm::vec3 circumCenter;
	float circumRadius2;
};

struct DelaunayTriangle
{
	DelaunayTriangle() = default;
	DelaunayTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	bool AlmostEquals(const DelaunayTriangle& other);

	bool operator==(const DelaunayTriangle& t) const;

	glm::vec3 a, b, c;
};

struct Edge
{
	Edge() = default;
	Edge(const glm::vec3& a, const glm::vec3& b);

	bool operator==(const Edge& t) const;

	glm::vec3 a, b;
};


// Override std::hash
namespace std
{
	template<>
	struct hash<Edge>
	{
		std::size_t operator()(const Edge& edge) const
		{
			size_t seed = 0;
			hash<float> hasher;
			glm::detail::hash_combine(seed, hasher(edge.a.x));
			glm::detail::hash_combine(seed, hasher(edge.a.y));
			glm::detail::hash_combine(seed, hasher(edge.a.z));

			glm::detail::hash_combine(seed, hasher(edge.b.x));
			glm::detail::hash_combine(seed, hasher(edge.b.y));
			glm::detail::hash_combine(seed, hasher(edge.b.z));
			return seed;
		}
	};

	template<>
	struct hash<DelaunayTriangle>
	{
		std::size_t operator()(const DelaunayTriangle& t) const
		{
			size_t seed = 0;
			hash<float> hasher;
			glm::detail::hash_combine(seed, hasher(t.a.x));
			glm::detail::hash_combine(seed, hasher(t.a.y));
			glm::detail::hash_combine(seed, hasher(t.a.z));

			glm::detail::hash_combine(seed, hasher(t.b.x));
			glm::detail::hash_combine(seed, hasher(t.b.y));
			glm::detail::hash_combine(seed, hasher(t.b.z));

			glm::detail::hash_combine(seed, hasher(t.c.x));
			glm::detail::hash_combine(seed, hasher(t.c.y));
			glm::detail::hash_combine(seed, hasher(t.c.z));
			return seed;
		}
	};
}