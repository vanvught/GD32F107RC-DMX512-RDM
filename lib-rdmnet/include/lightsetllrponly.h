/**
 * @file lightsetllrponly.h
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

#ifndef LIGHTSETLLRPONLY_H_
#define LIGHTSETLLRPONLY_H_

#include <cstdint>

#include "lightset.h"

class LightSetLLRPOnly final: public LightSet {
public:
	LightSetLLRPOnly() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void Start(__attribute__((unused)) uint32_t nPortIndex) override {
	}

	void Stop(__attribute__((unused)) uint32_t nPortIndex) override {
	}

	void SetData(__attribute__((unused)) uint32_t nPortIndex, __attribute__((unused)) const uint8_t *pData, __attribute__((unused)) uint32_t nLength) override {
	}

	void Print() override {
	}

	bool SetDmxStartAddress(__attribute__((unused)) uint16_t nDmxStartAddress) override {
		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return lightset::dmx::ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint() override {
		return 0;
	}

	bool GetSlotInfo(__attribute__((unused)) uint16_t nSlotOffset, __attribute__((unused)) lightset::SlotInfo &tSlotInfo) override {
		return false;
	}
};

#endif /* LIGHTSETLLRPONLY_H_ */
