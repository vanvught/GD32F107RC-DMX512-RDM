/**
 * @file gd32_spi_dma_i2s.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "gd32.h"

#if !defined(SPI_BUFFER_SIZE)
# define SPI_BUFFER_SIZE ((32 * 1024) / 2)
#endif

static uint16_t s_TxBuffer[SPI_BUFFER_SIZE] __attribute__ ((aligned (4)));

#ifndef NDEBUG
/* SPI/I2S parameter initialization mask */
#define SPI_INIT_MASK                   ((uint32_t)0x00003040U)  /*!< SPI parameter initialization mask */
#define I2S_INIT_MASK                   ((uint32_t)0x0000F047U)  /*!< I2S parameter initialization mask */

/* I2S clock source selection, multiplication and division mask */
#define I2S1_CLOCK_SEL                  ((uint32_t)0x00020000U)  /* I2S1 clock source selection */
#define I2S2_CLOCK_SEL                  ((uint32_t)0x00040000U)  /* I2S2 clock source selection */
#define I2S_CLOCK_MUL_MASK              ((uint32_t)0x0000F000U)  /* I2S clock multiplication mask */
#define I2S_CLOCK_DIV_MASK              ((uint32_t)0x000000F0U)  /* I2S clock division mask */

/* default value and offset */
#define SPI_I2SPSC_DEFAULT_VALUE        ((uint32_t)0x00000002U)  /* default value of SPI_I2SPSC register */
#define RCU_CFG1_PREDV1_OFFSET          4U                       /* PREDV1 offset in RCU_CFG1 */
#define RCU_CFG1_PLL2MF_OFFSET          12U                      /* PLL2MF offset in RCU_CFG1 */

extern int uart0_printf(const char* fmt, ...);

static void _i2s_psc_config_dump(uint32_t spi_periph, uint32_t audiosample, uint32_t frameformat, uint32_t mckout) {
	uint32_t i2sdiv = 2U, i2sof = 0U;
	uint32_t clks = 0U;
	uint32_t i2sclock = 0U;

#if defined (GD32F20X_CL)
	/* get the I2S clock source */
	if (SPI1 == ((uint32_t) spi_periph)) {
		/* I2S1 clock source selection */
		clks = I2S1_CLOCK_SEL;
	} else {
		/* I2S2 clock source selection */
		clks = I2S2_CLOCK_SEL;
	}

	if (0U != (RCU_CFG1 & clks)) {
		/* get RCU PLL2 clock multiplication factor */
		clks = (uint32_t) ((RCU_CFG1 & I2S_CLOCK_MUL_MASK)
				>> RCU_CFG1_PLL2MF_OFFSET);

		if ((clks > 5U) && (clks < 15U)) {
			/* multiplier is between 8 and 14 */
			clks += 2U;
		} else {
			if (15U == clks) {
				/* multiplier is 20 */
				clks = 20U;
			}
		}

		/* get the PREDV1 value */
		i2sclock = (uint32_t) (((RCU_CFG1 & I2S_CLOCK_DIV_MASK)
				>> RCU_CFG1_PREDV1_OFFSET) + 1U);
		/* calculate I2S clock based on PLL2 and PREDV1 */
		i2sclock = (uint32_t) ((HXTAL_VALUE / i2sclock) * clks * 2U);
	} else
#endif
	{
		/* get system clock */
		i2sclock = rcu_clock_freq_get(CK_SYS);
	}

	/* config the prescaler depending on the mclk output state, the frame format and audio sample rate */
	if (I2S_MCKOUT_ENABLE == mckout) {
		clks = (uint32_t) (((i2sclock / 256U) * 10U) / audiosample);
	} else {
		if (I2S_FRAMEFORMAT_DT16B_CH16B == frameformat) {
			clks = (uint32_t) (((i2sclock / 32U) * 10U) / audiosample);
		} else {
			clks = (uint32_t) (((i2sclock / 64U) * 10U) / audiosample);
		}
	}

	/* remove the floating point */
	clks = (clks + 5U) / 10U;
	i2sof = (clks & 0x00000001U);
	i2sdiv = ((clks - i2sof) / 2U);
	i2sof = (i2sof << 8U);

	/* set the default values */
	if ((i2sdiv < 2U) || (i2sdiv > 255U)) {
		i2sdiv = 2U;
		i2sof = 0U;
	}

	uart0_printf("clks=%u, i2sclock=%u, i2sof=%u, i2sdiv=%u\n", clks, i2sclock, i2sof, i2sdiv);
}
#endif

