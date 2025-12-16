#pragma once

#include "Types.h"

class OpenGLTexture
{
public:
    OpenGLTexture() = default;
    ~OpenGLTexture();

    void Create(uint32_t width, uint32_t height);
    void Destroy();

    void Bind();
    void Unbind();

    void Update(uint32_t width, uint32_t height, const char* const pixels);

private:
    uint32_t mId = 0;
};
