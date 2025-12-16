#include "OpenGL/GLVertexBuffer.h"

#include "glad/gl.h"

GLVertexBuffer::GLVertexBuffer(const void* data, unsigned int size) {
	glGenBuffers(1, &mId);
	glBindBuffer(GL_ARRAY_BUFFER, mId);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer() {
	glDeleteBuffers(1, &mId);
}

void GLVertexBuffer::Bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, mId);
}

void GLVertexBuffer::Unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::SetData(const void* data, unsigned int size) {
	glBindBuffer(GL_ARRAY_BUFFER, mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

std::shared_ptr<GLVertexBuffer> GLVertexBuffer::Create(const void* data, unsigned int size) {
	return std::make_shared<GLVertexBuffer>(data, size);
}
