/**
 * @file artnetrdmresponder.cpp
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>

#include "artnetrdmresponder.h"

#include "rdmdeviceresponder.h"
#include "rdmhandler.h"
#include "rdmmessage.h"

#include "lightset.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "debug.h"

namespace configstore {
void delay();
}  // namespace configstore

ArtNetRdmResponder *ArtNetRdmResponder::s_pThis;
TRdmMessage ArtNetRdmResponder::s_RdmCommand;

ArtNetRdmResponder::ArtNetRdmResponder(RDMPersonality **pRDMPersonalities, uint32_t nPersonalityCount) :
	RDMDeviceResponder(pRDMPersonalities, nPersonalityCount)
{
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_EXIT
}

ArtNetRdmResponder::~ArtNetRdmResponder() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

const uint8_t *ArtNetRdmResponder::Handler(uint32_t nPortIndex, const uint8_t *pRdmDataNoSC) {
	DEBUG_ENTRY

	if (nPortIndex != 0) {
		DEBUG_EXIT
		return nullptr;
	}

	if (pRdmDataNoSC == nullptr) {
		DEBUG_EXIT
		return nullptr;
	}

#ifndef NDEBUG
	RDMMessage::PrintNoSc(pRdmDataNoSC);
#endif

	HandleData(pRdmDataNoSC, reinterpret_cast<uint8_t*>(&s_RdmCommand));

	if (s_RdmCommand.start_code != E120_SC_RDM) {
		DEBUG_EXIT
		return nullptr;
	}

#ifndef NDEBUG
	RDMMessage::Print(reinterpret_cast<uint8_t*>(&s_RdmCommand));
#endif

	DEBUG_EXIT
	return reinterpret_cast<const uint8_t*>(&s_RdmCommand);
}
