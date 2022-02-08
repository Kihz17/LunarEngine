#include "TextureManager.h"
#include "Texture2D.h"
#include "TextureArray.h"
#include "CubeMap.h"

std::vector<ITexture*> TextureManager::textures;

void TextureManager::Initialize()
{

}

void TextureManager::CleanUp()
{
	for (ITexture* texture : textures) delete texture;
	textures.clear();
}

Texture2D* TextureManager::CreateTexture2D(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, bool flip, bool hdr)
{
	Texture2D* texture = new Texture2D(path, filter, wrap, genMipMaps, flip, hdr);
	textures.push_back(texture);
	return texture;
}

Texture2D* TextureManager::CreateTexture2D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
{
	Texture2D* texture = new Texture2D(internalFormat, format, dataFormat, width, height, filter, wrap, genMipMaps);
	textures.push_back(texture);
	return texture;
}

TextureArray* TextureManager::CreateTextureArray(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps)
{
	TextureArray* texture = new TextureArray(internalFormat, format, dataFormat, width, height, depth, filter, wrap, genMipMaps);
	textures.push_back(texture);
	return texture;
}

CubeMap* TextureManager::CreateCubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps, bool flip)
{
	CubeMap* texture = new CubeMap(paths, filter, wrap, genMipMaps, flip);
	textures.push_back(texture);
	return texture;
}

CubeMap* TextureManager::CreateCubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type)
{
	CubeMap* texture =  new CubeMap(width, minFilter, format, internalFormat, type);
	textures.push_back(texture);
	return texture;
}

void TextureManager::DeleteTexture(ITexture* texture)
{
	int textureIndex = -1;
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i] == texture)
		{
			textureIndex = i;
			break;
		}
	}

	if (textureIndex != -1) textures.erase(textures.begin() + textureIndex);
	delete texture;
}