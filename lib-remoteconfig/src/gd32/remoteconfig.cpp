/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"

#include "gd32.h"

#include "debug.h"

#if !defined (GD32F4XX)
#else
# define BKP_DATA_1	0
static void bkp_data_write(__attribute__((unused)) int x, uint16_t nData) {
	auto *pDst = reinterpret_cast<uint16_t *>(BKPSRAM_BASE + 16);
	DEBUG_PRINTF("*pDst=%u", *pDst);
	*pDst =  nData;
}
static uint16_t bkp_data_read(__attribute__((unused)) int x) {
	const auto *pSrc = reinterpret_cast<uint16_t *>(BKPSRAM_BASE + 16);
	DEBUG_PRINTF("*pSrc=%u", *pSrc);
	return *pSrc;
}
#endif

void RemoteConfig::PlatformHandleTftpSet() {
	DEBUG_ENTRY

	if (m_bEnableTFTP) {
		bkp_data_write(BKP_DATA_1, 0xA5A5);
	} else {
		bkp_data_write(BKP_DATA_1, 0x0);
	}

	DEBUG_EXIT
}

void RemoteConfig::PlatformHandleTftpGet() {
	DEBUG_ENTRY

	m_bEnableTFTP = (bkp_data_read(BKP_DATA_1) == 0xA5A5);

	DEBUG_PRINTF("m_bEnableTFTP=%d", m_bEnableTFTP);
	DEBUG_EXIT
}
