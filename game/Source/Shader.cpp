#include "ITPEnginePCH.h"
#include <SDL/SDL_log.h>

Shader::Shader(class Game& game)
	:Asset(game)
	,mGraphicsDriver(mGame.GetRenderer().GetGraphicsDriver())
{
}

Shader::~Shader()
{
}

void Shader::SetActive()
{
	// Set shaders as active
	mGraphicsDriver.SetVertexShader(mVertexShader);
	mGraphicsDriver.SetPixelShader(mPixelShader);

	// Set vertex shader constant buffers
	mGraphicsDriver.SetVSConstantBuffer(mPerCameraBuffer, 0);
	mGraphicsDriver.SetVSConstantBuffer(mPerObjectBuffer, 1);

	// Set pixel shader constant buffer
	mGraphicsDriver.SetPSConstantBuffer(mPerCameraBuffer, 0);

	// Set sampler state for pixel shader
	mGraphicsDriver.SetPSSamplerState(mDefaultSampler, 0);

	// Set the lighting constant buffer
	mGraphicsDriver.SetPSConstantBuffer(mLightingBuffer, 2);

	// Set the matrix palette buffer, if it exists
	if (mMatrixPaletteBuffer)
	{
		mGraphicsDriver.SetVSConstantBuffer(mMatrixPaletteBuffer, 3);
	}
}

void Shader::CreateMatrixPaletteBuffer()
{
	MatrixPalette matrixPalette;
	mMatrixPaletteBuffer = mGraphicsDriver.CreateGraphicsBuffer(
		&matrixPalette, sizeof(matrixPalette),
		EBF_ConstantBuffer, ECPUAF_CanWrite, EGBU_Dynamic);
}

void Shader::UploadPerCameraConstants()
{
	void* buffer = mGraphicsDriver.MapBuffer(mPerCameraBuffer);
	memcpy(buffer, &mPerCamera, sizeof(mPerCamera));
	mGraphicsDriver.UnmapBuffer(mPerCameraBuffer);
}

void Shader::UploadPerObjectConstants()
{
	void* buffer = mGraphicsDriver.MapBuffer(mPerObjectBuffer);
	memcpy(buffer, &mPerObject, sizeof(mPerObject));
	mGraphicsDriver.UnmapBuffer(mPerObjectBuffer);
}

void Shader::UploadLightingConstants()
{
	void* buffer = mGraphicsDriver.MapBuffer(mLightingBuffer);
	memcpy(buffer, &mLighting, sizeof(mLighting));
	mGraphicsDriver.UnmapBuffer(mLightingBuffer);
}

void Shader::UploadMatrixPalette(const struct MatrixPalette& palette)
{
	void* buffer = mGraphicsDriver.MapBuffer(mMatrixPaletteBuffer);
	memcpy(buffer, &palette, sizeof(palette));
	mGraphicsDriver.UnmapBuffer(mMatrixPaletteBuffer);
}

void Shader::BindTexture(TexturePtr texture, int slot)
{
	texture->SetActive(slot);
}

bool Shader::Load(const char* fileName, class AssetCache* cache)
{
	// Compile vertex and pixel shaders
	if (!mGraphicsDriver.CompileShaderFromFile(fileName, "VS", "vs_4_0", mCompiledVS) ||
		!mGraphicsDriver.CompileShaderFromFile(fileName, "PS", "ps_4_0", mCompiledPS))
		return false;

	// Create vertex and pixel shaders
	mVertexShader = mGraphicsDriver.CreateVertexShader(mCompiledVS);
	mPixelShader = mGraphicsDriver.CreatePixelShader(mCompiledPS);

	// Create per-camera buffer
	mPerCameraBuffer = mGraphicsDriver.CreateGraphicsBuffer(
		&mPerCamera, sizeof(mPerCamera),
		EBF_ConstantBuffer, ECPUAF_CanWrite, EGBU_Dynamic);

	// Create per-object buffer
	mPerObjectBuffer = mGraphicsDriver.CreateGraphicsBuffer(
		&mPerObject, sizeof(mPerObject),
		EBF_ConstantBuffer, ECPUAF_CanWrite, EGBU_Dynamic);
	
	// Create sample state
	mDefaultSampler = mGraphicsDriver.CreateSamplerState();

	// Create lighting constant buffer
	mLightingBuffer = mGraphicsDriver.CreateGraphicsBuffer(
		&mLighting, sizeof(mLighting),
		EBF_ConstantBuffer, ECPUAF_CanWrite, EGBU_Dynamic);

	return true;
}
