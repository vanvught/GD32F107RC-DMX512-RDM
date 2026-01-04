/**
 * @file flashcode.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "flashcode.h"
#include "gd32.h"

/**
 * With the latest GD32F firmware, this function is declared as static.
 */
#if defined(GD32F20X)
extern "C"
{
    fmc_state_enum fmc_bank0_state_get(void);
    fmc_state_enum fmc_bank1_state_get(void);
}
#endif

#include "firmware/debug/debug_debug.h"

namespace flashcode
{
/* Backwards compatibility with SPI FLASH */
static constexpr auto kFlashSectorSize = 4096U;
/* The flash page size is 2KB for bank0 */
static constexpr auto kBanK0FlashPage = (2U * 1024U);
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
static bool s_isBank0;
} // namespace flashcode

bool static is_bank0(const uint32_t page_address)
{
    /* flash size is greater than 512k */
    if (FMC_BANK0_SIZE < FMC_SIZE)
    {
        if (FMC_BANK0_END_ADDRESS > page_address)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

using namespace flashcode;

uint32_t FlashCode::GetSize() const
{
    return FMC_SIZE * 1024U;
}

uint32_t FlashCode::GetSectorSize() const
{
    return kFlashSectorSize;
}

bool FlashCode::Read(uint32_t offset, uint32_t length, uint8_t* pBuffer, flashcode::Result& result)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", offset, (((uint32_t)(offset) & 0x3) == 0), length, (((uint32_t)(length) & 0x3) == 0), pBuffer, (((uint32_t)(pBuffer) & 0x3) == 0));

    const auto* pSrc = reinterpret_cast<uint32_t*>(offset + FLASH_BASE);
    auto* pDst = reinterpret_cast<uint32_t*>(pBuffer);

    while (length > 0)
    {
        *pDst++ = *pSrc++;
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
            if ((s_isBank0 = is_bank0(s_page)))
            {
                fmc_bank0_unlock();
            }
            else
            {
                fmc_bank1_unlock();
            }
            s_state = State::ERASE_BUSY;
            DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));
            DEBUG_EXIT();
            return false;
            break;
        case State::ERASE_BUSY:
            if (s_isBank0)
            {
                if (FMC_BUSY == fmc_bank0_state_get())
                {
                    DEBUG_EXIT();
                    return false;
                }
            }
            else
            {
                if (FMC_BUSY == fmc_bank1_state_get())
                {
                    DEBUG_EXIT();
                    return false;
                }
            }

            if (s_isBank0)
            {
                FMC_CTL0 &= ~FMC_CTL0_PER;
            }
            else
            {
                FMC_CTL1 &= ~FMC_CTL1_PER;
            }

            if (s_length == 0)
            {
                if (s_isBank0)
                {
                    fmc_bank0_lock();
                }
                else
                {
                    fmc_bank1_lock();
                }
                s_state = State::IDLE;
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
                DEBUG_PRINTF("s_page=%p", s_page);

                if (s_isBank0)
                {
                    FMC_CTL0 |= FMC_CTL0_PER;
                    FMC_ADDR0 = s_page;
                    FMC_CTL0 |= FMC_CTL0_START;

                    s_length -= kBanK0FlashPage;
                    s_page += kBanK0FlashPage;
                }
                else
                {
                    FMC_CTL1 |= FMC_CTL1_PER;
                    FMC_ADDR1 = s_page;
                    if (FMC_OBSTAT & FMC_OBSTAT_SPC)
                    {
                        FMC_ADDR0 = s_page;
                    }
                    FMC_CTL1 |= FMC_CTL1_START;

                    s_length -= kBanK1FlashPage;
                    s_page += kBanK1FlashPage;
                }
            }

            s_state = State::ERASE_BUSY;
            DEBUG_EXIT();
            return false;
            break;
        case State::WRITE_BUSY:
            if (s_isBank0)
            {
                FMC_CTL0 &= ~FMC_CTL0_PG;
            }
            else
            {
                FMC_CTL1 &= ~FMC_CTL1_PG;
            }
            /*@fallthrough@*/
            /* no break */
        case State::WRITE_PROGRAM:
            s_state = State::IDLE;
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

bool FlashCode::Write(uint32_t offset, uint32_t length, const uint8_t* pBuffer, flashcode::Result& result)
{
    result = Result::kOk;

    switch (s_state)
    {
        case State::IDLE:
            DEBUG_PUTS("State::IDLE");
            s_address = offset + FLASH_BASE;
            s_data = const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(pBuffer));
            s_length = length;
            if ((s_isBank0 = is_bank0(s_address)))
            {
                fmc_bank0_unlock();
            }
            else
            {
                fmc_bank1_unlock();
            }
            s_state = State::WRITE_BUSY;
            DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));
            DEBUG_EXIT();
            return false;
            break;
        case State::WRITE_BUSY:
            if (s_isBank0)
            {
                if (FMC_BUSY == fmc_bank0_state_get())
                {
                    DEBUG_EXIT();
                    return false;
                }
            }
            else
            {
                if (FMC_BUSY == fmc_bank1_state_get())
                {
                    DEBUG_EXIT();
                    return false;
                }
            }

            if (s_isBank0)
            {
                FMC_CTL0 &= ~FMC_CTL0_PG;
            }
            else
            {
                FMC_CTL1 &= ~FMC_CTL1_PG;
            }

            if (s_length == 0)
            {
                if (s_isBank0)
                {
                    fmc_bank0_lock();
                }
                else
                {
                    fmc_bank1_lock();
                }
                s_state = State::IDLE;
                DEBUG_EXIT();
                return true;
            }

            s_state = State::WRITE_PROGRAM;
            return false;
            break;
        case State::WRITE_PROGRAM:
            if (s_length >= 4)
            {
                if (s_isBank0)
                {
                    FMC_CTL0 |= FMC_CTL0_PG;
                }
                else
                {
                    FMC_CTL1 |= FMC_CTL1_PG;
                }
                REG32(s_address) = *s_data;

                s_data++;
                s_address += 4;
                s_length -= 4;
            }
            else if (s_length > 0)
            {
                if (s_isBank0)
                {
                    FMC_CTL0 |= FMC_CTL0_PG;
                }
                else
                {
                    FMC_CTL1 |= FMC_CTL1_PG;
                }
                REG32(s_address) = *s_data;
            }
            s_state = State::WRITE_BUSY;
            return false;
            break;
        case State::ERASE_BUSY:
            if (s_isBank0)
            {
                FMC_CTL0 &= ~FMC_CTL0_PER;
            }
            else
            {
                FMC_CTL1 &= ~FMC_CTL1_PER;
            }
            /*@fallthrough@*/
            /* no break */
        case State::ERASE_PROGAM:
            s_state = State::IDLE;
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
