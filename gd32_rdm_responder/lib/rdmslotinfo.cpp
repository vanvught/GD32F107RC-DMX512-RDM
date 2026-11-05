/**
 * @file rdmslotinfo.cpp
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <string_view>

#include "rdm_slotinfo.h"
#include "pixeldmxparamsrdm.h"

using pixeldmx::paramsdmx::SlotsInfo;

namespace rdm::slotinfo {
std::string_view GetCategoryTextUndefined(uint16_t slot_offset) {
    switch (static_cast<pixeldmx::paramsdmx::SlotsInfo>(slot_offset)) {
        case SlotsInfo::TYPE:
            return "Type";
            break;
        case SlotsInfo::COUNT:
            return "Count";
            break;
        case SlotsInfo::GROUPING_COUNT:
            return "Grouping Count";
            break;
        case SlotsInfo::MAP:
            return "Map";
            break;
        case SlotsInfo::TEST_PATTERN:
            return "Test Pattern";
            break;
        case SlotsInfo::PROGRAM:
            return "Program";
            break;
        default:
            break;
    }

    return common::kUndefined;
}
} // namespace rdm::slotinfo
