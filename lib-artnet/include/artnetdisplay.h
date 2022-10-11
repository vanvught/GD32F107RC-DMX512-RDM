/**
 * @file artnetdisplay.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETDISPLAY_H_
#define ARTNETDISPLAY_H_

#include <cstdint>

#include "artnet.h"
#include "lightset.h"

class ArtNetDisplay {
public:
	virtual ~ArtNetDisplay() {}

	virtual void ShowShortName(const char *pShortName)= 0;
	virtual void ShowLongName(const char *pLongName)= 0;
	virtual void ShowUniverseSwitch(uint32_t nPortIndex, uint8_t nAddress)=0;
	virtual void ShowNetSwitch(uint8_t nAddress)=0;
	virtual void ShowSubnetSwitch(uint8_t nAddress)=0;
	virtual void ShowMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode)=0;
	virtual void ShowPortProtocol(uint32_t nPortIndex, artnet::PortProtocol portProtocol)=0;
	virtual void ShowRdmEnabled(uint32_t nPortIndex, bool isEnabled)=0;
	virtual void ShowFailSafe(uint8_t nFailsafe)=0;
};

#endif /* ARTNETDISPLAY_H_ */
