#include "EquirectangularToCubeMapConverter.h"
#include "ShaderLibrary.h"
#include "FrameBuffer.h"

const std::string EquirectangularToCubeMapConverter::CUBE_MAP_CONVERT_SHADER_KEY = "hdrToCubeShader";
Shader* EquirectangularToCubeMapConverter::conversionShader = nullptr;
IFrameBuffer* EquirectangularToCubeMapConverter::cubeMapBuffer = nullptr;

void EquirectangularToCubeMapConverter::Initialize()
{
	conversionShader = ShaderLibrary::Load(CUBE_MAP_CONVERT_SHADER_KEY, "assets/shaders/equirectangularToCubeMap.glsl");
	cubeMapBuffer = new FrameBuffer();

	// Setup conversion shader uniforms
	conversionShader->Bind();
	conversionShader->InitializeUniform("uProjection");
	conversionShader->InitializeUniform("uView");
	conversionShader->InitializeUniform("uEnvMap");
	conversionShader->Unbind();
}

void EquirectangularToCubeMapConverter::CleanUp()
{
	delete cubeMapBuffer;
}

void EquirectangularToCubeMapConverter::ConvertEquirectangularToCubeMap(Texture2D* envMapHDR, CubeMap* cubeMap, PrimitiveShape* cube, unsigned int nativeWidth, unsigned int nativeHeight)
{
	glDisable(GL_CULL_FACE); // Make sure none of our cubes faces get culled

	// Convert the HDR equirectangular texture to its cube map equivalent
	{
		// Setup conversion uniforms for shader
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 captureViews[] = // 6 different view matrices that face each side of the cube
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		conversionShader->Bind();
		conversionShader->SetMat4("uProjection", captureProjection);

		envMapHDR->BindToSlot(0);

		glViewport(0, 0, cubeMap->GetWidth(), cubeMap->GetHeight()); // Make sure we configure the viewport to capture the texture dimensions

		cubeMapBuffer->Bind();
		for (int i = 0; i < 6; i++) // Setup all faces of the cube
		{
			conversionShader->SetMat4("uView", captureViews[i]);
			cubeMapBuffer->AddColorAttachmentCubeMapFace("cubeFace", cubeMap, 0, (CubeMapFace)i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube->Draw();
		}

		cubeMap->ComputeMipmap();
		cubeMapBuffer->Unbind();
	}

	glViewport(0, 0, nativeWidth, nativeHeight); // Set viewport back to native width/height
	glEnable(GL_CULL_FACE); // Re-enable face culling
}