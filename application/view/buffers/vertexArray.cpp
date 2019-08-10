#include "vertexArray.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "vertexBuffer.h"
#include "bufferLayout.h"

#include "../util/log.h"

VertexArray::VertexArray() : attributeArrayOffset(0) {
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
}

VertexArray::~VertexArray() {
	close();
	Log::debug("Deleted vertex array with id (%d)", id);
}

void VertexArray::bind() {
	glBindVertexArray(id);
	for (unsigned int i = 0; i < attributeArrayOffset; i++)
		glEnableVertexAttribArray(i);
}

void VertexArray::unbind() {
	glBindVertexArray(0);
}

void VertexArray::addBuffer(VertexBuffer& buffer, const BufferLayout& layout) {
	bind();
	buffer.bind();

	size_t offset = 0;
	for (size_t i = 0; i < layout.elements.size(); i++) {
		auto& element = layout.elements[i];

		int iterations = (element.info == BufferDataType::MAT2 || element.info == BufferDataType::MAT3 || element.info == BufferDataType::MAT4)? element.info.count : 1;

		for (size_t j = 0; j < iterations; j++) {
			glEnableVertexAttribArray(attributeArrayOffset + i + j);
			glVertexAttribPointer(attributeArrayOffset + i + j, element.info.count, element.info.type, element.normalized, layout.stride, (const void*) offset);
			
			offset += element.info.size;

			if (j > 0) 
				attributeArrayOffset++;
		}
	}

	attributeArrayOffset += layout.elements.size();
}

void VertexArray::close() {
	unbind();
	glDeleteVertexArrays(1, &id);
}