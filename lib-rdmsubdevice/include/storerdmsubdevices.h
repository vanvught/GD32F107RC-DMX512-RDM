/**
 * @file storerdmsubdevices.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STORERDMSUBDEVICES_H_
#define STORERDMSUBDEVICES_H_

#include "rdmsubdevicesparams.h"

#include "configstore.h"

class StoreRDMSubDevices {
public:
	static StoreRDMSubDevices& Get() {
		static StoreRDMSubDevices instance;
		return instance;
	}

	static void Update(const rdm::subdevicesparams::Params *pParams) {
		Get().IUpdate(pParams);
	}

	static void Copy(rdm::subdevicesparams::Params *pParams) {
		Get().ICopy(pParams);
	}

private:
	void IUpdate(const rdm::subdevicesparams::Params *pParams)  {
		ConfigStore::Get()->Update(configstore::Store::RDMSUBDEVICES, pParams, sizeof(struct rdm::subdevicesparams::Params));
	}

	void ICopy(rdm::subdevicesparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::RDMSUBDEVICES, pParams, sizeof(struct rdm::subdevicesparams::Params));
	}
};

#endif /* STORERDMSUBDEVICES_H_ */
