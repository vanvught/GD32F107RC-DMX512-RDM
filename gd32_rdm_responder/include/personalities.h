/*
 * personalities.h
 *
 *  Created on: 20 Nov 2023
 *      Author: marcusbirkin
 */

#ifndef INCLUDE_PERSONALITIES_H_
#define INCLUDE_PERSONALITIES_H_

#include <cstdint>
#include <type_traits>

#include "pixeltype.h"

namespace Personalities
{

enum class Personality : uint8_t {
#if defined(CONFIG_RDM_ENABLE_CONFIG_PIDS)
	WS2801,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	WS2815,
	SK6812,
	SK6812W,
	UCS1903,
	UCS2903,
	CS8812,
	APA102,
	SK9822,
	P9813,
#else
	DEFAULT,
	CONFIG_MODE,
#endif
	// Last item
	COUNT
};
#define PERSONALITY_COUNT static_cast<std::underlying_type<Personalities::Personality>::type>(Personalities::Personality::COUNT)

uint8_t toPersonalityIdx(Personality personality);
Personality fromPersonalityIdx(uint8_t nPersonality);

pixel::Type toPixelType(Personality personality);
Personality fromPixelType(pixel::Type type);

} // namespace Personalities

#endif /* INCLUDE_PERSONALITIES_H_ */
