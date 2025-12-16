#pragma once

#include <vector>
#include <memory>

class GLVertexBuffer;
class GLIndexBuffer;

class GLVertexArray {
public:
	GLVertexArray();
	~GLVertexArray();

	void Bind() const;
	void Unbind() const;

	void AddVertexBuffer(const std::shared_ptr<GLVertexBuffer>& vertexBuffer);
	void SetIndexBuffer(const std::shared_ptr<GLIndexBuffer>& indexBuffer);

	const std::vector<std::shared_ptr<GLVertexBuffer>>& GetVertexBuffers() const { return mVertexBuffers; }
	const std::shared_ptr<GLIndexBuffer>& GetIndexBuffer() const { return mIndexBuffer; }

	static std::shared_ptr<GLVertexArray> Create();

private:
	std::vector<std::shared_ptr<GLVertexBuffer>> mVertexBuffers;
	std::shared_ptr<GLIndexBuffer> mIndexBuffer;

	unsigned int mId = 0;
	unsigned int mVertexBufferIndex = 0;
};
