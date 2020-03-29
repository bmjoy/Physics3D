#include "core.h"

#include "indexedMesh.h"

#include "buffers/indexBuffer.h"
#include "buffers/vertexBuffer.h"
#include "buffers/vertexArray.h"
#include "visualShape.h"
#include "renderer.h"
#include "../physics/geometry/shape.h"

namespace Graphics {

IndexedMesh::IndexedMesh(const VisualShape& shape) : AbstractMesh(), vertexCount(shape.vertexCount), triangleCount(shape.triangleCount) {
	float* vertices = new float[shape.vertexCount * 3];
	for (int i = 0; i < vertexCount; i++) {
		Vec3f vertex = shape[i];
		vertices[i * 3] = vertex.x;
		vertices[i * 3 + 1] = vertex.y;
		vertices[i * 3 + 2] = vertex.z;
	}
	unsigned int* triangles = new unsigned int[shape.triangleCount * 3];
	for (int i = 0; i < triangleCount; i++) {
		Triangle triangle = shape.getTriangle(i);
		triangles[i * 3] = triangle.firstIndex;
		triangles[i * 3 + 1] = triangle.secondIndex;
		triangles[i * 3 + 2] = triangle.thirdIndex;
	}

	vertexBuffer = new VertexBuffer(vertices, 3 * vertexCount * sizeof(float));
	normalBuffer = new VertexBuffer(reinterpret_cast<float const*>(shape.normals.get()), 3 * vertexCount * sizeof(float));
	uvBuffer = new VertexBuffer(reinterpret_cast<float const*>(shape.uvs.get()), 2 * vertexCount * sizeof(float));

	indexBuffer = new IndexBuffer(triangles, 3 * triangleCount);

	vertexBufferLayout = {
		{{ "vposition", BufferDataType::FLOAT3 }}
	};

	normalBufferLayout = {
		{{ "vnormal", BufferDataType::FLOAT3 }}
	};

	uvBufferLayout = {
		{{ "vUV", BufferDataType::FLOAT2 }}
	};

	vao->addBuffer(vertexBuffer, vertexBufferLayout);
	vao->addBuffer(normalBuffer, normalBufferLayout);
	vao->addBuffer(uvBuffer, uvBufferLayout);
}

IndexedMesh::IndexedMesh(const float* vertices, const float* normals, const float* uvs, const unsigned int* indices, const int vertexCount, const int triangleCount) : AbstractMesh(), vertexCount(vertexCount), triangleCount(triangleCount) {
	vertexBuffer = new VertexBuffer(vertices, 3 * vertexCount * sizeof(float));
	normalBuffer = new VertexBuffer(normals, 3 * vertexCount * sizeof(float));
	uvBuffer = new VertexBuffer(uvs, 2 * vertexCount * sizeof(float));

	indexBuffer = new IndexBuffer(indices, 3 * triangleCount);

	vertexBufferLayout = {
		{{ "vposition", BufferDataType::FLOAT3 }}
	};

	normalBufferLayout = {
		{{ "vnormal", BufferDataType::FLOAT3 }}
	};

	uvBufferLayout = {
		{{ "vUV", BufferDataType::FLOAT2 }}
	};

	vao->addBuffer(vertexBuffer, vertexBufferLayout);
	vao->addBuffer(normalBuffer, normalBufferLayout);
	vao->addBuffer(uvBuffer, uvBufferLayout);
}

IndexedMesh::~IndexedMesh() {
	close();
}

IndexedMesh::IndexedMesh(IndexedMesh&& other) {
	vao = other.vao;
	vertexBufferLayout = other.vertexBufferLayout;
	normalBufferLayout = other.normalBufferLayout;
	uvBufferLayout = other.uvBufferLayout;
	uniformBufferLayout = other.uniformBufferLayout;
	indexBuffer = other.indexBuffer;
	vertexBuffer = other.vertexBuffer;
	normalBuffer = other.normalBuffer;
	uvBuffer = other.uvBuffer;
	uniformBuffer = other.uniformBuffer;
	vertexCount = other.vertexCount;
	triangleCount = other.triangleCount;

	// Reset so they cant be deleted by close()
	other.vao = nullptr;
	other.uvBuffer = nullptr;
	other.indexBuffer = nullptr;
	other.vertexBuffer = nullptr;
	other.normalBuffer = nullptr;
	other.uniformBuffer = nullptr;

	other.close();
}

IndexedMesh& IndexedMesh::operator=(IndexedMesh&& other) {
	if (this != &other) {
		close();
		std::swap(vao, other.vao);
		std::swap(vertexBufferLayout, other.vertexBufferLayout);
		std::swap(normalBufferLayout, other.normalBufferLayout);
		std::swap(uvBufferLayout, other.uvBufferLayout);
		std::swap(uniformBufferLayout, other.uniformBufferLayout);
		std::swap(vertexBuffer, other.vertexBuffer);
		std::swap(normalBuffer, other.normalBuffer);
		std::swap(uvBuffer, other.uvBuffer);
		std::swap(uniformBuffer, other.uniformBuffer);
		std::swap(indexBuffer, other.indexBuffer);
		std::swap(vertexCount, other.vertexCount);
		std::swap(triangleCount, other.triangleCount);
	}

	return *this;
}

void IndexedMesh::render() {
	render(Renderer::FILL);
}

void IndexedMesh::addUniformBuffer(VertexBuffer* uniformBuffer, const BufferLayout& uniformBufferLayout) {
	this->uniformBuffer = uniformBuffer;
	this->uniformBufferLayout = uniformBufferLayout;

	vao->addBuffer(uniformBuffer, uniformBufferLayout);
}

void IndexedMesh::updateUniformBuffer(const void* data, size_t sizeInBytes, int offset) {
	if (uniformBuffer) {
		uniformBuffer->update(data, sizeInBytes, offset);
	}
}

void IndexedMesh::render(GLFLAG mode) {
	vao->bind();
	indexBuffer->bind();

	Renderer::polygonMode(Renderer::FRONT_AND_BACK, mode);
	Renderer::drawElements(renderMode, triangleCount * 3, Renderer::UINT, nullptr);
	Renderer::polygonMode(Renderer::FRONT_AND_BACK, Renderer::FILL);
}

void IndexedMesh::renderInstanced(size_t primitives) {
	renderInstanced(Renderer::FILL, primitives);
}

void IndexedMesh::renderInstanced(GLFLAG mode, size_t primitives) {
	vao->bind();
	indexBuffer->bind();

	Renderer::polygonMode(Renderer::FRONT_AND_BACK, mode);
	Renderer::drawElementsInstanced(renderMode, triangleCount * 3, Renderer::UINT, nullptr, primitives);
	Renderer::polygonMode(Renderer::FRONT_AND_BACK, Renderer::FILL);
}

void IndexedMesh::close() {
	if (vertexBuffer)
		vertexBuffer->close();
	if (indexBuffer)
		indexBuffer->close();
	if (normalBuffer)
		normalBuffer->close();
	if (uvBuffer)
		uvBuffer->close();
	if (uniformBuffer)
		uniformBuffer->close();
	if (vao)
		vao->close();

	vao = nullptr;
	uvBuffer = nullptr;
	indexBuffer = nullptr;
	vertexBuffer = nullptr;
	normalBuffer = nullptr;
	uniformBuffer = nullptr;
	uvBufferLayout = BufferLayout();
	normalBufferLayout = BufferLayout();
	vertexBufferLayout = BufferLayout();
	uniformBufferLayout = BufferLayout();
	triangleCount = 0;
	vertexCount = 0;
}

};