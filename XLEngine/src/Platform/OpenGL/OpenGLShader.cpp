#include "xlpch.h"

#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace XLEngine
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		XL_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		XL_PROFILE_FUNCTION();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		// Extract name from filepath, example: file/text.glsl -> text
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		:m_Name(name)
	{
		XL_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		XL_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in|std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			XL_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		XL_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		// 定义着色器类型标识符
		const char* typeToken = "#type";

		// 计算着色器类型标识符的长度
		size_t typeTokenLength = strlen(typeToken);

		// 在源代码中查找第一个着色器类型标识符的位置
		size_t pos = source.find(typeToken, 0);

		// 循环直到没有找到更多的标识符
		while (pos != std::string::npos)
		{
			// 查找标识符后的行结束符（换行符或回车符）
			size_t eol = source.find_first_of("\r\n", pos);
			XL_CORE_ASSERT(eol != std::string::npos, "Syntax error");

			// 计算标识符值的起始位置
			size_t begin = pos + typeTokenLength + 1;

			// 提取标识符值（着色器类型，如"vertex"或"fragment"）
			std::string type = source.substr(begin, eol - begin);
			XL_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			// 查找下一个非空白字符的位置，以确定着色器源代码的起始位置
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);

			// 查找下一个着色器类型标识符的位置
			pos = source.find(typeToken, nextLinePos);

			// 将提取的着色器源代码存储到映射中，以着色器类型作为键
			// 如果没有找到下一个标识符，则将剩余的源代码都包括在内
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		// 返回包含不同类型着色器源代码的映射
		return shaderSources;
	}


	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		XL_PROFILE_FUNCTION();

		// 创建一个OpenGL着色器程序
		GLuint program = glCreateProgram();

		XL_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
		// 用于存储着色器对象的容器
		std::array<GLenum, 2>glShaderIDs;
		int glShaderIDIndex = 0;

		// 遍历着色器源代码的unordered_map
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;                 // 着色器类型 (如GL_VERTEX_SHADER或GL_FRAGMENT_SHADER)
			const std::string& source = kv.second;  // 着色器源代码

			// 创建一个新的着色器对象
			GLuint shader = glCreateShader(type);

			// 将着色器源代码与着色器对象关联
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			// 编译着色器
			glCompileShader(shader);

			// 检查着色器是否编译成功
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				// 获取编译错误信息的长度
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// 创建一个存储错误信息的缓冲区
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// 删除失败的着色器对象
				glDeleteShader(shader);

				// 输出错误信息并断言失败
				XL_CORE_ERROR("{0}", infoLog.data());
				XL_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}

			// 将着色器附加到着色器程序上
			glAttachShader(program, shader);

			// 存储着色器对象的ID
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		// 将编译好的着色器程序的ID存储在类成员变量中
		m_RendererID = program;

		// 链接着色器程序
		glLinkProgram(program);

		// 检查着色器程序链接是否成功
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			// 获取链接错误信息的长度
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// 创建一个存储错误信息的缓冲区
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// 删除着色器程序和着色器对象
			glDeleteProgram(program);
			for (auto id : glShaderIDs)
				glDeleteShader(id);

			// 输出错误信息并断言失败
			XL_CORE_ERROR("{0}", infoLog.data());
			XL_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// 分离并删除着色器对象，因为它们已经被链接到程序中
		for (auto id : glShaderIDs)
			glDetachShader(program, id);
	}

	OpenGLShader::~OpenGLShader()
	{
		XL_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		XL_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		XL_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		XL_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		XL_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		XL_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		XL_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		XL_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());

		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}