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
	InvalidShaderCode,
};

static CommonParameters commonParameters;

static std::unique_ptr<Shader> vertexShader;
static std::unique_ptr<Shader> fragmentShader;
static std::unique_ptr<Program> program;

static std::atomic<bool> shaderChanged{ false };
static std::unique_ptr<Output> output;

bool compileProgram()
{
	std::ifstream shaderFile{ commonParameters.shaderPath, std::ios::in | std::ios::binary };
	if (!shaderFile)
	{
		std::cerr << "Unable to open shader." << std::endl;
		return false;
	}

	std::string shaderSource;

	shaderFile.seekg(0, std::ios::end);
	shaderSource.resize((std::size_t)shaderFile.tellg());
	shaderFile.seekg(0, std::ios::beg);
	shaderFile.read(&shaderSource[0], shaderSource.size());
	shaderFile.close();

	std::unique_ptr<Shader> newFragmentShader{ new Shader{ GL_FRAGMENT_SHADER } };
	std::unique_ptr<Program> newProgram{ new Program { *vertexShader, *newFragmentShader } };

	newFragmentShader->compile(shaderSource);

	if (!newProgram->link())
	{
		std::cerr << "Failed to link program." << std::endl;
		return false;
	}

	fragmentShader = std::move(newFragmentShader);
	program = std::move(newProgram);
	return true;
}

ExitCode run()
{
	PointTexture pointTextureXY{ 2, GL_RG32F, commonParameters.pointCount };
	PointTexture pointTextureRGB{ 3, GL_RGB32F, commonParameters.pointCount };

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

	vertexShader.reset(new Shader{ GL_VERTEX_SHADER });
	vertexShader->compile(vertexSource);

	if (!compileProgram())
	{
		return ExitCode::InvalidShaderCode;
	}

	Quad quad{ commonParameters.pointCount };

	glEnable(GL_CULL_FACE);
	glViewport(0, 0, commonParameters.pointCount, 1);

	auto points = std::unique_ptr<Point[]>(new Point[commonParameters.pointCount]);

	systemStartTime();

	for (;;)
	{
		if (shaderChanged)
		{
			if (commonParameters.verbose)
			{
				std::cout << "Shader changed, reloading." << std::endl;
			}

			shaderChanged = false;

			compileProgram();
		}

		if (program->isLinked())
		{
			program->incrementBase(commonParameters.pointCount);
			program->updateTime();

			quad.render();

			auto pointsXY = pointTextureXY.readPixels(GL_RG);
			auto pointsRGB = pointTextureRGB.readPixels(GL_RGB);

			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				break;
			}

			for (int pointIndex = 0; pointIndex < commonParameters.pointCount; ++pointIndex)
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

			while (!output->needPoints())
			{
				systemPause();
			}
		}
		else
		{
			systemPause();
		}
	}

	return ExitCode::Success;
}


int main(int argc, char **argv)
{
	cli::Parser parser{ argc, argv };

	commonParameters.pointCount = parser.option("points")
		.alias("p")
		.description("Resolution of a single rendering.")
		.defaultValue("1800")
		.getValueAs<int>();

	commonParameters.pointsPerSecond = parser.option("points-per-second")
		.alias("pps")
		.description("Laser speed.")
		.defaultValue("25000")
		.getValueAs<uint16_t>();

	commonParameters.shaderPath = parser.option("shader")
		.alias("s")
		.description("Shader file path.")
		.required()
		.getValueAs<std::string>();

	commonParameters.verbose = parser.flag("verbose")
		.alias("v")
		.description("Shows information messages.")
		.getValue();

	auto outputClass = parser.option("output")
		.alias("o")
		.description("Output implementation.")
#if defined(SYSTEM_WINDOWS)
		.defaultValue("etherdream")
#else
		.defaultValue("console")
#endif
		.getValueAs<std::string>();

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

	if (commonParameters.verbose)
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

	fileWatcher.watchFile(commonParameters.shaderPath, [&]()
	{
		shaderChanged = true;
	});

	fileWatcher.start();

	auto exitCode = run();

	contextDestroy();

	return exitCode;
}
