#include "OpenGL/GLIndexBuffer.h"

#include "glad/gl.h"

GLIndexBuffer::GLIndexBuffer(const unsigned short* indices, unsigned int count) : mCount(count) {
	glGenBuffers(1, &mId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * count, indices, GL_STATIC_DRAW);
}

GLIndexBuffer::~GLIndexBuffer() {
	glDeleteBuffers(1, &mId);
}

void GLIndexBuffer::Bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
}

void GLIndexBuffer::Unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::shared_ptr<GLIndexBuffer> GLIndexBuffer::Create(const unsigned short* indices, unsigned int count) {
	return std::make_shared<GLIndexBuffer>(indices, count);
}
