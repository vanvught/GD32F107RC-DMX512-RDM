/**
 * @file flashcode.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <stdio.h>
#include <cstring>
#include <cassert>

#include "flashcode.h"
#include "gd32.h"
#include "firmware/debug/debug_debug.h"

/* Backwards compatibility with SPI FLASH */
static constexpr auto kFlashSectorSize = 4096U;
/* The flash page size is 4KB for bank1 */
static constexpr auto kBanK1FlashPage = (4U * 1024U);

enum class State
{
    IDLE,
    ERASE_BUSY,
    ERASE_PROGAM,
    WRITE_BUSY,
    WRITE_PROGRAM,
    ERROR
};

static State s_state = State::IDLE;
static uint32_t s_page;
static uint32_t s_length;
static uint32_t s_address;
static uint32_t* s_data;

using flashcode::Result;

uint32_t FlashCode::GetSize() const
{
    const auto kFlashDensity = ((REG32(0x1FF0F7E0) >> 16) & 0xFFFF) * 1024U;
    return kFlashDensity;
}

uint32_t FlashCode::GetSectorSize() const
{
    return kFlashSectorSize;
}

bool FlashCode::Read(uint32_t offset, uint32_t length, uint8_t* buffer, Result& result)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", offset, (((uint32_t)(offset) & 0x3) == 0), length, (((uint32_t)(length) & 0x3) == 0), buffer, (((uint32_t)(buffer) & 0x3) == 0));

    const auto* src = reinterpret_cast<uint32_t*>(offset + FLASH_BASE);
    auto* dst = reinterpret_cast<uint32_t*>(buffer);

    while (length > 0)
    {
        *dst++ = *src++;
        length -= 4;
    }

    result = Result::kOk;

    DEBUG_EXIT();
    return true;
}

bool FlashCode::Erase(uint32_t offset, uint32_t length, flashcode::Result& result)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("State=%d", static_cast<int>(s_state));

    result = Result::kOk;

    switch (s_state)
    {
        case State::IDLE:
            s_page = offset + FLASH_BASE;
            s_length = length;
            fmc_unlock();
            s_state = State::ERASE_BUSY;
            DEBUG_EXIT();
            return false;
            break;
        case State::ERASE_BUSY:
            if (SET == fmc_flag_get(FMC_FLAG_BUSY))
            {
                DEBUG_EXIT();
                return false;
            }

            if (s_length == 0)
            {
                s_state = State::IDLE;
                fmc_lock();
                DEBUG_EXIT();
                return true;
            }

            s_state = State::ERASE_PROGAM;
            DEBUG_EXIT();
            return false;
            break;
        case State::ERASE_PROGAM:
            if (s_length > 0)
            {
                DEBUG_PRINTF("s_nPage=%p", s_page);

                fmc_sector_erase(s_page);

                s_length -= kBanK1FlashPage;
                s_page += kBanK1FlashPage;
            }

            s_state = State::ERASE_BUSY;
            DEBUG_EXIT();
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

bool FlashCode::Write(uint32_t offset, uint32_t length, const uint8_t* buffer, flashcode::Result& result)
{
    if ((s_state == State::WRITE_PROGRAM) || (s_state == State::WRITE_BUSY))
    {
    }
    else
    {
        DEBUG_ENTRY();
    }
    result = Result::kOk;

    switch (s_state)
    {
        case State::IDLE:
            DEBUG_PUTS("State::IDLE");
            s_address = offset + FLASH_BASE;
            s_data = const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(buffer));
            s_length = length;
            fmc_unlock();
            s_state = State::WRITE_BUSY;
            DEBUG_EXIT();
            return false;
            break;
        case State::WRITE_BUSY:
            if (SET == fmc_flag_get(FMC_FLAG_BUSY))
            {
                DEBUG_EXIT();
                return false;
            }

            if (s_length == 0)
            {
                fmc_lock();
                s_state = State::IDLE;

                if (memcmp(reinterpret_cast<void*>(offset + FLASH_BASE), buffer, length) == 0)
                {
                    DEBUG_PUTS("memcmp OK");
                }
                else
                {
                    DEBUG_PUTS("memcmp failed");
                }

                DEBUG_EXIT();
                return true;
            }

            s_state = State::WRITE_PROGRAM;
            return false;
            break;
        case State::WRITE_PROGRAM:
            if (s_length >= 4)
            {
                if (FMC_READY == fmc_ready_wait(0xFF))
                {
                    /* set the PG bit to start program */
                    FMC_CTL |= FMC_CTL_PG;
                    __ISB();
                    __DSB();
                    REG32(s_address) = *s_data;
                    __ISB();
                    __DSB();
                    /* reset the PG bit */
                    FMC_CTL &= ~FMC_CTL_PG;
                    s_data++;
                    s_address += 4;
                    s_length -= 4;
                }
            }
            else if (s_length > 0)
            {
                DEBUG_PUTS("Error!");
            }
            s_state = State::WRITE_BUSY;
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
