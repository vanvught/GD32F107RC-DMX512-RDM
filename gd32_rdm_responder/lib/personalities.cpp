/*
 * personalities.h
 *
 *  Created on: 20 Nov 2023
 *      Author: marcusbirkin
 */

#include "personalities.h"

#include <cassert>

uint8_t Personalities::toPersonalityIdx(Personality personality)
{
	return static_cast<uint8_t>(personality) + 1;
}

Personalities::Personality Personalities::fromPersonalityIdx(uint8_t nPersonality)
{
	assert(nPersonality >= 1);
	return static_cast<Personalities::Personality>(nPersonality - 1);
}

pixel::Type Personalities::toPixelType(Personalities::Personality personality)
{
	assert(static_cast<uint8_t>(personality) < static_cast<uint8_t>(pixel::Type::UNDEFINED));
	return static_cast<pixel::Type>(personality);
}

Personalities::Personality Personalities::fromPixelType(pixel::Type type)
{
	assert(static_cast<uint8_t>(type) < PERSONALITY_COUNT);
	return static_cast<Personalities::Personality>(type);
}
