#pragma once

#include <memory>

#include "OpenGL/GLBufferLayout.h"

class GLVertexBuffer {
public:
	GLVertexBuffer(const void* data, unsigned int size);
	~GLVertexBuffer();

	void Bind() const;
	void Unbind() const;

	void SetData(const void* data, unsigned int size);

	const GLBufferLayout &GetLayout() const { return mLayout; }
	void SetLayout(const GLBufferLayout &layout) { mLayout = layout; }

	static std::shared_ptr<GLVertexBuffer> Create(const void* const vertices, unsigned int size);

private:
	GLBufferLayout mLayout;
	unsigned int mId = 0;
};
