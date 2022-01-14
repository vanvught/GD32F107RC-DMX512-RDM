/**
 * @file config.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GD32_DMX_INTERNAL_H_
#define GD32_DMX_INTERNAL_H_

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "gd32/dmx_config.h"

static uint32_t _port_to_uart(uint32_t nPort) {
	switch (nPort) {
#if defined (DMX_USE_USART0)
	case dmx::config::USART0_PORT:
		return USART0;
		break;
#endif
#if defined (DMX_USE_USART1)
	case dmx::config::USART1_PORT:
		return USART1;
		break;
#endif
#if defined (DMX_USE_USART2)
	case dmx::config::USART2_PORT:
		return USART2;
		break;
#endif
#if defined (DMX_USE_UART3)
	case dmx::config::UART3_PORT:
		return UART3;
		break;
#endif
#if defined (DMX_USE_UART4)
	case dmx::config::UART4_PORT:
		return UART4;
		break;
#endif
#if defined (DMX_USE_USART5)
	case dmx::config::USART5_PORT:
		return USART5;
		break;
#endif
#if defined (DMX_USE_UART6)
	case dmx::config::UART6_PORT:
		return UART6;
		break;
#endif
#if defined (DMX_USE_UART7)
	case dmx::config::UART7_PORT:
		return UART7;
		break;
#endif
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return 0;
}

#endif /* GD32_DMX_INTERNAL_H_ */
