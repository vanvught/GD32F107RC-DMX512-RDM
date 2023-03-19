/**
 * @file artnetparamsconst.h
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

#ifndef ARTNETPARAMSCONST_H_
#define ARTNETPARAMSCONST_H_

#include "artnet.h"

struct ArtNetParamsConst {
	static const char FILE_NAME[];

	static const char NET[];
	static const char SUBNET[];

	static const char ENABLE_RDM[];

	static const char NODE_SHORT_NAME[];
	static const char NODE_LONG_NAME[];

	static const char DESTINATION_IP_PORT[artnet::PORTS][24];

	static const char RDM_ENABLE_PORT[artnet::PORTS][18];

	/**
	 * Art-Net 4
	 */

	static const char PROTOCOL_PORT[artnet::PORTS][16];

	static const char MAP_UNIVERSE0[];
};

#endif /* ARTNETPARAMSCONST_H_ */
