/**
 * @file llrponlylightset.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LLRPONLYLIGHTSET_H_
#define LLRPONLYLIGHTSET_H_

#include <cstdint>
#include <assert.h>

#include "lightset.h"

class LLRPOnlyLightSet: public LightSet {
	LLRPOnlyLightSet(void);
	~LLRPOnlyLightSet(void);

	void Start(uint8_t nPort) {
		assert(0);
	}
	void Stop(uint8_t nPort) {
		assert(0);
	}

	void SetData(uint8_t nPort, const uint8_t *pData, uint32_t nLength) {
		assert(0);
	}

	void Print(void) {

	}

public: // RDM Optional
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) {
		return false;
	}
	uint16_t GetDmxStartAddress(void) {
		return DMX_ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint(void) {
		return 0;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) {
		return false;
	}
};

#endif /* LLRPONLYLIGHTSET_H_ */
