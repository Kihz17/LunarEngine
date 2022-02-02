#include "ApplicationLayerManager.h"

ApplicationLayerManager::~ApplicationLayerManager()
{
	for (ApplicationLayer* layer : this->layers)
	{
		layer->OnDetach();
		delete layer;
	}
}

void ApplicationLayerManager::AddLayer(ApplicationLayer* layer)
{
	this->layers.emplace(this->layers.begin() + this->insertIndex++, layer); // Put the layer in the vector BEFORE the overlays
}

void ApplicationLayerManager::RemoveLayer(ApplicationLayer* layer, const bool remove)
{
	std::vector<ApplicationLayer*>::iterator it = std::find(this->layers.begin(), this->layers.begin() + this->insertIndex, layer);
	if (it != this->layers.begin() + this->insertIndex) // We found the layer, remove it
	{
		layer->OnDetach();
		this->layers.erase(it);
		this->insertIndex--;

		if (remove)
		{
			delete layer;
		}
	}
}

void ApplicationLayerManager::AddOverlay(ApplicationLayer* overlay)
{
	this->layers.push_back(overlay); // Add overlays to the end
}

void ApplicationLayerManager::RemoveOverlay(ApplicationLayer* overlay, const bool remove)
{
	std::vector<ApplicationLayer*>::iterator it = std::find(this->layers.begin() + this->insertIndex, this->layers.end(), overlay);
	if (it != this->layers.end()) // We found the layer, remove it
	{
		overlay->OnDetach();
		this->layers.erase(it);

		if (remove)
		{
			delete overlay;
		}
	}
}