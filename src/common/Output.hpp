#pragma once

#include <ostream>

struct CommonParameters
{
	int pointCount;
	uint16_t pointsPerSecond;
	std::string shaderPath;
	bool verbose;
};

enum class InitializationStatus
{
	Success,
	Failure,
	RequestExit,
};

struct Point
{
	float x, y; // [-1, 1]
	float r, g, b; // [0, 1]
};

std::ostream &operator<<(std::ostream &stream, const Point &point);

class Output
{
public:
	Output(const CommonParameters &commonParameters);
	virtual ~Output();

	virtual InitializationStatus initialize() = 0;
	virtual void shutdown();

	virtual bool needPoints() = 0;
	virtual bool streamPoints(const Point *data) = 0;

protected:
	const CommonParameters &commonParameters;
};
