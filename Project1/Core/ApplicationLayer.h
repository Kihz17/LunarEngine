#pragma once

#include <string>

class ApplicationLayer
{
public:
	ApplicationLayer(const std::string& name = "Layer") : name(name) {}
	virtual ~ApplicationLayer() = default;

	// These should NOT be pure virtual because we may or may not need to override them
	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(float deltaTime) {}
	virtual void OnImGuiRender() {}

	inline const std::string& GetName() { return this->name; }
protected:
	std::string name;
};