#include "EtherDreamOutput.hpp"

const int EtherDreamOutput::NameBufferSize = 256;

EtherDreamOutput::EtherDreamOutput(const CommonParameters &commonParameters, cli::Parser &parser)
	: Output{ commonParameters }
{
	points.reset(new EAD_Pnt_s[commonParameters.pointCount]);

	cardIndex = parser.option("card-index")
		.alias("i")
		.description("Card index in the device list.")
		.defaultValue(0)
		.getValueAs<int>();

	cardName = parser.option("card-name")
		.alias("n")
		.description("Card name in the device list (overrides card-index).")
		.getValue();

	listDevices = parser.flag("list-devices")
		.alias("l")
		.description("Lists devices.")
		.getValue();

	offsetX = parser.option("offset-x")
		.alias("ox")
		.description("Offsets X coordinates.")
		.defaultValue("0")
		.getValueAs<float>();

	offsetY = parser.option("offset-y")
		.alias("oy")
		.description("Offsets Y coordinates.")
		.defaultValue("0")
		.getValueAs<float>();

	scale = parser.option("scale")
		.alias("sc")
		.description("Scales coordinates.")
		.defaultValue("1")
		.getValueAs<float>();
}

EtherDreamOutput::~EtherDreamOutput()
{
	EtherDreamClose();
}

InitializationStatus EtherDreamOutput::initialize()
{
	auto cardCount = EtherDreamGetCardNum();

	if (listDevices)
	{
		std::cout << "Devices:" << std::endl;
		for (int index = 0; index < cardCount; ++index)
		{
			char nameBuffer[NameBufferSize];
			EtherDreamGetDeviceName(&index, nameBuffer, sizeof(nameBuffer));
			std::cout << index << ": " << nameBuffer << std::endl;
		}

		return InitializationStatus::RequestExit;
	}

	if (cardName != nullptr)
	{
		cardIndex = -1;

		for (int index = 0; index < cardCount; ++index)
		{
			char nameBuffer[NameBufferSize];
			EtherDreamGetDeviceName(&index, nameBuffer, sizeof(nameBuffer));
			if (!std::strncmp(cardName, nameBuffer, NameBufferSize))
			{
				cardIndex = index;
				break;
			}
		}

		if (cardIndex < 0)
		{
			std::cerr << "Card name was not found." << std::endl;
			return InitializationStatus::Failure;
		}
	}
	else if (cardIndex < 0 || cardIndex >= cardCount)
	{
		std::cerr << "Card index is out of bounds." << std::endl;
		return InitializationStatus::Failure;
	}

	auto result = EtherDreamOpenDevice(&cardIndex);
	if (!result)
	{
		std::cerr << "Cannot connect." << std::endl;
		return InitializationStatus::Failure;
	}

	std::cout << "Connected." << std::endl;
	open = true;
	return InitializationStatus::Success;
}

void EtherDreamOutput::shutdown()
{
	if (open)
	{
		EtherDreamCloseDevice(&cardIndex);
		open = false;
	}
}

bool EtherDreamOutput::needPoints()
{
	return open && (EtherDreamGetStatus(&cardIndex) == GET_STATUS_READY);
}

static float clamp(float t, float min, float max)
{
	if (t < min)
	{
		t = min;
	}

	if (t > max)
	{
		t = max;
	}

	return t;
}

bool EtherDreamOutput::streamPoints(const Point *data)
{
	for (int i = 0; i < commonParameters.pointCount; ++i)
	{
		auto &fromPoint = data[i];
		auto &toPoint = points[i];

		toPoint.X = (int16_t)clamp((fromPoint.x + offsetX) * scale * 32767.f, -32768.f, 32767.f);
		toPoint.Y = (int16_t)clamp((fromPoint.y + offsetY) * scale * 32767.f, -32768.f, 32767.f);
		toPoint.R = (uint16_t)clamp(fromPoint.r * 65535.f, 0.f, 65535.f);
		toPoint.G = (uint16_t)clamp(fromPoint.g * 65535.f, 0.f, 65535.f);
		toPoint.B = (uint16_t)clamp(fromPoint.b * 65535.f, 0.f, 65535.f);
	}

	return EtherDreamWriteFrame(&cardIndex, points.get(), sizeof(EAD_Pnt_s) * commonParameters.pointCount, commonParameters.pointsPerSecond, 1);
}
