#include "OpenGLShader.h"

#include "Log.h"

#include <glad/glad.h>
#include <vector>

OpenGLShader::OpenGLShader(const char* const vertexSrc, const char* const fragmentSrc)
{
    Create(vertexSrc, fragmentSrc);
}

OpenGLShader::~OpenGLShader()
{
    Delete();
}

void OpenGLShader::Create(const char* const vertexSrc, const char* const fragmentSrc)
{
    Delete();

    const uint32_t program = glCreateProgram();

    const uint32_t vertShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (!vertShader)
    {
        glDeleteProgram(program);
        MAKE_ASSERT(false, "Failed to compile vertex shader");
        return;
    }

    const uint32_t fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!fragShader)
    {
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        MAKE_ASSERT(false, "Failed to compile fragment shader");
        return;
    }

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, std::data(infoLog));

        glDeleteProgram(program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        LOG_ERROR("Failed to link shader: {}", std::data(infoLog));
        MAKE_ASSERT(false, "Shader link failure");
    }

    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);

    mId = program;
}

uint32_t OpenGLShader::CompileShader(const uint32_t type, const char* const src)
{
    const GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, std::data(infoLog));

        glDeleteShader(shader);

        LOG_ERROR("Failed to compile shader: {}", std::data(infoLog));
        return 0;
    }

    return shader;
}

void OpenGLShader::Delete()
{
    if (mId)
    {
        glDeleteProgram(mId);
        mId = 0;
    }
}

void OpenGLShader::Bind() const
{
    glUseProgram(mId);
}

void OpenGLShader::Unbind() const
{
    glUseProgram(0);
}

void OpenGLShader::SetVertexAttribf(
    const char* const name,
    int count,
    uint32_t stride,
    uint32_t offset)
{
    const auto attribLoc = glGetAttribLocation(mId, name);
    if (attribLoc == -1)
    {
        LOG_ERROR("Failed to find attribute location for \"{}\" in shader", name);
        MAKE_ASSERT(false, "SetVertexAttribf failed");
    }

    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, count, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
}
