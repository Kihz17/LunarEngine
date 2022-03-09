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

	bool CircumCircleContains(const glm::vec3& v) const;

	bool ContainsVertex(const glm::vec3& v) const;

	glm::vec3 a, b, c;
};

struct DelaunayTriangle2D
{
	DelaunayTriangle2D() = default;
	DelaunayTriangle2D(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

	bool AlmostEquals(const DelaunayTriangle2D& other);

	bool operator==(const DelaunayTriangle2D& t) const;

	bool CircumCircleContains(const glm::vec2& v) const;

	bool ContainsVertex(const glm::vec2& v) const;

	glm::vec2 a, b, c;
};

struct Edge
{
	Edge() = default;
	Edge(const glm::vec3& a, const glm::vec3& b);

	bool operator==(const Edge& t) const;

	bool AlmostEqual(const Edge& e) const;

	glm::vec3 a, b;
};

struct Edge2D
{
	Edge2D() = default;
	Edge2D(const glm::vec2& a, const glm::vec2& b);

	bool operator==(const Edge2D& t) const;

	bool AlmostEqual(const Edge2D& e) const;

	glm::vec2 a, b;
};

struct DungeonRoom
{
	DungeonRoom() = default;
	DungeonRoom(const glm::ivec3& center, const glm::ivec3& size)
		: center(center), size(size)
	{}

	bool Intersects(const DungeonRoom& otherRoom)
	{
		glm::ivec3 min = center - size;
		glm::ivec3 max = center + size;

		glm::ivec3 otherMin = otherRoom.center - otherRoom.size;
		glm::ivec3 otherMax = otherRoom.center + otherRoom.size;

		return (min.x <= otherMax.x && max.x >= otherMin.x)
			&& (min.y <= otherMax.y && max.y >= otherMin.y)
			&& (min.z <= otherMax.z && max.z >= otherMin.z);
	}

	glm::ivec3 center;
	glm::ivec3 size;
};

struct DungeonRoom2D
{
	DungeonRoom2D() = default;
	DungeonRoom2D(const glm::ivec2& center, const glm::ivec2& size)
		: pos(center), size(size)
	{}

	bool Intersects(const DungeonRoom2D & otherRoom)
	{
		glm::ivec2 min = pos - size;
		glm::ivec2 max = pos + size;

		glm::ivec2 otherMin = otherRoom.pos - otherRoom.size;
		glm::ivec2 otherMax = otherRoom.pos + otherRoom.size;

		return (min.x <= otherMax.x && max.x >= otherMin.x) && (min.y <= otherMax.y && max.y >= otherMin.y);
	}

	glm::ivec2 pos;
	glm::ivec2 size;
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
	struct hash<Edge2D>
	{
		std::size_t operator()(const Edge2D& edge) const
		{
			size_t seed = 0;
			hash<float> hasher;
			glm::detail::hash_combine(seed, hasher(edge.a.x));
			glm::detail::hash_combine(seed, hasher(edge.a.y));

			glm::detail::hash_combine(seed, hasher(edge.b.x));
			glm::detail::hash_combine(seed, hasher(edge.b.y));
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

	template<>
	struct hash<DelaunayTriangle2D>
	{
		std::size_t operator()(const DelaunayTriangle2D& t) const
		{
			size_t seed = 0;
			hash<float> hasher;
			glm::detail::hash_combine(seed, hasher(t.a.x));
			glm::detail::hash_combine(seed, hasher(t.a.y));

			glm::detail::hash_combine(seed, hasher(t.b.x));
			glm::detail::hash_combine(seed, hasher(t.b.y));

			glm::detail::hash_combine(seed, hasher(t.c.x));
			glm::detail::hash_combine(seed, hasher(t.c.y));
			return seed;
		}
	};
}