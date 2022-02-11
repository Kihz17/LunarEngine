#include "DynamicCubeMapRenderer.h"
#include "TextureManager.h"
#include "FrameBuffer.h"

DynamicCubeMapRenderer::DynamicCubeMapRenderer()
	: frameBuffer(new FrameBuffer()),
	cubeMap(TextureManager::CreateCubeMap(128, GL_LINEAR_MIPMAP_LINEAR, GL_RGB, GL_RGB16F, GL_FLOAT))
{

}

DynamicCubeMapRenderer::~DynamicCubeMapRenderer()
{
	delete frameBuffer;
	TextureManager::DeleteTexture(cubeMap);
}

CubeMap* DynamicCubeMapRenderer::GenerateDynamicCubeMap(const glm::vec3& center, std::vector<RenderSubmission>& submissions)
{
	// TODO: When getting the center position, make sure to get the middle of the meshe's AABB
	for (RenderSubmission& submission : submissions)
	{

	}

	return nullptr;
}