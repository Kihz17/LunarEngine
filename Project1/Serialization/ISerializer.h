#pragma once

#include <string>

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class ISerializer
{
public:
	virtual void Serialize(YAML::Emitter& emitter) = 0;
	virtual void Deserialize(YAML::Node& node) = 0;
};

namespace YAML
{
	Emitter& operator << (YAML::Emitter& out, const glm::vec2& vec);
	Emitter& operator << (YAML::Emitter& out, const glm::vec3& vec);
	Emitter& operator << (YAML::Emitter& out, const glm::vec4& vec);
	Emitter& operator << (YAML::Emitter& out, const glm::quat& q);
}