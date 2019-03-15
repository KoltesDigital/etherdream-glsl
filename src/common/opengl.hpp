#pragma once

#include <memory>
#include <string>
#include <vector>

#if defined(SYSTEM_MACOSX)
#include <OpenGL/glew.h>
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

class ObjectWithName
{
public:
	GLuint getName() const;

protected:
	GLuint name;
};

class Shader : public ObjectWithName
{
public:
	Shader(GLenum shaderType);
	~Shader();

	bool compile(const std::string &source);
};

class Program : public ObjectWithName
{
public:
	enum Uniform
	{
		Base,
		_Count,
	};

	Program(const Shader &vertexShader, const Shader &fragmentShader);
	~Program();

	bool link();
	bool isLinked() const;

	void incrementBase(int pointCount);

private:
	GLuint vertexShaderName{ 0 };
	GLuint fragmentShaderName{ 0 };

	GLint linked = 0;

	std::vector<GLuint> uniformLocations;
	int base = 0;
};

class PointTexture : public ObjectWithName
{
public:
	PointTexture(int components, GLint internalFormat, int pointCount);
	~PointTexture();

	float *readPixels(GLenum);

private:
	std::unique_ptr<float[]> pixels;
};

class Framebuffer : public ObjectWithName
{
public:
	Framebuffer(std::initializer_list<std::reference_wrapper<PointTexture>> list);
	Framebuffer(int textureCount);
	~Framebuffer();

	void setTexture(int index, const PointTexture &texture);

	bool isComplete() const;
};

class Quad
{
public:
	enum AttributeLayout
	{
		Position,
		Offset,
	};

	Quad(int pointCount);
	~Quad();

	void render() const;

private:
	GLuint vao, vbo[2];
};