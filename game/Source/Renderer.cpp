#include "ITPEnginePCH.h"
#include "SimdMath.h"

Renderer::Renderer(Game& game)
	:mGame(game)
	,mWindow(nullptr)
	,mWidth(0)
	,mHeight(0)
{

}

Renderer::~Renderer()
{
	// Clear components...
	mDrawComponents.clear();
	mComponents2D.clear();

	mDepthBuffer.reset();
	mSpriteDepthState.reset();
	mMeshDepthState.reset();
	mRasterState.reset();

	mSpriteBlendState.reset();
	mMeshBlendState.reset();

	mSpriteShader.reset();
	mSpriteVerts.reset();

	mMeshShaders.clear();

	// Shutdown the input cache and graphics driver
	mInputLayoutCache.reset();
	mGraphicsDriver.reset();

	if (mWindow != nullptr)
	{
		SDL_DestroyWindow(mWindow);
	}
}

bool Renderer::Init(int width, int height)
{
	// Create our SDL window
	mWindow = SDL_CreateWindow("ITP Engine 2 Demo!", 100, 100, width, height, 
		0);

	if (!mWindow)
	{
		SDL_Log("Could not create window.");
		return false;
	}

	mGraphicsDriver = std::make_shared<GraphicsDriver>(GetActiveWindow());
	mInputLayoutCache = std::make_shared<InputLayoutCache>();

	mWidth = width;
	mHeight = height;

	if (!InitFrameBuffer())
	{
		return false;
	}

	if (!InitShaders())
	{
		return false;
	}

	if (!InitSpriteVerts())
	{
		return false;
	}

	return true;
}

void Renderer::RenderFrame()
{
	Clear();
	DrawComponents();
	Present();
}

void Renderer::AddComponent(DrawComponentPtr component)
{
	if (IsA<SpriteComponent>(component) || IsA<FontComponent>(component))
	{
		mComponents2D.emplace(component);
	}
	else
	{
		mDrawComponents.emplace(component);
	}
}

void Renderer::RemoveComponent(DrawComponentPtr component)
{
	if (IsA<SpriteComponent>(component) || IsA<FontComponent>(component))
	{
		auto iter = mComponents2D.find(component);
		if (iter != mComponents2D.end())
		{
			mComponents2D.erase(iter);
		}
	}
	else
	{
		auto iter = mDrawComponents.find(component);
		if (iter != mDrawComponents.end())
		{
			mDrawComponents.erase(iter);
		}
	}
}

void Renderer::AddPointLight(PointLightComponentPtr light)
{
	mPointLights.insert(light);
	UpdatePointLights();
}

void Renderer::RemovePointLight(PointLightComponentPtr light)
{
	mPointLights.erase(light);
	UpdatePointLights();
}

void Renderer::UpdatePointLights()
{
	for (auto meshShader : mMeshShaders)
	{
		int i = 0;

		// Update point lights data stored in every mesh shader
		for (auto pointLight : mPointLights)
		{
			meshShader.second->GetLightingConstants().mPointLights[i] = pointLight->GetData();
			i++;
		}

		// If less than 8 point lights, set the extra indices in the array as enabled = false
		for ( ; i < 8; i++)
		{
			meshShader.second->GetLightingConstants().mPointLights[i] = PointLightData();
		}

		// Upload the lighting constants
		meshShader.second->UploadLightingConstants();
	}
}

void Renderer::DrawSprite(TexturePtr texture, const Matrix4& worldTransform)
{
	// Set sprite shader active
	mSpriteShader->SetActive();

	// Set world transform for sprite shader
	mSpriteShader->GetPerObjectConstants().mWorldTransform = worldTransform;

	// Upload world transform for sprite shader to GPU
	mSpriteShader->UploadPerObjectConstants();

	// Bind texture
	mSpriteShader->BindTexture(texture, 0);

	// Draw sprite vertices
	DrawVertexArray(mSpriteVerts);
}

void Renderer::DrawMesh(VertexArrayPtr vertArray, TexturePtr texture, const Matrix4& worldTransform, EMeshShader type)
{
	// Set active
	mMeshShaders[type]->SetActive();

	// Set world transform for basic mesh shader
	mMeshShaders[type]->GetPerObjectConstants().mWorldTransform = worldTransform;

	// Upload world transform for basic mesh shader to GPU
	mMeshShaders[type]->UploadPerObjectConstants();

	// Bind texture
	mMeshShaders[type]->BindTexture(texture, 0);

	// Draw vertex array
	DrawVertexArray(vertArray);
}

