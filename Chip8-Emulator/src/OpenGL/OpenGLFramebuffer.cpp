#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

static constexpr uint32_t s_MaxFramebufferSize = 8192;

OpenGLFramebuffer::OpenGLFramebuffer(const uint32_t width, const uint32_t height) :
    mId(0),
    mWidth(width),
    mHeight(height),
    mColorAttachment(0)
{
    Invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    Destroy();
}

void OpenGLFramebuffer::Create(const uint32_t width, const uint32_t height)
{
    mWidth = width;
    mHeight = height;

    Invalidate();
}

void OpenGLFramebuffer::Destroy()
{
    if (mId)
    {
        glDeleteFramebuffers(1, &mId);
        glDeleteTextures(1, &mColorAttachment);
        mId = 0;
        mColorAttachment = 0;
    }
}

void OpenGLFramebuffer::Invalidate()
{
    Destroy();

    glCreateFramebuffers(1, &mId);
    glBindFramebuffer(GL_FRAMEBUFFER, mId);

    glCreateTextures(GL_TEXTURE_2D, 1, &mColorAttachment);
    glBindTexture(GL_TEXTURE_2D, mColorAttachment);

    // TODO: Fix hard coded width and height
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment, 0);

    constexpr GLenum buffers[1] = {
        GL_COLOR_ATTACHMENT0
    };
    glDrawBuffers(1, std::data(buffers));

    MAKE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mId);
    glViewport(0, 0, mWidth, mHeight);
}

void OpenGLFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height)
{
    if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
    {
        LOG_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
        return;
    }

    mWidth = width;
    mHeight = height;

    Invalidate();
}

void OpenGLFramebuffer::ClearAttachment(const int32_t value)
{
    glClearTexImage(mColorAttachment, 0, GL_RGBA8, GL_INT, &value);
}
