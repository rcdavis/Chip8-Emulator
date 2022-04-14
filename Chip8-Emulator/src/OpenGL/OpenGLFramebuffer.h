#pragma once

#include "Types.h"
#include "Log.h"

class OpenGLFramebuffer
{
public:
    OpenGLFramebuffer() = default;
    OpenGLFramebuffer(const uint32_t width, const uint32_t height);
    ~OpenGLFramebuffer();

    void Create(const uint32_t width, const uint32_t height);
    void Destroy();

    void Invalidate();

    void Bind();
    void Unbind();

    void Resize(const uint32_t width, const uint32_t height);

    void ClearAttachment(const int32_t value);

    uint32_t GetColorAttachmentRendererId() const { return mColorAttachment; }

private:
    uint32_t mId = 0;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    uint32_t mColorAttachment = 0;
};