void Renderer::DrawSkeletalMesh(VertexArrayPtr vertArray, TexturePtr texture, const Matrix4& worldTransform, const struct MatrixPalette& palette)
{
	// Set active
	mMeshShaders[EMS_Skinned]->SetActive();

	// Set world transform for skinned mesh shader to GPU
	mMeshShaders[EMS_Skinned]->GetPerObjectConstants().mWorldTransform = worldTransform;

	// Upload world transform for skinned mesh shader to GPU
	mMeshShaders[EMS_Skinned]->UploadPerObjectConstants();

	// Upload matrix palette for skinned mesh shader to GPU
	mMeshShaders[EMS_Skinned]->UploadMatrixPalette(palette);

	// Bind texture
	mMeshShaders[EMS_Skinned]->BindTexture(texture, 0);

	// Draw vertex array
	DrawVertexArray(vertArray);
}

void Renderer::DrawVertexArray(VertexArrayPtr vertArray)
{
	// Set vertex array as active
	vertArray->SetActive();

	// Draw indexed vertices
	mGraphicsDriver->DrawIndexed(vertArray->GetIndexCount(), 0, 0);
}

void Renderer::UpdateViewMatrix(const Matrix4& view)
{
	// Set view
	mView = view;

	for (auto meshShader : mMeshShaders)
	{
		// Get view projection matrix
		Matrix4 viewProjection = mView * mProj;

		// Set per-camera constants
		meshShader.second->GetPerCameraConstants().mViewProjection = viewProjection;
		meshShader.second->GetPerCameraConstants().mWorldCameraPosition = mView.GetTranslation();
	}
}

void Renderer::SetAmbientLight(const Vector3& color)
{
	for (auto meshShader : mMeshShaders)
	{
		meshShader.second->GetLightingConstants().mAmbientLight = color;
		meshShader.second->UploadLightingConstants();
	}
}

Vector3 Renderer::Unproject(const Vector3& screenPoint) const
{
	// Convert screenPoint to device coordinates (between -1 and +1)
	Vector3 deviceCoord = screenPoint;
	deviceCoord.x /= (mWidth) * 0.5f;
	deviceCoord.y /= (mHeight) * 0.5f;

	// First, undo the projection
	Matrix4 unprojection = mProj;
	unprojection.Invert();
	Vector3 unprojVec = TransformWithPerspDiv(deviceCoord, unprojection);

	// Now undo the view matrix
	Matrix4 uncamera = mView;
	uncamera.Invert();
	return Transform(unprojVec, uncamera);
}

void Renderer::Clear()
{
	// Clear the back buffer black with an alpha of 1
	mGraphicsDriver->ClearBackBuffer(Vector3(), 1.0f);

	// Clear depth stencil
	mGraphicsDriver->ClearDepthStencil(mDepthBuffer, 1.0f);
}

void Renderer::DrawComponents()
{
	// Enable depth buffering and disable blending
	mGraphicsDriver->SetDepthStencilState(mMeshDepthState);
	mGraphicsDriver->SetBlendState(mMeshBlendState);

	// Upload per camera constants
	for (auto meshShader : mMeshShaders)
	{
		meshShader.second->UploadPerCameraConstants();
	}

	// Update point lights
	UpdatePointLights();

	// Draw the normal components
	for (auto& comp : mDrawComponents)
	{
		if (comp->IsVisible())
		{
			comp->Draw(*this);
		}
	}

	// Disable depth buffering and enable blending
	mGraphicsDriver->SetDepthStencilState(mSpriteDepthState);
	mGraphicsDriver->SetBlendState(mSpriteBlendState);

	// Draw the 2D components
	for (auto& comp : mComponents2D)
	{
		if (comp->IsVisible())
		{
			comp->Draw(*this);
		}
	}
}

void Renderer::Present()
{
	mGraphicsDriver->Present();
}

bool Renderer::InitFrameBuffer()
{
	// Render solid
	mRasterState = mGraphicsDriver->CreateRasterizerState(EFM_Solid);
	mGraphicsDriver->SetRasterizerState(mRasterState);

	// Create depth buffer
	mDepthBuffer = mGraphicsDriver->CreateDepthStencil(mWidth, mHeight);
	
	// Set depth stencil
	mGraphicsDriver->SetDepthStencil(mDepthBuffer);
	
	// Create depth stencil states
	mMeshDepthState = mGraphicsDriver->CreateDepthStencilState(true, ECF_Less);
	mSpriteDepthState = mGraphicsDriver->CreateDepthStencilState(false, ECF_Always);

	// Create blend states
	mMeshBlendState = mGraphicsDriver->CreateBlendState(false);
	mSpriteBlendState = mGraphicsDriver->CreateBlendState(true);

	return true;
}

