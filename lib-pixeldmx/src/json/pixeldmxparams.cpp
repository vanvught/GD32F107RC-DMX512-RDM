/**
 * @file pixeldmxparams.cpp
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
#include <utility>

#include "pixeltype.h"
#include "json/pixeldmxparams.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "common/utils/utils_string.h"
#include "pixelconfiguration.h"
#include "pixeldmxconfiguration.h"
#ifdef CONFIG_PIXELDMX_ENABLE_GAMMATABLE
#include "gamma/gamma_tables.h"
#endif // CONFIG_PIXELDMX_ENABLE_GAMMATABLE
#include "dmxnode_outputtype.h"
#include "firmware/pixeldmx/show.h"
#include "dmxnode.h"
#include "dmxnode_nodetype.h"
#include "pixeltestpattern.h"
#include "pixeldmx_debug.h"

static constexpr uint32_t kConfigMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;

using common::store::dmxled::Flags;

namespace json {
PixelDmxParams::PixelDmxParams() {
    ConfigStore::Instance().Copy(&store_dmxled, &ConfigurationStore::dmx_led);
}

void PixelDmxParams::SetType(std::string_view value) {
    if (value.empty()) {
        store_dmxled.type = std::to_underlying(pixel::LedType::kUndefined);
        return;
    }

    store_dmxled.type = std::to_underlying(pixel::GetTypeByName(value));
}

void PixelDmxParams::SetMap(std::string_view value) {
    if (value.empty()) {
        store_dmxled.map = std::to_underlying(pixel::LedMap::kUndefined);
        return;
    }

    store_dmxled.map = std::to_underlying(pixel::GetMapByName(value));
}

void PixelDmxParams::SetCount(std::string_view value) {
    store_dmxled.count = ParseValue<uint16_t>(value);
}

void PixelDmxParams::SetGroupingCount(std::string_view value) {
    store_dmxled.grouping_count = ParseValue<uint16_t>(value);
}

void PixelDmxParams::SetLowCode(std::string_view value) {
    store_dmxled.low_code = pixel::ConvertTxH(common::Atof(value));
}

void PixelDmxParams::SetHighCode(std::string_view value) {
    store_dmxled.high_code = pixel::ConvertTxH(common::Atof(value));
}

#ifdef OUTPUT_DMX_PIXEL_MULTI
void PixelDmxParams::SetActiveOutputs(std::string_view value) {
    store_dmxled.active_outputs = ParseValue<uint8_t>(value);
}
#endif // OUTPUT_DMX_PIXEL_MULTI

void PixelDmxParams::SetTestPattern(std::string_view value) {
    if (value.size() == 1) {
        store_dmxled.test_pattern = ParseValue<uint8_t>(value);
    }
}

void PixelDmxParams::SetSpiSpeedHz(std::string_view value) {
    store_dmxled.spi_speed_hz = ParseValue<uint32_t>(value);
}

void PixelDmxParams::SetGlobalBrightness(std::string_view value) {
    uint8_t global_brightness;

    if (ParseInRange<uint16_t, uint8_t>(value, 0U, 255U, &global_brightness)) {
        store_dmxled.global_brightness = global_brightness;
    }
}

void PixelDmxParams::SetStartUniPort(std::string_view key, std::string_view value) {
    const auto kSuffix = key.back();
    auto index = static_cast<uint32_t>(kSuffix - '1');

    if (key.size() == 17) {
        index += 10;
    }

    store_dmxled.start_universe[index] = ParseValue<uint16_t>(value);
}

#ifdef RDM_RESPONDER
void PixelDmxParams::SetDmxStartAddress(std::string_view value) {
    store_dmxled.dmx_start_address = ParseValue<uint16_t>(value);
}
#endif // RDM_RESPONDER

#ifdef CONFIG_PIXELDMX_ENABLE_GAMMATABLE
void PixelDmxParams::SetGammaCorrection(std::string_view value) {
    if (value.size() == 1) {
        store_dmxled.flags = common::SetFlagValue(store_dmxled.flags, Flags::Flag::kEnableGamma, v[0] != '0');
    }
}

void PixelDmxParams::SetGammaValue(std::string_view value) {
    if ((value.size() == 1) && (value.front() == '0')) {
        store_dmxled.gamma_value = 0;
        return;
    }

    if (value.size() != 3) {
        return;
    }

    const auto kV = gamma::GetValidValue(static_cast<uint32_t>(common::Atof(val, 3) * 10));
    store_dmxled.gamma_value = static_cast<uint8_t>(kV);
}
#endif // CONFIG_PIXELDMX_ENABLE_GAMMATABLE

void PixelDmxParams::Store(std::string_view buffer) {
    ParseJsonWithTable(buffer,kPixelDmxKeys);
    ConfigStore::Instance().Store(&store_dmxled, &ConfigurationStore::dmx_led);

    Dump();
}

void PixelDmxParams::Set() {
    PIXELDMX_DEBUG_ENTRY();
    auto& pixel_configuration = PixelConfiguration::Get();

    pixel_configuration.SetType(static_cast<pixel::LedType>(store_dmxled.type));
    pixel_configuration.SetMap(static_cast<pixel::LedMap>(store_dmxled.map));
    pixel_configuration.SetCount(store_dmxled.count);
    pixel_configuration.SetLowCode(store_dmxled.low_code);
    pixel_configuration.SetHighCode(store_dmxled.high_code);
    pixel_configuration.SetClockSpeedHz(store_dmxled.spi_speed_hz);
#ifdef CONFIG_PIXELDMX_ENABLE_GAMMATABLE
    pixel_configuration.SetEnableGammaCorrection(common::IsFlagSet(store_dmxled.flags, Flags::Flag::kEnableGamma));
    pixel_configuration.SetGammaTable(store_dmxled.gamma_value);
#endif // CONFIG_PIXELDMX_ENABLE_GAMMATABLE
    auto& pixel_dmx_configuration = PixelDmxConfiguration::Get();
    pixel_dmx_configuration.SetGroupingCount(store_dmxled.grouping_count);
#ifdef OUTPUT_DMX_PIXEL_MULTI
    pixel_dmx_configuration.SetOutputPorts(store_dmxled.active_outputs);
#endif // OUTPUT_DMX_PIXEL_MULTI
#ifndef OUTPUT_DMX_PIXEL_MULTI
    pixel_dmx_configuration.SetDmxStartAddress(store_dmxled.dmx_start_address);
#endif // OUTPUT_DMX_PIXEL_MULTI

    DmxPixelOutputType::Get().ApplyConfiguration();

#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
    const auto kUniverses = pixel_dmx_configuration.GetUniverses();
    const auto kPixelOutputPorts = pixel_dmx_configuration.GetOutputPorts();

    uint32_t protocol_port_index = 0;

    for (uint32_t pixel_port_index = 0; pixel_port_index < kPixelOutputPorts; pixel_port_index++) {
        const auto kStartUniverse = ConfigStore::Instance().DmxLedIndexedGetStartUniverse(pixel_port_index);

        for (uint32_t universe = 0; universe < kUniverses; universe++) {
            if (kStartUniverse != 0) {
                DmxNodeNodeType::Get()->SetUniverse(protocol_port_index, static_cast<uint16_t>(kStartUniverse + universe));
                DmxNodeNodeType::Get()->SetDirection(protocol_port_index, dmxnode::Direction::kOutput);

                char label[dmxnode::kPortNameLength];
                snprintf(label, dmxnode::kPortNameLength - 1, "Pixel %c -> %u:%u", static_cast<char>('A' + pixel_port_index), static_cast<unsigned>(protocol_port_index), static_cast<unsigned>(kStartUniverse + universe));
                DmxNode::Instance().SetShortName(protocol_port_index, label);
            }
            protocol_port_index++;
        }
    }

    for (; protocol_port_index < dmxnode::kMaxPorts; protocol_port_index++) {
        DmxNodeNodeType::Get()->SetDirection(protocol_port_index, dmxnode::Direction::kDisable);
        DmxNode::Instance().SetShortNameDefault(protocol_port_index);
    }
#endif // defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)

    if constexpr (kDebugPixelDmxEnabled) {
        pixel_dmx_configuration.Print();
        Dump();
#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
        DmxNodeNodeType::Get()->Print();
#endif // defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
    }

    const auto kTestPattern = static_cast<pixelpatterns::Pattern>(store_dmxled.test_pattern);

    if (kTestPattern != PixelTestPattern::Get()->GetPattern()) {
        const auto kIsSet = PixelTestPattern::Get()->SetPattern(kTestPattern);

        if (kIsSet) {
            PixelOutputType::Get()->Blackout();
#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
            if (static_cast<pixelpatterns::Pattern>(kTestPattern) == pixelpatterns::Pattern::kNone) {
                DmxNodeNodeType::Get()->SetOutput(&DmxNodeOutputType::Get());
            } else {
                DmxNodeNodeType::Get()->SetOutput(nullptr);
            }
#endif // defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
        }
    }

    common::firmware::pixeldmx::Show(7, kTestPattern);

    PIXELDMX_DEBUG_EXIT();
}

void PixelDmxParams::Dump() {
    if constexpr (kDebugPixelDmxEnabled) {
        static const auto kMaxStartUniverses = std::min(kConfigMaxPorts, common::store::dmxled::kMaxUniverses);

        printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxLedParamsConst::kFileName);
        printf(" %s=%s [%u]\n", json::DmxLedParamsConst::kType.name, pixel::GetTypeName(static_cast<pixel::LedType>(store_dmxled.type)), store_dmxled.type);
        printf(" %s=%.2f [0x%X]\n", DmxLedParamsConst::kT0H.name, pixel::ConvertTxH(store_dmxled.low_code), store_dmxled.low_code);
        printf(" %s=%.2f [0x%X]\n", DmxLedParamsConst::kT1H.name, pixel::ConvertTxH(store_dmxled.high_code), store_dmxled.high_code);
        printf(" %s=%s\n", DmxLedParamsConst::kMap.name, pixel::GetMapName(static_cast<pixel::LedMap>(store_dmxled.map)));
        printf(" %s=%u\n", DmxLedParamsConst::kCount.name, static_cast<unsigned>(store_dmxled.count));
        printf(" %s=%u\n", DmxLedParamsConst::kGroupingCount.name, static_cast<unsigned>(store_dmxled.grouping_count));
        for (uint32_t i = 0; i < kMaxStartUniverses; i++) {
            printf(" %s=%d\n", PixelDmxParamsConst::kStartUniPort[i].name, store_dmxled.start_universe[i]);
        }
#ifdef OUTPUT_DMX_PIXEL_MULTI
        printf(" %s=%d\n", DmxLedParamsConst::kActiveOutputPorts.name, store_dmxled.active_outputs);
#endif // OUTPUT_DMX_PIXEL_MULTI
        printf(" %s=%u\n", DmxLedParamsConst::kTestPattern.name, static_cast<unsigned>(store_dmxled.test_pattern));
        printf(" %s=%u\n", DmxLedParamsConst::kSpiSpeedHz.name, static_cast<unsigned>(store_dmxled.spi_speed_hz));
        printf(" %s=%u\n", DmxLedParamsConst::kGlobalBrightness.name, static_cast<unsigned>(store_dmxled.global_brightness));
#ifdef RDM_RESPONDER
        printf(" %s=%u\n", PixelDmxParamsConst::kDmxStartAddress.name, static_cast<unsigned>(store_dmxled.dmx_start_address));
#endif // RDM_RESPONDER
#ifdef CONFIG_PIXELDMX_ENABLE_GAMMATABLE
        printf(" %s=%d\n", DmxLedParamsConst::kGammaCorrection.name, common::IsFlagSet(store_dmxled.flags, Flags::Flag::kEnableGamma));
        printf(" %s=%1.1f [%u]\n", DmxLedParamsConst::kGammaValue.name, static_cast<float>(store_dmxled.gamma_value) / 10.0f, store_dmxled.gamma_value);
#endif // CONFIG_PIXELDMX_ENABLE_GAMMATABLE
    }
}
} // namespace json
