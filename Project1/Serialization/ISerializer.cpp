#include "ISerializer.h"

namespace YAML
{
	Emitter& operator << (YAML::Emitter& out, const glm::vec2& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
		return out;
	}

	Emitter& operator << (YAML::Emitter& out, const glm::vec3& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	Emitter& operator << (YAML::Emitter& out, const glm::vec4& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
		return out;
	}

	Emitter& operator << (YAML::Emitter& out, const glm::quat& q)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << q.w << q.x << q.y << q.z << YAML::EndSeq;
		return out;
	}
}