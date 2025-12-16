#pragma once

#include <memory>

class GLIndexBuffer {
public:
	GLIndexBuffer(const unsigned short* indices, unsigned int count);
	~GLIndexBuffer();

	void Bind() const;
	void Unbind() const;

	unsigned int GetCount() const { return mCount; }

	static std::shared_ptr<GLIndexBuffer> Create(const unsigned short* indices, unsigned int count);

private:
	unsigned int mId = 0;
	unsigned int mCount = 0;
};
