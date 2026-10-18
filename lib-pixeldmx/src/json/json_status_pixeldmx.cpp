/**
 * @file json_status_pixeldmx.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "common/utils/utils_string.h"
#include "dmxnode.h"
#include "json/json_helpers.h"

namespace json::status {
uint32_t PixelDmx(char* buffer, uint32_t length) {
    return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
        doc.BeginObject("PortName");

        static_assert(dmxnode::kMaxPorts <= 99);

        for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++) {
          char key[2];
          const auto* end = common::AppendUnsigned(key, i + 1U);
          doc[std::string_view{key, static_cast<size_t>(end - key)}] =
              DmxNode::Instance().GetPortName(i);        }

        doc.EndObject();
    });
}
} // namespace json::status