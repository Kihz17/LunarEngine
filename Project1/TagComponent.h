#pragma once

#include "Component.h"

#include <string>
#include <unordered_map>

struct TagValue
{
	virtual bool GetBoolean() const { return false; }
	virtual int8_t GetByte() const { return 0; }
	virtual int16_t GetShort() const { return 0; }
	virtual int GetInt() const { return 0; }
	virtual int64_t GetInt64() const { return 0; }
	virtual float GetFloat() const { return 0.0f; }
	virtual double GetDouble() const { return 0.0; }
	virtual std::string GetString() const { return ""; }
};

struct BoolTagValue : public TagValue
{
	BoolTagValue(bool value) : value(value) {}

	virtual bool GetBoolean() const override { return value; }

	bool value;
};

struct ByteTagValue : public TagValue
{
	ByteTagValue(int8_t value) : value(value) {}

	virtual int8_t GetByte() const override { return value; }

	int8_t value;
};

struct ShortTagValue : public TagValue
{
	ShortTagValue(int16_t value) : value(value) {}

	virtual int16_t GetShort() const override { return value; }

	int16_t value;
};

struct IntTagValue : public TagValue
{
	IntTagValue(int value) : value(value) {}

	virtual int GetInt() const override { return value; }

	int value;
};

struct Int64TagValue : public TagValue
{
	Int64TagValue(int64_t value) : value(value) {}

	virtual int64_t GetInt64() const override { return value; }

	int64_t value;
};

struct FloatTagValue : public TagValue
{
	FloatTagValue(float value) : value(value) {}

	virtual float GetFloat() const override { return value; }

	float value;
};

struct DoubleTagValue : public TagValue
{
	DoubleTagValue(double value) : value(value) {}

	virtual double GetDouble() const override { return value; }

	double value;
};

struct StringTagValue : public TagValue
{
	StringTagValue(std::string value) : value(value) {}

	virtual std::string GetString() const override { return value; }

	std::string value;
};


struct TagComponent : public Component
{
	TagComponent() {}

	virtual void ImGuiUpdate() override
	{

	}

	void AddTag(const std::string& tag) { AddTag(tag, BoolTagValue(true)); }
	void AddTag(const std::string& tag, const TagValue& value) { tags.insert({ tag, value }); }

	TagValue GetValue(const std::string& tag) 
	{ 
		std::unordered_map<std::string, TagValue>::iterator it = tags.find(tag);
		if (it == tags.end()) return StringTagValue("Tag Does Not Exist");
		return it->second;
	}

	bool HasTag(const std::string& tag) const { return tags.find(tag) != tags.end(); }

	std::unordered_map<std::string, TagValue> tags;
};