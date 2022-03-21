#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>

class Texture2D;
class Texture3D;
class TextureArray;
class Utils
{
public:
	static float RandFloat(float min, float max);
	static int RandInt(int min, int max);
	static bool RandBool();
	static float Gaussian(float min, float max);

	static constexpr float PI() { return 3.14159265358979323846f; }
	static constexpr glm::vec3 FrontVec() { return glm::vec3(0.0f, 0.0f, -1.0f); }
	static constexpr glm::vec3 UpVec() { return glm::vec3(0.0f, 1.0f, 0.0f); }
	static constexpr glm::vec3 RightVec() { return glm::vec3(1.0f, 0.0f, 0.0f); }

	static void SaveTextureAsBMP(const std::string& savePath, Texture2D* texture);
	static void SaveTexture3DAsBMP(const std::string& savePath, Texture3D* texture);
	static void SaveTextureArrayAsBMP(const std::string& savePath, TextureArray* texture);
};