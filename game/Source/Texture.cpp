#include "ITPEnginePCH.h"

Texture::Texture(class Game& game)
	:Asset(game)
	,mWidth(-1)
	,mHeight(-1)
	,mGraphicsDriver(game.GetRenderer().GetGraphicsDriver())
{

}

Texture::~Texture()
{

}

void Texture::SetActive(int slot)
{
	mGraphicsDriver.SetPSTexture(mTexture, slot);
}

bool Texture::Load(const char* fileName, class AssetCache* cache)
{
	// Load texture
	mTexture = mGraphicsDriver.CreateTextureFromFile(fileName, mWidth, mHeight);

	// Return false if fail
	if (!mTexture)
		return false;

	return true;
}

std::shared_ptr<Texture> Texture::CreateFromSurface(class Game& game, struct SDL_Surface* surface)
{
	TexturePtr tex = std::make_shared<Texture>(game);
	tex->mWidth = surface->w;
	tex->mHeight = surface->h;

	tex->mTexture = game.GetRenderer().GetGraphicsDriver().CreateTextureFromMemory(
		surface->pixels, tex->mWidth, tex->mHeight, EXT_BGRA);
	return tex;
}