bool Renderer::InitShaders()
{
	// Load sprite shader
	{
		mSpriteShader = mGame.GetAssetCache().Load<Shader>("Shaders/Sprite.hlsl");
		if (!mSpriteShader)
			return false;

		// Array of InputLayoutElements for sprite shader
		InputLayoutElement spriteShaderILE[] = {
			{ "POSITION", 0, EGF_R32G32B32_Float, 0 },
			{ "TEXCOORD", 0, EGF_R32G32_Float, sizeof(float) * 3 },
		};

		// Create and register input layout as "positiontexcoord"
		mInputLayoutCache->RegisterLayout("positiontexcoord",
			mGraphicsDriver->CreateInputLayout(
				spriteShaderILE, 2,
				mSpriteShader->GetCompiledVS()
			)
		);

		// Set view-projection matrix for sprite shader
		mSpriteShader->GetPerCameraConstants().mViewProjection =
			Matrix4::CreateOrtho(static_cast<float>(mWidth), static_cast<float>(mHeight), 1000.0f, -1000.0f);

		// Upload sprite shader view-projection matrix to GPU
		mSpriteShader->UploadPerCameraConstants();
	}

	// Load basic mesh shader
	{
		// Set projection matrix
		mProj = Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.0f),
			static_cast<float>(mWidth), static_cast<float>(mHeight), 25.0f, 10000.0f);

		// Load basic mesh shader
		mMeshShaders[EMS_Basic] = mGame.GetAssetCache().Load<Shader>("Shaders/BasicMesh.hlsl");

		// Array of InputLayoutElements for basic mesh shader
		InputLayoutElement basicMeshShaderILE[] = {
			{ "POSITION", 0, EGF_R32G32B32_Float, 0 },
			{ "NORMAL", 0, EGF_R32G32B32_Float, sizeof(float) * 3 },
			{ "TEXCOORD", 0, EGF_R32G32_Float, sizeof(float) * 6 },
		};

		// Create and register input layout as "positionnormaltexcoord"
		mInputLayoutCache->RegisterLayout("positionnormaltexcoord",
			mGraphicsDriver->CreateInputLayout(
				basicMeshShaderILE, 3,
				mMeshShaders[EMS_Basic]->GetCompiledVS()
			)
		);
	}

	// Load phong shader
	{
		// Set projection matrix
		mProj = Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.0f),
			static_cast<float>(mWidth), static_cast<float>(mHeight), 25.0f, 10000.0f);

		// Load basic mesh shader
		mMeshShaders[EMS_Phong] = mGame.GetAssetCache().Load<Shader>("Shaders/Phong.hlsl");

		// Array of InputLayoutElements for basic mesh shader
		InputLayoutElement phongShaderILE[] = {
			{ "POSITION", 0, EGF_R32G32B32_Float, 0 },
			{ "NORMAL", 0, EGF_R32G32B32_Float, sizeof(float) * 3 },
			{ "TEXCOORD", 0, EGF_R32G32_Float, sizeof(float) * 6 },
		};

		// Create and register input layout as "positionnormaltexcoord"
		mInputLayoutCache->RegisterLayout("positionnormaltexcoord",
			mGraphicsDriver->CreateInputLayout(
				phongShaderILE, 3,
				mMeshShaders[EMS_Phong]->GetCompiledVS()
			)
		);
	}

	// Load skinned mesh and create appropriate input layout
	{
		// Set projection matrix
		mProj = Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.0f),
			static_cast<float>(mWidth), static_cast<float>(mHeight), 25.0f, 10000.0f);

		// Load skinned mesh shader
		mMeshShaders[EMS_Skinned] = mGame.GetAssetCache().Load<Shader>("Shaders/Skinned.hlsl");
		mMeshShaders[EMS_Skinned]->CreateMatrixPaletteBuffer();

		// Array of InputLayoutElements for skinned mesh shader
		InputLayoutElement skinnedShaderILE[] = {
			{ "POSITION", 0, EGF_R32G32B32_Float, 0 },
			{ "NORMAL", 0, EGF_R32G32B32_Float, sizeof(float) * 3 },
			{ "BONES", 0, EGF_R8G8B8A8_UInt, sizeof(float) * 6 },
			{ "WEIGHTS", 0, EGF_R8G8B8A8_UNorm, sizeof(float) * 6 + sizeof(char) * 4 },
			{ "TEXCOORD", 0, EGF_R32G32_Float, sizeof(float) * 6 + sizeof(char) * 8 },
		};

		// Create and register input layout as "positionnormalbonesweightstexcoord"
		mInputLayoutCache->RegisterLayout("positionnormalbonesweightstexcoord",
			mGraphicsDriver->CreateInputLayout(
				skinnedShaderILE, 5,
				mMeshShaders[EMS_Skinned]->GetCompiledVS()
			)
		);
	}

	return true;
}

bool Renderer::InitSpriteVerts()
{
	// Create the vertex array for sprites
	float verts[] =
	{
		-0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  // top left
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f,   // top right
		0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom left
	};

	uint16_t indices[] =
	{
		0, 1, 2, // <top left, top right, bottom right>
		2, 3, 0, // <bottom right, bottom left, top left>
	};

	mSpriteVerts = VertexArray::Create(GetGraphicsDriver(), GetInputLayoutCache(),
		verts, 4, 20, "positiontexcoord", indices, 6);

	return true;
}
