#pragma once

#include <filesystem>

class GLTexture {
public:
	GLTexture();
	GLTexture(const std::filesystem::path &filepath);
	~GLTexture();

	unsigned int GetId() const { return mId; }
	unsigned int GetWidth() const { return mWidth; }
	unsigned int GetHeight() const { return mHeight; }

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	bool IsLoaded() const { return mIsLoaded; }

	bool operator==(const GLTexture &texture) const;

	static std::shared_ptr<GLTexture> Load(const std::filesystem::path &filepath);

private:
	unsigned int mId = 0;
	unsigned int mWidth = 0;
	unsigned int mHeight = 0;
	unsigned int mInternalFormat = 0;
	unsigned int mDataFormat = 0;
	bool mIsLoaded = false;
};
