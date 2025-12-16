#include "OpenGL/GLShader.h"

#include "Utils/Log.h"

#include "glad/gl.h"
#include <vector>

GLShader::GLShader(const char* const vertexSrc, const char* const fragmentSrc) :
	mId(0)
{
    Create(vertexSrc, fragmentSrc);
}

GLShader::~GLShader() {
    Delete();
}

void GLShader::Create(const char* const vertexSrc, const char* const fragmentSrc) {
    Delete();

    const uint32_t vertShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (!vertShader) {
		LOG_ERROR("Failed to compile vertex shader");
        return;
    }

    const uint32_t fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!fragShader) {
        glDeleteShader(vertShader);
		LOG_ERROR("Failed to compile fragment shader");
        return;
    }

    const uint32_t program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, std::data(infoLog));

        glDeleteProgram(program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        LOG_ERROR("Failed to link shader: {0}", std::data(infoLog));
    }

    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);

    mId = program;
}

uint32_t GLShader::CompileShader(const uint32_t type, const char* const src) {
    const GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
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

void GLShader::Delete() {
    if (mId) {
        glDeleteProgram(mId);
        mId = 0;
    }
}

void GLShader::Bind() const {
    glUseProgram(mId);
}

void GLShader::Unbind() const {
    glUseProgram(0);
}

void GLShader::SetVertexAttribf(const char* const name, int count, uint32_t stride, uint32_t offset) {
    const auto attribLoc = glGetAttribLocation(mId, name);
    if (attribLoc == -1) {
        LOG_ERROR("Failed to find attribute location for \"{0}\" in shader", name);
		return;
    }

    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, count, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
}
