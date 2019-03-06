#include <cli.hpp>
#include <cmath>
#include <GL/glew.h>
#include <GL/GL.h>
#include <iostream>

#include "context.hpp"
#include "opengl.hpp"
#include "system.hpp"

#include "../windows/etherdream.hpp"

enum ExitCode
{
	Success,
	ContextCreationFailed,
	ExtensionsInitializationFailed,
	FramebufferIncomplete,
};

int main(int argc, char **argv)
{
	auto pointCount = 1800;

	cli::Parser parser{ argc, argv };

	EtherDreamOutput output{ parser, pointCount };

	bool help = parser.defaultHelpFlag()
		.getValue();

	if (help)
	{
		parser.showHelp();
		return ExitCode::Success;
	}

	if (parser.hasErrors())
	{
		return EXIT_FAILURE;
	}

	auto status = output.initialize();
	if (status != InitializationStatus::Success)
	{
		return 1;
	}

	if (!createContext())
	{
		std::cerr << "Context creation failed." << std::endl;
		return ExitCode::ContextCreationFailed;
	}

	std::cout << "OpenGL version: " << (char *)glGetString(GL_VERSION) << "." << std::endl;
	std::cout << "GLSL version: " << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << std::endl;

	auto err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << glewGetErrorString(err) << std::endl;
		return ExitCode::ExtensionsInitializationFailed;
	}

	{
		PointTexture pointTextureXY{ 2, GL_RG32F, pointCount };
		PointTexture pointTextureRGB{ 3, GL_RGB32F, pointCount };

		Framebuffer framebuffer{
			pointTextureXY,
			pointTextureRGB,
		};

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Framebuffer is incomplete." << std::endl;
			return ExitCode::FramebufferIncomplete;
		}

		std::string vertexSource = "#version 330\n\
		layout(location = 0) in vec2 aPosition;\n\
		layout(location = 1) in float aOffset;\n\
		out float index;\n\
		uniform float base;\n\
		void main() {\n\
			gl_Position = vec4(aPosition, 0, 1);\n\
			index = base + aOffset;\n\
		}";

		std::string fragmentSource = "#version 330\n\
		\n\
		smooth in float index;\n\
		\n\
		layout(location = 0) out vec2 position;\n\
		layout(location = 1) out vec3 color;\n\
		\n\
		void main()\n\
		{\n\
			float angle = index * .05;\n\
			float radius = abs(sin(angle * .4)) * .5;\n\
			position = vec2(cos(angle*1.1), sin(angle)) * radius;\n\
			color = vec3(1);\n\
		}";

		Shader vertexShader{ GL_VERTEX_SHADER };
		vertexShader.compile(vertexSource);

		Shader fragmentShader{ GL_FRAGMENT_SHADER };
		fragmentShader.compile(fragmentSource);

		Program program{ vertexShader, fragmentShader };
		if (!program.link())
		{
			return 1;
		}

		Quad quad{ pointCount };

		glEnable(GL_CULL_FACE);
		glViewport(0, 0, pointCount, 1);

		auto points = std::make_unique<Point[]>(pointCount);

		int base = 0;

		for (;;)
		{
			glUniform1f(program.getUniformLocation(Program::Uniform::Base), (float)base);
			base += pointCount;

			while (!output.needPoints())
			{
				takeANap();
			}

			quad.render();

			auto pointsXY = pointTextureXY.readPixels(GL_RG);
			auto pointsRGB = pointTextureRGB.readPixels(GL_RGB);

			err = glGetError();
			if (err != GL_NO_ERROR)
			{
				break;
			}

			for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
			{
				auto &point = points[pointIndex];
				point.x = pointsXY[pointIndex * 2 + 0];
				point.y = pointsXY[pointIndex * 2 + 1];
				point.r = pointsRGB[pointIndex * 3 + 0];
				point.g = pointsRGB[pointIndex * 3 + 1];
				point.b = pointsRGB[pointIndex * 3 + 2];
				//std::cout << point << std::endl;
			}

			if (!output.streamPoints(points.get()))
			{
				break;
			}
		}
	}

	destroyContext();

	return 0;
}
