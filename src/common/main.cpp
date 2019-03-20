#include <atomic>
#include <cli.hpp>
#include <cmath>
#include <fstream>
#include <iostream>

#include "ConsoleOutput.hpp"
#include "context.hpp"
#include "FileWatcher.hpp"
#include "opengl.hpp"
#include "system.hpp"

#if defined(SYSTEM_LINUX)
// ...
#elif defined(SYSTEM_MACOSX)
// ...
#elif defined(SYSTEM_WINDOWS)
#include "../windows/EtherDreamOutput.hpp"
#else
#error "Unrecognized system"
#endif

enum ExitCode
{
	Success,
	ParameterError,
	OutputCreationFailed,
	ContextCreationFailed,
	ExtensionsInitializationFailed,
	FramebufferIncomplete,
};

int main(int argc, char **argv)
{
	cli::Parser parser{ argc, argv };

	auto pointCount = parser.option("points")
		.alias("p")
		.description("Resolution of a single rendering.")
		.defaultValue("1800")
		.getValueAs<int>();

	auto shaderPath = parser.option("shader")
		.alias("s")
		.description("Shader file path.")
		.required()
		.getValueAs<std::string>();

	auto verbose = parser.flag("verbose")
		.alias("v")
		.description("Shows information messages.")
		.getValue();

	CommonParameters commonParameters
	{
		pointCount,
		shaderPath,
		verbose,
	};

	auto outputClass = parser.option("output")
		.alias("o")
		.description("Output implementation.")
#if defined(SYSTEM_WINDOWS)
		.defaultValue("etherdream")
#else
		.defaultValue("console")
#endif
		.getValueAs<std::string>();

	std::unique_ptr<Output> output;

	if (outputClass == "console")
	{
		output.reset(new ConsoleOutput(commonParameters, parser));
	}

#if defined(SYSTEM_WINDOWS)
	else if (outputClass == "etherdream")
	{
		output.reset(new EtherDreamOutput(commonParameters, parser));
	}
#endif

	if (!output)
	{
		std::cerr << "Unrecognized output." << std::endl;
		return ExitCode::ParameterError;
	}

	bool help = parser.defaultHelpFlag()
		.getValue();

	if (help)
	{
		parser.showHelp();
		return ExitCode::Success;
	}

	if (parser.hasErrors())
	{
		return ExitCode::ParameterError;
	}

	auto status = output->initialize();
	if (status != InitializationStatus::Success)
	{
		return ExitCode::OutputCreationFailed;
	}

	if (!contextCreate())
	{
		std::cerr << "Context creation failed." << std::endl;
		return ExitCode::ContextCreationFailed;
	}

	if (verbose)
	{
		std::cout << "OpenGL version: " << (char *)glGetString(GL_VERSION) << "." << std::endl;
		std::cout << "GLSL version: " << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << std::endl;
	}

	auto err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << glewGetErrorString(err) << std::endl;
		return ExitCode::ExtensionsInitializationFailed;
	}

	FileWatcher fileWatcher;

	std::atomic<bool> compileShader{ false };
	fileWatcher.watchFile(shaderPath, [&]()
	{
		compileShader = true;
	});

	fileWatcher.start();

	{
		PointTexture pointTextureXY{ 2, GL_RG32F, pointCount };
		PointTexture pointTextureRGB{ 3, GL_RGB32F, pointCount };

		Framebuffer framebuffer{
			pointTextureXY,
			pointTextureRGB,
		};

		if (!framebuffer.isComplete())
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

		Shader vertexShader{ GL_VERTEX_SHADER };
		vertexShader.compile(vertexSource);

		Shader fragmentShader{ GL_FRAGMENT_SHADER };

		Program program{ vertexShader, fragmentShader };

		Quad quad{ pointCount };

		glEnable(GL_CULL_FACE);
		glViewport(0, 0, pointCount, 1);

		auto points = std::unique_ptr<Point[]>(new Point[pointCount]);

		systemStartTime();

		for (;;)
		{
			if (compileShader)
			{
				if (verbose)
				{
					std::cout << "Shader changed, reloading." << std::endl;
				}

				compileShader = false;

				std::ifstream shaderFile{ shaderPath, std::ios::in | std::ios::binary };
				if (shaderFile)
				{
					std::string shaderSource;

					shaderFile.seekg(0, std::ios::end);
					shaderSource.resize((std::size_t)shaderFile.tellg());
					shaderFile.seekg(0, std::ios::beg);
					shaderFile.read(&shaderSource[0], shaderSource.size());
					shaderFile.close();

					fragmentShader.compile(shaderSource);

					program.link();
				}
				else
				{
					std::cerr << "Unable to open shader." << std::endl;
				}
			}

			while (!output->needPoints())
			{
				systemPause();
			}

			if (program.isLinked())
			{
				program.incrementBase(pointCount);
				program.updateTime();

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
				}

				if (!output->streamPoints(points.get()))
				{
					break;
				}
			}
			else
			{
				systemPause();
			}
		}
	}

	contextDestroy();

	return ExitCode::Success;
}
