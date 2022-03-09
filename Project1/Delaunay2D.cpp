#include "Delaunay2D.h"

#include <unordered_set>

void Delaunay2D::Triangulate()
{
	glm::vec2 min(FLT_MAX, FLT_MAX);
	glm::vec2 max(-FLT_MAX, -FLT_MAX);

	for (const glm::vec2& v : vertices)
	{
		min.x = glm::min(min.x, v.x);
		min.y = glm::min(min.y, v.y);
		max.x = glm::max(max.x, v.x);
		max.y = glm::max(max.y, v.y);
	}

	glm::vec2 diff = max - min;
	float deltaMax = glm::max(diff.x, diff.y) * 2.0f;

	// Create "super triangle" (triangle that contains all vertices)
	glm::vec2 v1(min.x - 1, min.y - 1);
	glm::vec2 v2(min.x - 1, max.y + deltaMax);
	glm::vec2 v3(max.x + deltaMax, min.y - 1);

	triangles.push_back({ v1, v2, v3 });

	for (const glm::vec2& v : vertices)
	{
		std::vector<Edge2D> polygon;
		std::vector<DelaunayTriangle2D> trianglesToRemove;
		std::vector<Edge2D> edgesToRemove;

		for (const DelaunayTriangle2D& t : triangles)
		{
			if (t.CircumCircleContains(v))
			{
				polygon.push_back({ t.a, t.b });
				polygon.push_back({ t.b, t.c });
				polygon.push_back({ t.c, t.a });
				trianglesToRemove.push_back(t);
			}
		}

		for (DelaunayTriangle2D& t : trianglesToRemove)
		{
			std::vector<DelaunayTriangle2D>::iterator it = std::find(triangles.begin(), triangles.end(), t);
			if (it != triangles.end()) triangles.erase(it);
		}

		for (int i = 0; i < polygon.size(); i++)
		{
			for (int j = i + 1; j < polygon.size(); j++)
			{
				if (polygon[i].AlmostEqual(polygon[j]))
				{
					edgesToRemove.push_back(polygon[i]);
					edgesToRemove.push_back(polygon[j]);
				}
			}
		}

		for (const Edge2D& e : edgesToRemove)
		{
			std::vector<Edge2D>::iterator it = std::find(polygon.begin(), polygon.end(), e);
			if (it != polygon.end()) polygon.erase(it);
		}

		for (const Edge2D& e : polygon) triangles.push_back({ e.a, e.b, v });
	}

	std::vector<DelaunayTriangle2D>::iterator it = triangles.begin();
	while (it != triangles.end())
	{
		DelaunayTriangle2D& t = (*it);
		if (t.ContainsVertex(v1) || t.ContainsVertex(v2) || t.ContainsVertex(v3))
		{
			it = triangles.erase(it);
		}
		else
		{
			it++;
		}
	}

	std::unordered_set<Edge2D> edgeSet;
	for (const DelaunayTriangle2D& t : triangles)
	{
		Edge2D a(t.a, t.b);
		Edge2D b(t.b, t.c);
		Edge2D c(t.c, t.a);

		if (edgeSet.find(a) == edgeSet.end())
		{
			edgeSet.insert(a);
			edges.push_back(a);
		}

		if (edgeSet.find(b) == edgeSet.end())
		{
			edgeSet.insert(b);
			edges.push_back(b);
		}

		if (edgeSet.find(c) == edgeSet.end())
		{
			edgeSet.insert(c);
			edges.push_back(c);
		}
	}
}