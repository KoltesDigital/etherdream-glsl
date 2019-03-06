#pragma once

#include <memory>
#include <cli.hpp>
#include <windows.h>
#include <j4cDAC.h>

#include "../common/output.hpp"

class EtherDreamOutput : public Output
{
public:
	EtherDreamOutput(cli::Parser &parser, int pointCount);
	~EtherDreamOutput();

	InitializationStatus initialize() override;
	void shutdown() override;

	bool needPoints() override;
	bool streamPoints(const Point *data) override;

private:
	std::unique_ptr<EAD_Pnt_s[]> points;
	int pointCount;

	int cardIndex;
	bool listDevices;

	bool open{ false };
};