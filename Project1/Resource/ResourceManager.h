#pragma once


#include "ResourceHandle.h"
#include "Factory/ResourceFactory.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

// TODO: Handles (ID, metadata, asset type, dependencies, versioning)
// TODO: Factory loading (Serialize/Deserialize readable and binary)
// TODO: Fallback for failed loads
// TODO: Load priority options
// TODO: Events for progress completions 
// TODO: Cache line contention
// TODO: Preallocated max pool ahead of time
// TODO: Dedicated mem for perm resouces that never unload
// TODO: Load to unique ptr and move
// TODO: Load to shared ref
// TODO: DAG resolution
// TODO: Dont allow viewing the resource until loaded
// TODO: Sync load
// TODO: Async load
// TODO: Streaming
// TODO: Bad LOD first then progressively load better textures later


namespace lunar
{
	namespace resource
	{
		class ResourceManager
		{
		private:
			struct CachedAsset
			{
				void* mData;
				int mRefCount = 0;
			};

		public:
			ResourceManager();

	

		private:
			void LoadSync(const ResourceID& id);
			void LoadAsync(const ResourceID& id, int priority);

			IResourceLoader* FindLoader(const ResourceID& id) const;
			void SortDependencies(const ResourceID& id, std::vector<ResourceID>& sorted) const;

#ifndef FINAL
			void SortDependencies(const ResourceID& id, std::vector<ResourceID>& sorted, std::unordered_set<UUID>& visited) const;

#endif

			std::unordered_map<ResourceID, CachedAsset> mPermenantAssets; // Never unloaded
			std::unordered_map<ResourceID, CachedAsset> mAssetCache;

			std::vector<std::unique_ptr<IResourceLoader>> mLoaders; // Indexed by ResourceType, IMMUTABLE after initialization
		};
	}
}