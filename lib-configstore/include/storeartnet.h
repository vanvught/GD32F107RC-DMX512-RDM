/**
 * @file storeartnet.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STOREARTNET_H_
#define STOREARTNET_H_

#include "artnetparams.h"
#include "artnetstore.h"
#include "configstore.h"

#include "debug.h"

class StoreArtNet final: public ArtNetParamsStore, public ArtNetStore {
public:
	StoreArtNet(uint32_t nPortIndexOffset);

	void Update(const struct artnetparams::Params* pArtNetParams) override {
		DEBUG_ENTRY

		ConfigStore::Get()->Update(configstore::Store::NODE, pArtNetParams, sizeof(struct artnetparams::Params));

		DEBUG_EXIT
	}

	void Copy(struct artnetparams::Params* pArtNetParams) override {
		DEBUG_ENTRY

		ConfigStore::Get()->Copy(configstore::Store::NODE, pArtNetParams, sizeof(struct artnetparams::Params));

		DEBUG_EXIT
	}

	void SaveLongName(const char* pLongName) override {
		DEBUG_ENTRY

		ConfigStore::Get()->Update(configstore::Store::NODE, __builtin_offsetof(struct artnetparams::Params, aLongName), pLongName, artnet::LONG_NAME_LENGTH, artnetparams::Mask::LONG_NAME);

		DEBUG_EXIT
	}

	void SaveFailSafe(const uint8_t nFailSafe) override {
		DEBUG_ENTRY

		ConfigStore::Get()->Update(configstore::Store::NODE, __builtin_offsetof(struct artnetparams::Params, nFailSafe), &nFailSafe, sizeof(uint8_t), artnetparams::Mask::FAILSAFE);

		DEBUG_EXIT
	}

	void SaveUniverseSwitch(const uint32_t nPortIndex, __attribute__((unused)) const uint8_t nAddress) override {
		DEBUG_ENTRY

		SaveUniverse(nPortIndex);

		DEBUG_EXIT
	}

	void SaveNetSwitch(const uint32_t nPortIndex, __attribute__((unused)) const uint8_t nAddress) override {
		DEBUG_ENTRY

		SaveUniverse(nPortIndex);

		DEBUG_EXIT
	}

	void SaveSubnetSwitch(const uint32_t nPortIndex, __attribute__((unused)) const uint8_t nAddress) override {
		DEBUG_ENTRY

		SaveUniverse(nPortIndex);

		DEBUG_EXIT
	}

	void SaveShortName(uint32_t nPortIndex, const char *pShortName) override;
	void SaveMergeMode(uint32_t nPortIndex, const lightset::MergeMode tMerge) override;
	void SavePortProtocol(uint32_t nPortIndex, const artnet::PortProtocol tPortProtocol) override;
	void SaveOutputStyle(uint32_t nPortIndex, const lightset::OutputStyle outputStyle) override;
	void SaveRdmEnabled(uint32_t nPortIndex, bool isEnabled) override;

	static StoreArtNet *Get() {
		return s_pThis;
	}

private:
	void SaveUniverse(uint32_t nPortIndex);

private:
	static uint32_t s_nPortIndexOffset;
	static StoreArtNet *s_pThis;
};

#endif /* STOREARTNET_H_ */
