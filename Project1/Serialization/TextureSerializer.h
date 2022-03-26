#pragma once

#include "ISerializer.h"
#include "ITexture.h"
#include "Texture2D.h"

class TextureSerializer : public ISerializer
{
public:
	TextureSerializer(ITexture* t);

	virtual void Serialize(YAML::Emitter& emitter) override;
	virtual void Deserialize(const YAML::Node& node) override;

private:
	void SaveTexture2D(YAML::Emitter& emitter, Texture2D* t);

	ITexture* texture;
};