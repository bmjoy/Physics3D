#include "core.h"

#include "pointMesh.h"

#include "buffers/vertexArray.h"
#include "buffers/vertexBuffer.h"
#include "renderer.h"

namespace Graphics {

PointMesh::PointMesh(const float* vertices, const size_t vertexCount, size_t capacity) : AbstractMesh(Renderer::POINT), vertexCount(vertexCount), capacity(capacity) {
	vertexBuffer = new VertexBuffer(nullptr, 10 * capacity * sizeof(float), Renderer::DYNAMIC_DRAW);

	vertexBufferLayout = {
		{
			{ "vposition", BufferDataType::FLOAT3 },
			{ "vsize", BufferDataType::FLOAT },
			{ "vcolor1", BufferDataType::FLOAT3 },
			{ "vcolor2", BufferDataType::FLOAT3 }
		}
	};

	vao->addBuffer(vertexBuffer, vertexBufferLayout);
}

void PointMesh::render() {
	vao->bind();
	Renderer::drawArrays(renderMode, 0, vertexCount);
}

void PointMesh::close() {
	vertexBuffer->close();
	vao->close();
}

void PointMesh::update(const float* vertices, const size_t vertexCount) {
	vertexBuffer->bind();
	this->vertexCount = vertexCount;
	if (vertexCount > capacity) {
		capacity = vertexCount;
		Log::warn("Point buffer overflow, creating new buffer with size (%d)", vertexCount);
		vertexBuffer->fill(vertices, capacity * vertexBufferLayout.stride * sizeof(float), Graphics::Renderer::DYNAMIC_DRAW);
	} else {
		vertexBuffer->update(vertices, vertexCount * vertexBufferLayout.stride * sizeof(float), 0);
	}
}

};