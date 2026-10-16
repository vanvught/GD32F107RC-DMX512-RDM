/**
 * @file pixeldmxparamsconst.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_PIXELDMXPARAMSCONST_H_
#define JSON_PIXELDMXPARAMSCONST_H_

#include "json/json_key.h"
#include "common/utils/utils_hash.h"

namespace json {
struct PixelDmxParamsConst {
    static constexpr auto kDmxStartAddress = json::MakeKey("dmx_start_address");

    static constexpr auto kDmxSlotInfo = json::MakeKey("dmx_slot_info");
    static constexpr auto kStartUniPort1 = json::MakeKey("start_uni_port_1");
#if (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
    static constexpr auto kStartUniPort2 = json::MakeKey("start_uni_port_2");
    static constexpr auto kStartUniPort3 = json::MakeKey("start_uni_port_3");
    static constexpr auto kStartUniPort4 = json::MakeKey("start_uni_port_4");
    static constexpr auto kStartUniPort5 = json::MakeKey("start_uni_port_5");
    static constexpr auto kStartUniPort6 = json::MakeKey("start_uni_port_6");
    static constexpr auto kStartUniPort7 = json::MakeKey("start_uni_port_7");
    static constexpr auto kStartUniPort8 = json::MakeKey("start_uni_port_8");
#endif // (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
    static constexpr auto kStartUniPort9 = json::MakeKey("start_uni_port_9");
    static constexpr auto kStartUniPort10 = json::MakeKey("start_uni_port_10");
    static constexpr auto kStartUniPort11 = json::MakeKey("start_uni_port_11");
    static constexpr auto kStartUniPort12 = json::MakeKey("start_uni_port_12");
    static constexpr auto kStartUniPort13 = json::MakeKey("start_uni_port_13");
    static constexpr auto kStartUniPort14 = json::MakeKey("start_uni_port_14");
    static constexpr auto kStartUniPort15 = json::MakeKey("start_uni_port_15");
    static constexpr auto kStartUniPort16 = json::MakeKey("start_uni_port_16");
#endif // CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16

    static constexpr json::KeyDefinition kStartUniPort[] = {kStartUniPort1, //
#if (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
                                                            kStartUniPort2, //
                                                            kStartUniPort3, //
                                                            kStartUniPort4, //
                                                            kStartUniPort5, //
                                                            kStartUniPort6, //
                                                            kStartUniPort7, //
                                                            kStartUniPort8, //
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
                                                            kStartUniPort9,  //
                                                            kStartUniPort10, //
                                                            kStartUniPort11, //
                                                            kStartUniPort12, //
                                                            kStartUniPort13, //
                                                            kStartUniPort14, //
                                                            kStartUniPort15, //
                                                            kStartUniPort16
#endif // CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
#endif // (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
    };
};
} // namespace json

#endif // JSON_PIXELDMXPARAMSCONST_H_
