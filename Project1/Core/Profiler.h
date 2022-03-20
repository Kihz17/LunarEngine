#pragma once

#include <unordered_map>
#include <string>

class Profiler
{
public:

	static void BeginProfile(const std::string& key);
	static void EndProfile(const std::string& key);
	static std::unordered_map<std::string, float> Results();
	static void DrawResults();
private:
	static std::unordered_map<std::string, float> startTimes;
	static std::unordered_map<std::string, float> results;
};