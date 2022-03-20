#include "Profiler.h"

#include <GLFW/glfw3.h>

#include "vendor/imgui/imgui.h"

std::unordered_map<std::string, float> Profiler::startTimes;
std::unordered_map<std::string, float> Profiler::results;

void Profiler::BeginProfile(const std::string& key)
{
	startTimes.insert({ key, glfwGetTime() });
}

void Profiler::EndProfile(const std::string& key)
{
	float currentTime = glfwGetTime();
	if (results.find(key) != results.end()) return;
	if (startTimes.find(key) == startTimes.end()) return;

	float timePassed = currentTime - startTimes.at(key);
	results.insert({key, timePassed});
}

std::unordered_map<std::string, float> Profiler::Results()
{
	return results;
}

void Profiler::DrawResults()
{
	ImGui::Begin("CPU Profiler");

	std::unordered_map<std::string, float>::iterator it = results.begin();
	while (it != results.end())
	{
		std::string v = it->first + ": " + std::to_string(it->second * 1000.0f) + "ms";
		ImGui::Text(v.c_str());
		it++;
	}

	ImGui::End();

	startTimes.clear();
	results.clear();
}