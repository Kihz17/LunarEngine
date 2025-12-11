#pragma once

#include "UUID.h"

#include <vector>

namespace lunar
{
	namespace resource
	{
		enum class ResourceType
		{
			None = -1,
			Texture,
			Mesh,
			Audio,
			Num,
		};

		struct ResourceID
		{
			UUID mID = 0;
			ResourceType mType = ResourceType::None;
			int mVersion = 0;
			std::vector<ResourceID> mDependencies;
		};
	}
}

namespace std
{
	template<> 
	struct hash<lunar::resource::ResourceID>
	{
		std::size_t operator()(const lunar::resource::ResourceID& id) const
		{
			return hash<uint64_t>()(id.mID);
		}
	};
}