#pragma once

#include "Types.h"

class OpenGLShader
{
public:
    OpenGLShader() = default;
    OpenGLShader(const char* const vertexSrc, const char* const fragmentSrc);
    ~OpenGLShader();

    void Create(const char* const vertexSrc, const char* const fragmentSrc);
    void Delete();

    void Bind() const;
    void Unbind() const;

    void SetVertexAttribf(
        const char* const name,
        int count = 1,
        uint32_t stride = 0,
        uint32_t offset = 0);

private:
    uint32_t CompileShader(const uint32_t type, const char* const src);

private:
    uint32_t mId = 0;
};
