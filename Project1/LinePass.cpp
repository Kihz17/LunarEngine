#include "LinePass.h"
#include "ShaderLibrary.h"

LinePass::LinePass()
	: shader(ShaderLibrary::Load("lineShader", "assets/shaders/lines.glsl"))
{
	shader->Bind();
	shader->InitializeUniform("uMatModel");
	shader->InitializeUniform("uMatView");
	shader->InitializeUniform("uMatProjection");
	shader->InitializeUniform("uLineColor");
	shader->Unbind();
}

LinePass::~LinePass()
{

}

void LinePass::DoPass(std::vector<LineRenderSubmission>& submissions, const glm::mat4& projection, const glm::mat4& view, const WindowSpecs* windowSpecs)
{
	// NOTE: This pass should always be after the forward pass, meaning that the depth values have already been copied over to the default framebuffer

	shader->Bind();

	for (LineRenderSubmission& submission : submissions)
	{
		shader->SetMat4("uMatModel", submission.transform);
		shader->SetMat4("uMatView", view);
		shader->SetMat4("uMatProjection", projection);
		shader->SetFloat3("uLineColor", submission.lineColor);

		submission.vao->Bind();
		glLineWidth(submission.lineWidth); // Set width of line
		glDrawElements(GL_LINES, submission.indexCount, GL_UNSIGNED_INT, 0);
		submission.vao->Unbind();
	}
}