static void _spi_i2s_dma_config(void) {
	dma_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA1);

	dma_deinit(SPI_DMAx, SPI_DMA_CHx);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
	dma_init_struct.periph_addr = SPI_PERIPH + 0x0CU;
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(SPI_DMAx, SPI_DMA_CHx, &dma_init_struct);

	dma_circulation_disable(SPI_DMAx, SPI_DMA_CHx);
	dma_memory_to_memory_disable(SPI_DMAx, SPI_DMA_CHx);

	DMA_CHCNT(SPI_DMAx, SPI_DMA_CHx) = 0;
}

void gd32_spi_dma_begin(void) {
	assert(SPI_PERIPH != SPI0);

	rcu_periph_clock_enable(SPI_NSS_RCU_GPIOx);
	rcu_periph_clock_enable(SPI_RCU_GPIOx);
	rcu_periph_clock_enable(SPI_RCU_CLK);
	rcu_periph_clock_enable(RCU_AF);

	gpio_init(SPI_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN);
	gpio_init(SPI_NSS_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_NSS_GPIO_PINx);

#if defined (SPI2_REMAP)
	gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
#else
	if (SPI_PERIPH == SPI2) {
		gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE);
	}
#endif

#if defined (GD32F20X_CL)
	/**
	 * Setup PLL2
	 *
	 * clks=14
	 * i2sclock=160000000
	 * i2sdiv=12, i2sof=256
	 */
	rcu_pll2_config(RCU_PLL2_MUL16);
	RCU_CTL |= RCU_CTL_PLL2EN;
	while ((RCU_CTL & RCU_CTL_PLL2STB) == 0U) {
	}
	if (SPI_PERIPH == SPI2) {
		rcu_i2s2_clock_config(RCU_I2S2SRC_CKPLL2_MUL2);
	} else {
		rcu_i2s1_clock_config(RCU_I2S1SRC_CKPLL2_MUL2);
	}
#endif

	i2s_disable(SPI_PERIPH);
	i2s_psc_config(SPI_PERIPH, 200000, I2S_FRAMEFORMAT_DT16B_CH16B,  I2S_MCKOUT_DISABLE);
	i2s_init(SPI_PERIPH, I2S_MODE_MASTERTX, I2S_STD_MSB, I2S_CKPL_LOW);
	i2s_enable(SPI_PERIPH);

	_spi_i2s_dma_config();

#ifndef NDEBUG
	_i2s_psc_config_dump(SPI_PERIPH, 200000, I2S_FRAMEFORMAT_DT16B_CH16B,  I2S_MCKOUT_DISABLE);
#endif
}

void gd32_spi_dma_set_speed_hz(uint32_t nSpeedHz) {
	const uint32_t audiosample = nSpeedHz / 16 / 2 ;

	i2s_disable(SPI_PERIPH);
	i2s_psc_config(SPI2, audiosample, I2S_FRAMEFORMAT_DT16B_CH16B,  I2S_MCKOUT_DISABLE);
	i2s_enable(SPI_PERIPH);
}

/**
 * DMA
 */

const uint8_t *gd32_spi_dma_tx_prepare(uint32_t *nLength) {
	*nLength = (sizeof(s_TxBuffer) / sizeof(s_TxBuffer[0])) * 2;
	return (const uint8_t *)s_TxBuffer;
}

void gd32_spi_dma_tx_start(const uint8_t *pTxBuffer, uint32_t nLength) {
	assert(((uint32_t)pTxBuffer & 0x1) != 0x1);
//	assert((uint32_t)pTxBuffer >= (uint32_t)s_TxBuffer);
	assert(nLength != 0);

	const uint32_t dma_chcnt = (((nLength + 1) / 2) & DMA_CHANNEL_CNT_MASK);

	uint32_t nDmaChCTL = DMA_CHCTL(SPI_DMAx, SPI_DMA_CHx);
	nDmaChCTL &= ~DMA_CHXCTL_CHEN;
	DMA_CHCTL(SPI_DMAx, SPI_DMA_CHx) = nDmaChCTL;
	DMA_CHMADDR(SPI_DMAx, SPI_DMA_CHx) = (uint32_t)pTxBuffer;
	DMA_CHCNT(SPI_DMAx, SPI_DMA_CHx) = dma_chcnt;
	nDmaChCTL |= DMA_CHXCTL_CHEN;
	DMA_CHCTL(SPI_DMAx, SPI_DMA_CHx) = nDmaChCTL;
	spi_dma_enable(SPI_PERIPH, SPI_DMA_TRANSMIT);
}

bool gd32_spi_dma_tx_is_active(void) {
	return (uint32_t) DMA_CHCNT(SPI_DMAx, SPI_DMA_CHx) != (uint32_t) 0;
}

/**
 * /CS
 */

//TODO Implement /CS
