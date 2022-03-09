#include "DungeonGenUtils.h"

#include <glm/gtx/hash.hpp>

#include <unordered_set>

namespace DungeonGenUtils
{
	bool AlmostEqual(const glm::vec3& v1, const glm::vec3& v2)
	{
		glm::vec3 diff = v1 - v2;
		return glm::length(diff) * glm::length(diff) < 0.01f;
	}

	bool AlmostEqual(const glm::vec2& v1, const glm::vec2& v2)
	{
		glm::vec2 diff = v1 - v2;
		return glm::length(diff) * glm::length(diff) < 0.01f;
	}

	std::vector<Edge> MinimumSpanningTree(const std::vector<Edge>& edges, const glm::vec3& startVertex)
	{
		std::unordered_set<glm::vec3> openSet;
		std::unordered_set<glm::vec3> closedSet;

		for (const Edge& e : edges)
		{
			openSet.insert(e.a);
			openSet.insert(e.b);
		}

		closedSet.insert(startVertex);

		std::vector<Edge> results;

		while (openSet.size() > 0)
		{
			float minWeight = std::numeric_limits<float>::max();
			bool foundEdge = false;
			Edge chosenEdge;

			for (const Edge& e : edges) // Find the edge with the least distance between them
			{
				int closedVertices = 0;
				if (closedSet.find(e.a) == closedSet.end()) closedVertices++;
				if (closedSet.find(e.b) == closedSet.end()) closedVertices++;
				if (closedVertices != 1) continue;

				float edgeDistance = glm::distance(e.a, e.b);
				if (edgeDistance < minWeight)
				{
					chosenEdge = e;
					foundEdge = true;
					minWeight = edgeDistance;
				}
			}

			if (!foundEdge) break; // Nothing left to find, we're done here
			results.push_back(chosenEdge);

			openSet.erase(chosenEdge.a);
			openSet.erase(chosenEdge.b);

			closedSet.insert(chosenEdge.a);
			closedSet.insert(chosenEdge.b);
		}

		return results;
	}

	std::vector<Edge2D> MinimumSpanningTree2D(const std::vector<Edge2D>& edges, const glm::vec2& startVertex)
	{
		std::unordered_set<glm::vec2> openSet;
		std::unordered_set<glm::vec2> closedSet;

		for (const Edge2D& e : edges)
		{
			openSet.insert(e.a);
			openSet.insert(e.b);
		}

		closedSet.insert(startVertex);

		std::vector<Edge2D> results;

		while (openSet.size() > 0)
		{
			float minWeight = std::numeric_limits<float>::max();
			bool foundEdge = false;
			Edge2D chosenEdge;

			for (const Edge2D& e : edges) // Find the edge with the least distance between them
			{
				int closedVertices = 0;
				if (closedSet.find(e.a) == closedSet.end()) closedVertices++;
				if (closedSet.find(e.b) == closedSet.end()) closedVertices++;
				if (closedVertices != 1) continue;

				float edgeDistance = glm::distance(e.a, e.b);
				if (edgeDistance < minWeight)
				{
					chosenEdge = e;
					foundEdge = true;
					minWeight = edgeDistance;
				}
			}

			if (!foundEdge) break; // Nothing left to find, we're done here
			results.push_back(chosenEdge);

			openSet.erase(chosenEdge.a);
			openSet.erase(chosenEdge.b);

			closedSet.insert(chosenEdge.a);
			closedSet.insert(chosenEdge.b);
		}

		return results;
	}
}