#include "ResourceManager.h"

#include <stack>

#include "pch.h"

namespace lunar
{
	namespace resource
	{
		ResourceManager::ResourceManager()
		{
			// TODO: Initialize mLoaders
		}

		void ResourceManager::LoadSync(const ResourceID& id)
		{
			// SortDependencies
			// For each dependency in order:
	/*		IResourceLoader* loader = FindLoader(id);
			loader->LoadRaw();
			loader->Decode();
			Asset asset = loader->Finalize();
			mAssetCache.insert(id, asset);
			return asset;*/
		}

		void ResourceManager::LoadAsync(const ResourceID& id, int priority)
		{
			// SortDependencies
			// For each dependency in order:
			// Add to MPSC I/O queue with priority
			// I/O thread will eventually pick this up and load it and stage it for CPU job
		}

		IResourceLoader* ResourceManager::FindLoader(const ResourceID& id) const
		{
			ASSERT(id.mType != ResourceType::None && id.mType < ResourceType::Num)
			ASSERT(id.mType < mLoaders.size())

			return mLoaders[static_cast<int>(id.mType)].get();
		}

		void ResourceManager::SortDependencies(const ResourceID& id, std::vector<ResourceID>& sorted) const
		{
			for (const ResourceID& child : id.mDependencies)
			{
				SortDependencies(child, sorted);
			}

			sorted.push_back(id);
		}

#ifndef FINAL
		void ResourceManager::SortDependencies(const ResourceID& id, std::vector<ResourceID>& sorted, std::unordered_set<UUID>& visited) const
		{
			visited.insert(id.mID);

			for (const ResourceID& child : id.mDependencies)
			{
				ASSERT(!visited.count(child.mID)) // Check for circular dependencies
				SortDependencies(child, sorted);
			}

			sorted.push_back(id);
		}
#endif
	}
}

