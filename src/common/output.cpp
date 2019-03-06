#include "output.hpp"

std::ostream &operator<<(std::ostream &stream, const Point &point)
{
	return stream << "Point: x=" << point.x << ", y=" << point.y << ", r=" << point.r << ", g=" << point.g << ", b=" << point.b;
}
