#include "ConsoleOutput.hpp"

#include <algorithm>

#include "system.hpp"

ConsoleOutput::ConsoleOutput(const CommonParameters &commonParameters, cli::Parser &parser)
	: Output{ commonParameters }
{
	limitPoints = parser.option("limit-points")
		.alias("l")
		.description("If greater than 0, limits the number of dumped points.")
		.defaultValue("0")
		.getValueAs<int>();

	pauseDuration = parser.option("pause-duration")
		.alias("d")
		.description("Pause between renderings, in seconds.")
		.defaultValue("0")
		.getValueAs<float>();
}

InitializationStatus ConsoleOutput::initialize()
{
	return InitializationStatus::Success;
}

bool ConsoleOutput::needPoints()
{
	return true;
}

bool ConsoleOutput::streamPoints(const Point *data)
{
	auto count = limitPoints > 0 ? std::min(limitPoints, commonParameters.pointCount) : commonParameters.pointCount;

	for (int i = 0; i < count; ++i)
	{
		std::cout << data[i] << std::endl;
	}

	pause(pauseDuration);

	return true;
}
