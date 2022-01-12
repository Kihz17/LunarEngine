#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>

class VertexArrayObject
{
public:
	VertexArrayObject();
	virtual ~VertexArrayObject();

	void Bind() const;
	void Unbind() const;

	void AddVertexBuffer(VertexBuffer* vbo);
	void SetIndexBuffer(IndexBuffer* ebo);

	inline virtual const std::vector<VertexBuffer*> GetVertexBuffers() const { return this->vertexBuffers; }
	inline virtual const IndexBuffer* GetIndexBuffer() const { return this->indexBuffer; }

private:
	GLuint ID; // Holds the ID to our VAO
	GLuint VBOIndex; // Holds the current index of out VBO

	std::vector<VertexBuffer*> vertexBuffers;
	IndexBuffer* indexBuffer;
};