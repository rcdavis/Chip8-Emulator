#include "OpenGL/GLTexture.h"

#include "glad/gl.h"
#include "stb_image.h"

#include "Utils/Log.h"

GLTexture::GLTexture() {
	glGenTextures(1, &mId);
}

GLTexture::GLTexture(const std::filesystem::path &filepath) {
	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	stbi_uc *const data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
	if (!data) {
		LOG_ERROR("Failed to load image {0}", filepath.filename().string());
		return;
	}

	mIsLoaded = true;

	mWidth = width;
	mHeight = height;

	if (channels == 4) {
		mInternalFormat = GL_RGBA8;
		mDataFormat = GL_RGBA;
	} else if (channels == 3) {
		mInternalFormat = GL_RGB8;
		mDataFormat = GL_RGB;
	}

	assert(mInternalFormat & mDataFormat && "Format not supported");

	glGenTextures(1, &mId);
	glBindTexture(GL_TEXTURE_2D, mId);
	glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, mDataFormat, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

GLTexture::~GLTexture() {
	glDeleteTextures(1, &mId);
}

void GLTexture::Bind(unsigned int slot) const {
	glBindTexture(GL_TEXTURE_2D, mId);
}

void GLTexture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool GLTexture::operator==(const GLTexture &texture) const {
	if (this == &texture)
		return true;

	return mId == texture.mId;
}

std::shared_ptr<GLTexture> GLTexture::Load(const std::filesystem::path &filepath) {
	return std::make_shared<GLTexture>(filepath);
}
