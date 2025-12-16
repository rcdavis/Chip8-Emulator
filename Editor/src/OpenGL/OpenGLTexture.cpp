#include "OpenGL/OpenGLTexture.h"

#include "glad/gl.h"

OpenGLTexture::~OpenGLTexture() {
	Destroy();
}

void OpenGLTexture::Create(uint32_t width, uint32_t height) {
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &mId);
	glBindTexture(GL_TEXTURE_2D, mId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void OpenGLTexture::Destroy() {
	if (mId) {
		glDeleteTextures(1, &mId);
		mId = 0;
	}
}

void OpenGLTexture::Bind() {
	glBindTexture(GL_TEXTURE_2D, mId);
}

void OpenGLTexture::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLTexture::Update(uint32_t width, uint32_t height, const char* const pixels) {
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
