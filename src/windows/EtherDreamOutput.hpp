#pragma once

#include <memory>
#include <cli.hpp>
#include <windows.h>
#include <j4cDAC.h>

#include "../common/Output.hpp"

class EtherDreamOutput : public Output
{
public:
	EtherDreamOutput(const CommonParameters &commonParameters, cli::Parser &parser);
	~EtherDreamOutput();

	InitializationStatus initialize() override;
	void shutdown() override;

	bool needPoints() override;
	bool streamPoints(const Point *data) override;

private:
	std::unique_ptr<EAD_Pnt_s[]> points;

	int cardIndex;
	bool listDevices;

	bool open{ false };
};