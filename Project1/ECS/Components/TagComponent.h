#pragma once

#include "Component.h"

#include <string>
#include <unordered_map>

struct ITagValue
{
	virtual ~ITagValue() = default;
};

template<typename T>
struct TagValue : public ITagValue
{
	TagValue(T value) : value(value) {}

	T value;
};

struct TagComponent : public Component
{
	TagComponent() {}
	virtual ~TagComponent()
	{
		std::unordered_map<std::string, ITagValue*>::iterator it;
		for (it = tags.begin(); it != tags.end(); it++)
		{
			delete it->second;
		}
	}

	void AddTag(const std::string& tag) { AddTag(tag, new TagValue<bool>(true)); }
	template <typename T> void AddTag(const std::string& tag, TagValue<T>* value) { tags.insert({ tag, value }); }

	template <typename T> TagValue<T>* GetValue(const std::string& tag)
	{ 
		std::unordered_map<std::string, ITagValue*>::iterator it = tags.find(tag);
		if (it == tags.end()) return nullptr;
		TagValue<T>* value = dynamic_cast<TagValue<T>*>(it->second);
		return value;
	}

	bool HasTag(const std::string& tag) const { return tags.find(tag) != tags.end(); }

	std::unordered_map<std::string, ITagValue*> tags;
};