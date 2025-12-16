#pragma once

#include "glm/glm.hpp"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>

class GLShader {
public:
	GLShader(const std::string& name, const std::filesystem::path& vs, const std::filesystem::path& fs);
	~GLShader();

	void Bind() const;
	void Unbind() const;

	void SetInt(const std::string &name, unsigned int value);
	void SetIntArray(const std::string &name, const int* value, unsigned int count);
	void SetFloat(const std::string &name, float value);
	void SetFloat2(const std::string &name, const glm::vec2 &value);
	void SetFloat3(const std::string &name, const glm::vec3 &value);
	void SetFloat4(const std::string &name, const glm::vec4 &value);
	void SetMat3(const std::string &name, const glm::mat3 &value);
	void SetMat4(const std::string &name, const glm::mat4 &value);

	const std::string &GetName() const { return mName; }

	static std::shared_ptr<GLShader> Create(const std::string& name, const std::filesystem::path& vs, const std::filesystem::path& fs);

private:
	void CompileFromSources(const std::unordered_map<unsigned int, std::string>& shaderSources);
	unsigned int CompileShaderFromSource(unsigned int type, const std::string &src);

	void CompileProgram(const std::unordered_map<unsigned int, std::filesystem::path> &shaderFiles);
	unsigned int CompileShader(unsigned int type, const std::filesystem::path& shaderFile);

private:
	std::string mName;
	unsigned int mId = 0;
};
