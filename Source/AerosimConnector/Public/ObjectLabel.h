#pragma once

#include <cstdint>

enum class ObjectLabel : uint8_t
{
	None = 0,
	Terrain,
	Building,
	Road,
	Water,
	Any = 0xFF
};
