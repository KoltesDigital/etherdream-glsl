#pragma once

#include <ostream>

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
	virtual ~Output() {}

	virtual InitializationStatus initialize() = 0;
	virtual void shutdown() {};

	virtual bool needPoints() = 0;
	virtual bool streamPoints(const Point *data) = 0;
};
