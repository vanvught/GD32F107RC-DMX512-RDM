/**
 * @file json_config_dmxpixel.cpp
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

#include <cstdint>
#include <string_view>
#include <algorithm>

#include "json/pixeldmxparams.h"
#include "json/json_helpers.h"
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "configstore.h"
#include "configurationstore.h"
#include "json/pixeldmxparamsconst.h"

namespace json::config {
uint32_t GetPixelDmx(char* buffer, uint32_t length) {
    char t[16];

    return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
        const auto kWriteFixed = [&](std::string_view key, float value, uint32_t decimals) {
            const auto* end = common::AppendFixed(t, value, decimals);
            doc[key] = std::string_view{t, static_cast<size_t>(end - t)};
        };

        auto& pixel_configuration = PixelConfiguration::Get();
        doc[DmxLedParamsConst::kType.name] = pixel::GetTypeName(pixel_configuration.GetType());
        doc[DmxLedParamsConst::kCount.name] = pixel_configuration.GetCount();
        kWriteFixed(DmxLedParamsConst::kT0H.name, pixel::ConvertTxH(pixel_configuration.GetLowCode()), 2U);
        kWriteFixed(DmxLedParamsConst::kT1H.name, pixel::ConvertTxH(pixel_configuration.GetHighCode()), 2U);
        doc[DmxLedParamsConst::kMap.name] = pixel::GetMapName(pixel_configuration.GetMap());
        doc[DmxLedParamsConst::kSpiSpeedHz.name] = pixel_configuration.GetClockSpeedHz();
        doc[DmxLedParamsConst::kGlobalBrightness.name] = static_cast<uint32_t>(pixel_configuration.GetGlobalBrightness());
#ifdef CONFIG_PIXELDMX_ENABLE_GAMMATABLE
        doc[DmxLedParamsConst::kGammaCorrection.name] = static_cast<uint32_t>(pixel_configuration.IsEnableGammaCorrection());
        kWriteFixed(DmxLedParamsConst::kGammaValue.name, static_cast<float>(pixel_configuration.GetGammaTableValue()) / 10.0F, 1U);
#endif // CONFIG_PIXELDMX_ENABLE_GAMMATABLE
        auto& pixel_dmx_configuration = PixelDmxConfiguration::Get();
        doc[DmxLedParamsConst::kGroupingCount.name] = pixel_dmx_configuration.GetGroupingCount();
#ifdef OUTPUT_DMX_PIXEL_MULTI
        doc[DmxLedParamsConst::kActiveOutputPorts.name] = pixel_dmx_configuration.GetOutputPorts();
#endif // OUTPUT_DMX_PIXEL_MULTI
#ifdef RDM_RESPONDER
        doc[PixelDmxParamsConst::kDmxStartAddress.name] = static_cast<uint32_t>(pixel_dmx_configuration.GetDmxStartAddress());
#endif // RDM_RESPONDER

        static constexpr uint32_t kConfigMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;
        static const auto kMaxStartUniverses = std::min(kConfigMaxPorts, common::store::dmxled::kMaxUniverses);

        for (uint32_t i = 0; i < kMaxStartUniverses; i++) {
            doc[PixelDmxParamsConst::kStartUniPort[i].name] = static_cast<uint32_t>(ConfigStore::Instance().DmxLedIndexedGetStartUniverse(i));
        }

        doc[DmxLedParamsConst::kTestPattern.name] = static_cast<uint32_t>(PixelTestPattern::Get()->GetPattern());
    });
}

void SetPixelDmx(const char* buffer, uint32_t size) {
    ::json::PixelDmxParams pixel_dmx_params;
    pixel_dmx_params.Store(std::string_view{buffer, size});
    pixel_dmx_params.Set();
}
} // namespace json::config