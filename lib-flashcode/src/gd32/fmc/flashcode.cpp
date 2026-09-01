/**
 * @file flashcode.cpp
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
#include <span>
#include <cassert>

#include "flashcode.h"
#include "gd32.h"

/**
 * With the latest GD32F firmware, this function is declared as static.
 */
#if defined(GD32F20X)
extern "C" {
fmc_state_enum fmc_bank0_state_get(void);
fmc_state_enum fmc_bank1_state_get(void);
}
#endif

namespace {
/* Backwards compatibility with SPI FLASH */
constexpr auto kFlashSectorSize = 4096U;
/* The flash page size is 2KB for bank0 */
constexpr auto kBanK0FlashPage = (2U * 1024U);
/* The flash page size is 4KB for bank1 */
constexpr auto kBanK1FlashPage = (4U * 1024U);

enum class State { IDLE, ERASE_BUSY, ERASE_PROGAM, WRITE_BUSY, WRITE_PROGRAM, ERROR };

State s_state = State::IDLE;
uint32_t s_page;
uint32_t s_length;
uint32_t s_address;
const uint32_t* s_data;
bool s_isBank0;

bool IsBank0(uint32_t page_address) {
    // flash size is greater than 512k
    if (FMC_BANK0_SIZE < FMC_SIZE) {
        return FMC_BANK0_END_ADDRESS > page_address;
    }

    return true;
}
} // namespace

uint32_t FlashCode::GetSize() const {
    return FMC_SIZE * 1024U;
}

uint32_t FlashCode::GetSectorSize() const {
    return kFlashSectorSize;
}

bool FlashCode::Read(uint32_t offset, std::span<uint8_t> buffer, flashcode::Result& result) {
    FLASHCODE_DEBUG_ENTRY();
    FLASHCODE_DEBUG_PRINTF("offset=%x, length=%u, data=%p", static_cast<unsigned>(offset), static_cast<unsigned>(buffer.size()), buffer.data());

    const auto* src = reinterpret_cast<const uint32_t*>(offset + FLASH_BASE);
    auto* dst = reinterpret_cast<uint32_t*>(buffer.data());

    auto length = buffer.size();

    while (length >= sizeof(uint32_t)) {
        *dst++ = *src++;
        length -= sizeof(uint32_t);
    }

    result = flashcode::Result::kOk;

    FLASHCODE_DEBUG_EXIT();
    return true;
}

bool FlashCode::Erase(uint32_t offset, uint32_t length, flashcode::Result& result) {
    FLASHCODE_DEBUG_ENTRY();
    FLASHCODE_DEBUG_PRINTF("State=%d", static_cast<int>(s_state));

    result = flashcode::Result::kOk;

    switch (s_state) {
        case State::IDLE:
            s_page = offset + FLASH_BASE;
            s_length = length;
            if ((s_isBank0 = IsBank0(s_page))) {
                fmc_bank0_unlock();
            } else {
                fmc_bank1_unlock();
            }
            s_state = State::ERASE_BUSY;
            FLASHCODE_DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));
            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        case State::ERASE_BUSY:
            if (s_isBank0) {
                if (FMC_BUSY == fmc_bank0_state_get()) {
                    FLASHCODE_DEBUG_EXIT();
                    return false;
                }
            } else {
                if (FMC_BUSY == fmc_bank1_state_get()) {
                    FLASHCODE_DEBUG_EXIT();
                    return false;
                }
            }

            if (s_isBank0) {
                FMC_CTL0 &= ~FMC_CTL0_PER;
            } else {
                FMC_CTL1 &= ~FMC_CTL1_PER;
            }

            if (s_length == 0) {
                if (s_isBank0) {
                    fmc_bank0_lock();
                } else {
                    fmc_bank1_lock();
                }
                s_state = State::IDLE;
                FLASHCODE_DEBUG_EXIT();
                return true;
            }

            s_state = State::ERASE_PROGAM;
            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        case State::ERASE_PROGAM:
            if (s_length > 0) {
                FLASHCODE_DEBUG_PRINTF("s_page=%p", s_page);

                if (s_isBank0) {
                    FMC_CTL0 |= FMC_CTL0_PER;
                    FMC_ADDR0 = s_page;
                    FMC_CTL0 |= FMC_CTL0_START;

                    s_length -= kBanK0FlashPage;
                    s_page += kBanK0FlashPage;
                } else {
                    FMC_CTL1 |= FMC_CTL1_PER;
                    FMC_ADDR1 = s_page;
                    if (FMC_OBSTAT & FMC_OBSTAT_SPC) {
                        FMC_ADDR0 = s_page;
                    }
                    FMC_CTL1 |= FMC_CTL1_START;

                    s_length -= kBanK1FlashPage;
                    s_page += kBanK1FlashPage;
                }
            }

            s_state = State::ERASE_BUSY;
            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        case State::WRITE_BUSY:
            if (s_isBank0) {
                FMC_CTL0 &= ~FMC_CTL0_PG;
            } else {
                FMC_CTL1 &= ~FMC_CTL1_PG;
            }
            /*@fallthrough@*/
            /* no break */
        case State::WRITE_PROGRAM:
            s_state = State::IDLE;
            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }

    assert(0);
    __builtin_unreachable();
    return true;
}

