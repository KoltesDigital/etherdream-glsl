#include "opengl.hpp"

#include <iostream>
#include <iterator>

GLuint ObjectWithName::getName() const
{
	return name;
}

Shader::Shader(GLenum shaderType)
{
	name = glCreateShader(shaderType);
}

Shader::~Shader()
{
	glDeleteShader(name);
}

bool Shader::compile(const std::string &source)
{
	auto cSource = (const GLchar *)source.c_str();
	glShaderSource(name, 1, &cSource, 0);

	glCompileShader(name);

	GLint isCompiled;
	glGetShaderiv(name, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(name, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(name, maxLength, &maxLength, &infoLog[0]);

		std::copy(infoLog.begin(), infoLog.end(), std::ostream_iterator<char>(std::cerr));

		return false;
	}

	return true;
}

Program::Program(const Shader &vertexShader, const Shader &fragmentShader)
{
	uniformLocations.resize(Uniform::_Count);

	name = glCreateProgram();

	vertexShaderName = vertexShader.getName();
	glAttachShader(name, vertexShaderName);

	fragmentShaderName = fragmentShader.getName();
	glAttachShader(name, fragmentShaderName);
}

Program::~Program()
{
	glDetachShader(name, vertexShaderName);
	glDetachShader(name, fragmentShaderName);

	glDeleteProgram(name);
}

bool Program::link()
{
	glLinkProgram(name);

	GLint isLinked = 0;
	glGetProgramiv(name, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(name, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(name, maxLength, &maxLength, &infoLog[0]);

		std::copy(infoLog.begin(), infoLog.end(), std::ostream_iterator<char>(std::cerr));

		return false;
	}

	glUseProgram(name);

	uniformLocations[Uniform::Base] = glGetUniformLocation(name, "base");

	return true;
}

GLint Program::getUniformLocation(Uniform uniform) const
{
	return uniformLocations[uniform];
}

PointTexture::PointTexture(int components, GLint internalFormat, int pointCount)
{
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_1D, name);

	glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, pointCount, 0, GL_RGB, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	pixels = std::make_unique<float[]>(components * pointCount);
}

PointTexture::~PointTexture()
{
	glDeleteTextures(1, &name);
}

float *PointTexture::readPixels(GLenum format)
{
	glBindTexture(GL_TEXTURE_1D, name);
	glGetTexImage(GL_TEXTURE_1D, 0, format, GL_FLOAT, pixels.get());
	return pixels.get();
}

Framebuffer::Framebuffer(std::initializer_list<std::reference_wrapper<PointTexture>> list)
	: Framebuffer((int)list.size())
{
	int index = 0;
	for (auto texture : list)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture.get().getName(), 0);
		++index;
	}


	std::vector<GLenum> drawBuffers(list.size());
	for (int i = 0; i < list.size(); ++i)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	};

	glDrawBuffers((int)list.size(), drawBuffers.data());
}

Framebuffer::Framebuffer(int textureCount)
{
	glGenFramebuffers(1, &name);
	glBindFramebuffer(GL_FRAMEBUFFER, name);

	std::vector<GLenum> drawBuffers(textureCount);
	for (int i = 0; i < textureCount; ++i)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	};

	glDrawBuffers(textureCount, drawBuffers.data());
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &name);
}

void Framebuffer::setTexture(int index, const PointTexture &texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, name);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture.getName(), 0);
}

Quad::Quad(int pointCount)
{
	const GLfloat positions[4][2] = {
		{ -1.f,  1.f },
		{ -1.f, -1.f },
		{  1.f,  1.f },
		{  1.f, -1.f },
	};

	const GLfloat offsets[4] = {
		-.5f,
		-.5f,
		pointCount - .5f,
		pointCount - .5f,
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(sizeof(vbo) / sizeof(vbo[0]), vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(AttributeLayout::Position, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);
	glVertexAttribPointer(AttributeLayout::Offset, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

Quad::~Quad()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDeleteBuffers(sizeof(vbo) / sizeof(vbo[0]), vbo);
	glDeleteVertexArrays(1, &vao);
}

void Quad::render() const
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
