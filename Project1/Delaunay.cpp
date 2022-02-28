#include "Delaunay.h"

#include <unordered_set>

void Delaunay::Tetrahedralize()
{
	glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const glm::vec3& v : vertices)
	{
		min.x = glm::min(min.x, v.x);
		min.y = glm::min(min.y, v.y);
		min.z = glm::min(min.z, v.z);
		max.x = glm::max(max.x, v.x);
		max.y = glm::max(max.y, v.y);
		max.z = glm::max(max.z, v.z);
	}

	glm::vec3 diff = max - min;
	float deltaMax = glm::max(glm::max(diff.x, diff.y), diff.z) * 2.0f;

	// Create "super" tetrahedron (a tetragedron that contains all of our vertices)
	glm::vec3 v1(min.x - 1, min.y - 1, min.z - 1);
	glm::vec3 v2(max.x + deltaMax, min.y - 1, min.z - 1);
	glm::vec3 v3(min.x - 1, max.y + deltaMax, min.z - 1);
	glm::vec3 v4(min.x - 1, min.y - 1, max.z + deltaMax);

	tetrahedra.push_back({v1, v2, v3, v4});

	for (const glm::vec3& v : vertices)
	{
		std::vector<DelaunayTriangle> triangles;
		std::vector<Tetrahedron> tetrahedraToRemove;
		std::vector<DelaunayTriangle> trianglesToRemove;

		for(Tetrahedron& t : tetrahedra)
		{
			if (t.CircumSphereContains(v)) // Check the the vertex is within the tetrahedras circumsphere
			{
				triangles.push_back({ t.a, t.b, t.c });
				triangles.push_back({ t.a, t.b, t.d });
				triangles.push_back({ t.a, t.c, t.d });
				triangles.push_back({ t.b, t.c, t.d });
				tetrahedraToRemove.push_back(t);
			}
		}

		for (unsigned int i = 0; i < triangles.size(); i++)
		{
			for (unsigned int j = i + 1; j < triangles.size(); j++)
			{
				if (triangles[i].AlmostEquals(triangles[j]))
				{
					trianglesToRemove.push_back(triangles[i]);
					trianglesToRemove.push_back(triangles[j]);
				}
			}
		}

		for (Tetrahedron& t : tetrahedraToRemove)
		{
			std::vector<Tetrahedron>::iterator it = std::find(tetrahedra.begin(), tetrahedra.end(), t);
			if (it != tetrahedra.end()) tetrahedra.erase(it);
		}

		// Remove invalid triangles
		for (const DelaunayTriangle& t : trianglesToRemove)
		{
			std::vector<DelaunayTriangle>::iterator findIt = std::find(triangles.begin(), triangles.end(), t);
			if (findIt != triangles.end())
			{
				triangles.erase(std::remove(triangles.begin(), triangles.end(), t));
			}
		}

		// Convert valid triangles into tetrahedra
		for (const DelaunayTriangle& tri : triangles)
		{
			tetrahedra.push_back({ tri.a, tri.b, tri.c, v});
		}
	}

	std::vector<Tetrahedron>::iterator it = tetrahedra.begin();
	while (it != tetrahedra.end())
	{
		Tetrahedron& t = (*it);
		if (t.ContainsVertex(v1) || t.ContainsVertex(v2) || t.ContainsVertex(v3) || t.ContainsVertex(v4))
		{
			it = tetrahedra.erase(it);
		}
		else
		{
			it++;
		}
	}

	std::unordered_set<DelaunayTriangle> triangleSet;
	std::unordered_set<Edge> edgeSet;

	for (Tetrahedron& t : tetrahedra)
	{
		DelaunayTriangle abc(t.a, t.b, t.c);
		DelaunayTriangle abd(t.a, t.b, t.d);
		DelaunayTriangle acd(t.a, t.c, t.d);
		DelaunayTriangle bcd(t.b, t.c, t.d);

		if (triangleSet.find(abc) == triangleSet.end())
		{
			triangleSet.insert(abc);
			triangles.push_back(abc);
		}

		if (triangleSet.find(abd) == triangleSet.end())
		{
			triangleSet.insert(abd);
			triangles.push_back(abd);
		}

		if (triangleSet.find(acd) == triangleSet.end())
		{
			triangleSet.insert(acd);
			triangles.push_back(acd);
		}

		if (triangleSet.find(bcd) == triangleSet.end())
		{
			triangleSet.insert(bcd);
			triangles.push_back(bcd);
		}

		Edge ab(t.a, t.b);
		Edge bc(t.b, t.c);
		Edge ca(t.c, t.a);
		Edge da(t.d, t.a);
		Edge db(t.d, t.b);
		Edge dc(t.d, t.c);

		if (edgeSet.find(ab) == edgeSet.end())
		{
			edgeSet.insert(ab);
			edges.push_back(ab);
		}

		if (edgeSet.find(bc) == edgeSet.end())
		{
			edgeSet.insert(bc);
			edges.push_back(bc);
		}

		if (edgeSet.find(ca) == edgeSet.end())
		{
			edgeSet.insert(ca);
			edges.push_back(ca);
		}

		if (edgeSet.find(da) == edgeSet.end())
		{
			edgeSet.insert(da);
			edges.push_back(da);
		}

		if (edgeSet.find(db) == edgeSet.end())
		{
			edgeSet.insert(db);
			edges.push_back(db);
		}

		if (edgeSet.find(dc) == edgeSet.end())
		{
			edgeSet.insert(dc);
			edges.push_back(dc);
		}
	}
}