bool FlashCode::Write(uint32_t offset, std::span<const uint8_t> buffer, flashcode::Result& result) {
    result = flashcode::Result::kOk;

    switch (s_state) {
        case State::IDLE:
            FLASHCODE_DEBUG_PUTS("State::IDLE");

            s_address = offset + FLASH_BASE;
            s_data = reinterpret_cast<const uint32_t*>(buffer.data());
            s_length = static_cast<uint32_t>(buffer.size());

            if ((s_isBank0 = IsBank0(s_address))) {
                fmc_bank0_unlock();
            } else {
                fmc_bank1_unlock();
            }

            s_state = State::WRITE_BUSY;

            FLASHCODE_DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));

            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        case State::WRITE_BUSY:
            if (s_isBank0) {
                if (FMC_BUSY == fmc_bank0_state_get()) {
                    FLASHCODE_DEBUG_EXIT();
                    return false;
                }
            } else {
                if (FMC_BUSY == fmc_bank1_state_get()) {
                    FLASHCODE_DEBUG_EXIT();
                    return false;
                }
            }

            if (s_isBank0) {
                FMC_CTL0 &= ~FMC_CTL0_PG;
            } else {
                FMC_CTL1 &= ~FMC_CTL1_PG;
            }

            if (s_length == 0) {
                if (s_isBank0) {
                    fmc_bank0_lock();
                } else {
                    fmc_bank1_lock();
                }
                s_state = State::IDLE;
                FLASHCODE_DEBUG_EXIT();
                return true;
            }

            s_state = State::WRITE_PROGRAM;
            return false;
            break;
        case State::WRITE_PROGRAM:
            if (s_length >= 4) {
                if (s_isBank0) {
                    FMC_CTL0 |= FMC_CTL0_PG;
                } else {
                    FMC_CTL1 |= FMC_CTL1_PG;
                }
                REG32(s_address) = *s_data;

                s_data++;
                s_address += 4;
                s_length -= 4;
            } else if (s_length > 0) {
                if (s_isBank0) {
                    FMC_CTL0 |= FMC_CTL0_PG;
                } else {
                    FMC_CTL1 |= FMC_CTL1_PG;
                }
                REG32(s_address) = *s_data;
            }
            s_state = State::WRITE_BUSY;
            return false;
            break;
        case State::ERASE_BUSY:
            if (s_isBank0) {
                FMC_CTL0 &= ~FMC_CTL0_PER;
            } else {
                FMC_CTL1 &= ~FMC_CTL1_PER;
            }
            /*@fallthrough@*/
            /* no break */
        case State::ERASE_PROGAM:
            s_state = State::IDLE;
            FLASHCODE_DEBUG_EXIT();
            return false;
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }

    assert(0);
    __builtin_unreachable();
    return true;
}
