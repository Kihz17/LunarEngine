#pragma once

#include "Component.h"

#include <string>
#include <unordered_map>

enum class TagValueType
{
	Int,
	Float,
	Bool
};

struct ITagValue
{
	virtual ~ITagValue() = default;

	virtual TagValueType GetType() const = 0;
};

template<typename T>
struct TagValue : public ITagValue
{
	TagValue(T value, TagValueType type) : value(value), type(type) {}

	virtual TagValueType GetType() const override { return type; }

	T value;
	TagValueType type;
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

	void AddTag(const std::string& tag) { tags.insert({ tag, new TagValue<bool>(true, TagValueType::Bool) }); }
	void AddTagFloat(const std::string& tag, float value) { tags.insert({ tag, new TagValue<float>(value, TagValueType::Float) }); }
	void AddTagInt(const std::string& tag, int value) { tags.insert({ tag, new TagValue<int>(value, TagValueType::Int) }); }
	void AddTagBool(const std::string& tag, bool value) { tags.insert({ tag, new TagValue<bool>(value, TagValueType::Bool) }); }

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