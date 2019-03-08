#pragma once

#include <cli.hpp>

#include "Output.hpp"

class ConsoleOutput : public Output
{
public:
	ConsoleOutput(const CommonParameters &commonParameters, cli::Parser &parser);

	InitializationStatus initialize() override;

	bool needPoints() override;
	bool streamPoints(const Point *data) override;

private:
	int limitPoints;
	float pauseDuration;
};
