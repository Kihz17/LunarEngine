#include "TextureSerializer.h"

TextureSerializer::TextureSerializer(ITexture* t)
	: texture(t)
{

}

void TextureSerializer::Serialize(YAML::Emitter& emitter)
{
	emitter << YAML::BeginMap;

	if (dynamic_cast<Texture2D*>(texture))
	{
		SaveTexture2D(emitter, dynamic_cast<Texture2D*>(texture));
	}

	emitter << YAML::EndMap;
}

void TextureSerializer::Deserialize(YAML::Node& node)
{

}

void TextureSerializer::SaveTexture2D(YAML::Emitter& emitter, Texture2D* t)
{
	emitter << YAML::Key << "Path" << YAML::Value << t->GetPath();
	emitter << YAML::Key << "FilterType" << YAML::Value << (int) t->GetFilterType();
	emitter << YAML::Key << "WrapType" << YAML::Value << (int)t->GetWrapType();
}