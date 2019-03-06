#pragma once

#include <memory>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/GL.h>

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
	GLint getUniformLocation(Uniform uniform) const;

private:
	GLuint vertexShaderName{ 0 };
	GLuint fragmentShaderName{ 0 };

	std::vector<GLuint> uniformLocations;
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