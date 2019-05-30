#pragma once

#include <memory>
#include <cli.hpp>
#include <windows.h>
#include <j4cDAC.h>

#include "../common/Output.hpp"

class EtherDreamOutput : public Output
{
public:
	static const int NameBufferSize;

	EtherDreamOutput(const CommonParameters &commonParameters, cli::Parser &parser);
	~EtherDreamOutput();

	InitializationStatus initialize() override;
	void shutdown() override;

	bool needPoints() override;
	bool streamPoints(const Point *data) override;

private:
	std::unique_ptr<EAD_Pnt_s[]> points;

	int cardIndex;
	const char *cardName;
	bool listDevices;
	float offsetX;
	float offsetY;
	float scale;

	bool open{ false };
};
