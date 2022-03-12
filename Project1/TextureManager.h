#pragma once

#include "ITexture.h"

#include <vector>
#include <string>

class Texture2D;
class Texture3D;
class TextureArray;
class CubeMap;
class TextureManager
{
public:
	static void Initialize();
	static void CleanUp();

	static Texture2D* CreateTexture2D(const std::string& path, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true, bool hdr = false);
	static Texture2D* CreateTexture2D(void* data, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);
	static Texture2D* CreateTexture2D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);

	static Texture3D* CreateTexture3D(void* data, GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);
	static Texture3D* CreateTexture3D(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);

	static TextureArray* CreateTextureArray(GLenum internalFormat, GLenum format, GLenum dataFormat, int width, int height, int depth, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true);

	static CubeMap* CreateCubeMap(const std::vector<std::string>& paths, TextureFilterType filter, TextureWrapType wrap, bool genMipMaps = true, bool flip = true);
	static CubeMap* CreateCubeMap(int width, GLenum minFilter, GLenum format, GLenum internalFormat, GLenum type = GL_UNSIGNED_BYTE);

	static void DeleteTexture(ITexture* texture);

private:
	TextureManager() = default;

	static std::vector<ITexture*> textures;
};