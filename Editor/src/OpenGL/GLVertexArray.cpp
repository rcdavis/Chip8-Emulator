#include "OpenGL/GLVertexArray.h"

#include "glad/gl.h"

#include "OpenGL/GLVertexBuffer.h"
#include "OpenGL/GLIndexBuffer.h"

#include <cassert>

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
	switch (type)
	{
	case ShaderDataType::Float:
	case ShaderDataType::Float2:
	case ShaderDataType::Float3:
	case ShaderDataType::Float4:
	case ShaderDataType::Mat3:
	case ShaderDataType::Mat4:
		return GL_FLOAT;
	case ShaderDataType::Int:
	case ShaderDataType::Int2:
	case ShaderDataType::Int3:
	case ShaderDataType::Int4:
		return GL_INT;
	case ShaderDataType::UInt:
	case ShaderDataType::UInt2:
	case ShaderDataType::UInt3:
	case ShaderDataType::UInt4:
		return GL_UNSIGNED_INT;
	case ShaderDataType::Bool:
		return GL_BOOL;
	}

	assert(false && "Unknown shader data type");
	return 0;
}

GLVertexArray::GLVertexArray() {
	glGenVertexArrays(1, &mId);
}

GLVertexArray::~GLVertexArray() {
	glDeleteVertexArrays(1, &mId);
}

void GLVertexArray::Bind() const {
	glBindVertexArray(mId);
}

void GLVertexArray::Unbind() const {
	glBindVertexArray(0);
}

void GLVertexArray::AddVertexBuffer(const std::shared_ptr<GLVertexBuffer> &vertexBuffer) {
	assert(!std::empty(vertexBuffer->GetLayout().GetElements()) && "Vertex buffer does not have a layout");

	glBindVertexArray(mId);
	vertexBuffer->Bind();

	const auto &layout = vertexBuffer->GetLayout();
	for (const auto &element : layout) {
		switch (element.type)
		{
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		{
			const unsigned int componentCount = element.GetComponentCount();
			for (unsigned int i = 0; i < componentCount; ++i)
			{
				glEnableVertexAttribArray(mVertexBufferIndex);
				glVertexAttribPointer(
					mVertexBufferIndex,
					componentCount,
					ShaderDataTypeToOpenGLBaseType(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void *)(intptr_t)(element.offset + (sizeof(float) * componentCount * i)));
				glVertexAttribDivisor(mVertexBufferIndex, 1);
				++mVertexBufferIndex;
			}
		}
		break;

		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
		{
			glEnableVertexAttribArray(mVertexBufferIndex);
			glVertexAttribPointer(
				mVertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void *)(intptr_t)element.offset);
			++mVertexBufferIndex;
		}
		break;

		default:
		{
			glEnableVertexAttribArray(mVertexBufferIndex);
			glVertexAttribIPointer(
				mVertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.type),
				layout.GetStride(),
				(const void *)(intptr_t)element.offset);
			++mVertexBufferIndex;
		}
		break;
		}
	}

	mVertexBuffers.push_back(vertexBuffer);
}

void GLVertexArray::SetIndexBuffer(const std::shared_ptr<GLIndexBuffer> &indexBuffer) {
	glBindVertexArray(mId);
	indexBuffer->Bind();

	mIndexBuffer = indexBuffer;
}

std::shared_ptr<GLVertexArray> GLVertexArray::Create() {
	return std::make_shared<GLVertexArray>();
}
