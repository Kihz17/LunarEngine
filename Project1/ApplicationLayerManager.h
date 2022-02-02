#pragma once

#include "ApplicationLayer.h"

#include <vector>

class ApplicationLayerManager
{
public:
	ApplicationLayerManager() = default;
	~ApplicationLayerManager();

	void AddLayer(ApplicationLayer* layer);
	void RemoveLayer(ApplicationLayer* layer, const bool remove = false); // WARNING: If remove is false, you will be responsible for releasing the pointer

	// We have seperate functions for pushing an "overlay" because they need to be treated differently.
	// This is because an overlay could possibly rely on another layer to finish update run properly
	void AddOverlay(ApplicationLayer* overlay);
	void RemoveOverlay(ApplicationLayer* overlay, const bool remove = false); // WARNING: If you remove is false, you will be responsible for releasing the pointer

	std::vector<ApplicationLayer*>::iterator begin() { return this->layers.begin(); }
	std::vector<ApplicationLayer*>::iterator end() { return this->layers.end(); }
	std::vector<ApplicationLayer*>::reverse_iterator rbegin() { return this->layers.rbegin(); }
	std::vector<ApplicationLayer*>::reverse_iterator rend() { return this->layers.rend(); }

	std::vector<ApplicationLayer*>::const_iterator begin() const { return this->layers.begin(); }
	std::vector<ApplicationLayer*>::const_iterator end()	const { return this->layers.end(); }
	std::vector<ApplicationLayer*>::const_reverse_iterator rbegin() const { return this->layers.rbegin(); }
	std::vector<ApplicationLayer*>::const_reverse_iterator rend() const { return this->layers.rend(); }

private:
	std::vector<ApplicationLayer*> layers;
	unsigned int insertIndex;
};