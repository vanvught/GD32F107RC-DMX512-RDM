/**
 * @file is_config_mode.h
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

#ifndef IS_CONFIG_MODE_H_
#define IS_CONFIG_MODE_H_

#include "gd32.h"

bool IsConfigMode()
{
    rcu_periph_clock_enable(KEY1_RCU_GPIOx);
    gpio_init(KEY1_GPIOx, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY1_PINx);
    const auto kIsConfigMode = (gpio_input_bit_get(KEY1_GPIOx, KEY1_PINx) == RESET);

    DEBUG_PRINTF("kIsConfigMode=%s", kIsConfigMode ? "Yes" : "No");

    return kIsConfigMode;
}

#endif  // IS_CONFIG_MODE_H_
