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

		// ������ɫ�����ͱ�ʶ��
		const char* typeToken = "#type";

		// ������ɫ�����ͱ�ʶ���ĳ���
		size_t typeTokenLength = strlen(typeToken);

		// ��Դ�����в��ҵ�һ����ɫ�����ͱ�ʶ����λ��
		size_t pos = source.find(typeToken, 0);

		// ѭ��ֱ��û���ҵ�����ı�ʶ��
		while (pos != std::string::npos)
		{
			// ���ұ�ʶ������н����������з���س�����
			size_t eol = source.find_first_of("\r\n", pos);
			XL_CORE_ASSERT(eol != std::string::npos, "Syntax error");

			// �����ʶ��ֵ����ʼλ��
			size_t begin = pos + typeTokenLength + 1;

			// ��ȡ��ʶ��ֵ����ɫ�����ͣ���"vertex"��"fragment"��
			std::string type = source.substr(begin, eol - begin);
			XL_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			// ������һ���ǿհ��ַ���λ�ã���ȷ����ɫ��Դ�������ʼλ��
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);

			// ������һ����ɫ�����ͱ�ʶ����λ��
			pos = source.find(typeToken, nextLinePos);

			// ����ȡ����ɫ��Դ����洢��ӳ���У�����ɫ��������Ϊ��
			// ���û���ҵ���һ����ʶ������ʣ���Դ���붼��������
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		// ���ذ�����ͬ������ɫ��Դ�����ӳ��
		return shaderSources;
	}


	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		XL_PROFILE_FUNCTION();

		// ����һ��OpenGL��ɫ������
		GLuint program = glCreateProgram();

		XL_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
		// ���ڴ洢��ɫ�����������
		std::array<GLenum, 2>glShaderIDs;
		int glShaderIDIndex = 0;

		// ������ɫ��Դ�����unordered_map
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;                 // ��ɫ������ (��GL_VERTEX_SHADER��GL_FRAGMENT_SHADER)
			const std::string& source = kv.second;  // ��ɫ��Դ����

			// ����һ���µ���ɫ������
			GLuint shader = glCreateShader(type);

			// ����ɫ��Դ��������ɫ���������
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			// ������ɫ��
			glCompileShader(shader);

			// �����ɫ���Ƿ����ɹ�
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				// ��ȡ���������Ϣ�ĳ���
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// ����һ���洢������Ϣ�Ļ�����
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// ɾ��ʧ�ܵ���ɫ������
				glDeleteShader(shader);

				// ���������Ϣ������ʧ��
				XL_CORE_ERROR("{0}", infoLog.data());
				XL_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}

			// ����ɫ�����ӵ���ɫ��������
			glAttachShader(program, shader);

			// �洢��ɫ�������ID
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		// ������õ���ɫ�������ID�洢�����Ա������
		m_RendererID = program;

		// ������ɫ������
		glLinkProgram(program);

		// �����ɫ�����������Ƿ�ɹ�
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			// ��ȡ���Ӵ�����Ϣ�ĳ���
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// ����һ���洢������Ϣ�Ļ�����
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// ɾ����ɫ���������ɫ������
			glDeleteProgram(program);
			for (auto id : glShaderIDs)
				glDeleteShader(id);

			// ���������Ϣ������ʧ��
			XL_CORE_ERROR("{0}", infoLog.data());
			XL_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// ���벢ɾ����ɫ��������Ϊ�����Ѿ������ӵ�������
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