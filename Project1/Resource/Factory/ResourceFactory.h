#pragma once
#include <string>

namespace lunar
{
	namespace resource
	{
		class IResourceLoader
		{
		public:
			virtual ~IResourceLoader() = default;

			virtual void* LoadRaw(const std::string& path) = 0; // I/O load
			virtual void* Decode(const std::string& path) = 0; // CPU bound
			virtual void* Finalize(const std::string& path) = 0; // Finalize asset
		};
	}
}
