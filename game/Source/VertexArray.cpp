#include "ITPEnginePCH.h"

VertexArrayPtr VertexArray::Create(GraphicsDriver& graphics, InputLayoutCache& inputCache, 
	const void* verts, size_t vertCount, size_t vertSize, const std::string& inputLayoutName,
	const void* indices, size_t indexCount, size_t indexSize)
{
	return std::make_shared<VertexArray>(graphics, inputCache, 
		verts, vertCount, vertSize, inputLayoutName, 
		indices, indexCount, indexSize);
}

VertexArray::VertexArray(GraphicsDriver& graphics, InputLayoutCache& inputCache,
	const void* verts, size_t vertCount, size_t vertSize, const std::string& inputLayoutName,
	const void* indices, size_t indexCount, size_t indexSize)
	:mGraphics(graphics)
{
	mVertexCount = vertCount;
	mVertexSize = vertSize;
	mIndexCount = indexCount;

	// Get layout from input cache and save it in mInputLayout
	mInputLayout = inputCache.GetLayout(inputLayoutName);

	// Create graphics buffer for mVertexBuffer
	mVertexBuffer = mGraphics.CreateGraphicsBuffer(
		verts, vertSize * vertCount,
		EBF_VertexBuffer,
		ECPUAF_Neither,
		EGBU_Immutable
	);

	// Create graphics buffer for mIndexBuffer
	mIndexBuffer = mGraphics.CreateGraphicsBuffer(
		indices,
		indexCount * indexSize,
		EBF_IndexBuffer,
		ECPUAF_Neither,
		EGBU_Immutable
	);
}

VertexArray::~VertexArray()
{

}

void VertexArray::SetActive()
{
	mGraphics.SetInputLayout(mInputLayout);
	mGraphics.SetVertexBuffer(mVertexBuffer, mVertexSize);
	mGraphics.SetIndexBuffer(mIndexBuffer);
}
