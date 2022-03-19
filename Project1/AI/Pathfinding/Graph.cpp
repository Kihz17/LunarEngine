#include "Graph.h"

#include <stb/stb_image.h>

#include <iostream>

Graph::Graph(const std::string& path)
	: grid(nullptr)
{

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 3);
	if (!data)
	{
		std::cout << "Couldn't load PathGrid. File: " << path << "\n";
		return;
	}

	grid = new Grid2D<Node*>(glm::ivec2(width, height), glm::ivec2(0, 0));

	// Setup nodes
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < height; z++)
		{
			int index = channels * (z * width + x);
			int r = +data[index];
			int g = +data[index + 1];
			int b = +data[index + 2];

			if (r == 0 && g == 0 && b == 0)
			{
				Node* node = new Node();
				node->type = NodeType::Wall;
				grid->Set(glm::ivec2(x, z), node);
			}
			else if (r == 255 && g == 255 && b == 0)
			{
				Node* node = new Node();
				node->type = NodeType::Difficult;
				grid->Set(glm::ivec2(x, z), node);
			}
			else if (r == 0 && g == 255 && b == 0)
			{
				Node* node = new Node();
				node->type = NodeType::StartPosition;
				grid->Set(glm::ivec2(x, z), node);
			}
			else if (r == 255 && g == 0 && b == 0)
			{
				Node* node = new Node();
				node->type = NodeType::Resource;
				grid->Set(glm::ivec2(x, z), node);
			}
			else if (r == 0 && g == 0 && b == 255)
			{
				Node* node = new Node();
				node->type = NodeType::Home;
				grid->Set(glm::ivec2(x, z), node);
			}
			else
			{
				Node* node = new Node();
				grid->Set(glm::ivec2(x, z), node);
			}
		}
	}

	// Setup edges
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < height; z++)
		{
			glm::ivec2 currentPos(x, z);

			bool canMoveUp = false;
			bool canMoveDown = false;
			bool canMoveRight = false;
			bool canMoveLeft = false;
			if (grid->InBounds(glm::ivec2(x, z + 1))) // Look above
			{
				canMoveUp = SetupEdge(currentPos, glm::ivec2(x, z + 1), false);
			}
			if (grid->InBounds(glm::ivec2(x, z - 1))) // Look below
			{
				canMoveDown = SetupEdge(currentPos, glm::ivec2(x, z - 1), false);
			}
			if (grid->InBounds(glm::ivec2(x + 1, z))) // Look right
			{
				canMoveRight = SetupEdge(currentPos, glm::ivec2(x + 1, z), false);
			}
			if (grid->InBounds(glm::ivec2(x - 1, z))) // Look left
			{
				canMoveLeft = SetupEdge(currentPos, glm::ivec2(x - 1, z), false);
			}

			// Diagonal checks
			if (grid->InBounds(glm::ivec2(x + 1, z + 1)) && canMoveUp && canMoveRight)
			{
				SetupEdge(currentPos, glm::ivec2(x + 1, z + 1), true);
			}
			if (grid->InBounds(glm::ivec2(x + 1, z - 1)) && canMoveDown && canMoveRight)
			{
				SetupEdge(currentPos, glm::ivec2(x + 1, z - 1), true);
			}
			if (grid->InBounds(glm::ivec2(x - 1, z - 1)) && canMoveDown && canMoveLeft)
			{
				SetupEdge(currentPos, glm::ivec2(x - 1, z + 1), true);
			}
			if (grid->InBounds(glm::ivec2(x - 1, z + 1)) && canMoveUp && canMoveLeft)
			{
				SetupEdge(currentPos, glm::ivec2(x - 1, z + 1), true);
			}
		}
	}

	stbi_image_free(data);
}

Graph::~Graph()
{
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < height; z++)
		{
			delete grid->Get(glm::ivec2(x, z));
		}
	}

	delete grid;
}

void Graph::Reset()
{
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < height; z++)
		{
			Node* node = grid->Get(glm::ivec2(x, z));
			node->prev = nullptr;
			node->visited = false;
			node->cost = std::numeric_limits<float>::max();
		}
	}
}

bool Graph::SetupEdge(const glm::ivec2& current, const glm::ivec2& neighbourPos, bool diag)
{
	Node* node = grid->Get(current);
	Node* neighbour = grid->Get(neighbourPos);

	NodeType fromType = node->type;
	NodeType toType = neighbour->type;
	bool traversable = toType != NodeType::Wall;

	if (traversable) // We can move to neighbour
	{
		float weight = diag ? 14.0f : 10.0f;

		if (toType == NodeType::Difficult) // Double the cost from normal -> difficult
		{
			weight *= 2.0f;
		}

		node->edges.push_back({ neighbour, weight });
	}

	return traversable;
}