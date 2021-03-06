/**
 *
 * \file
 *
 * \brief This module contains SAMD21 BSP APIs implementation.
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
//#include "asf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"

#include "nrf_drv_gpiote.h"

#include "gpio_one.h"

static tpfNmBspIsr gpfIsr;
static nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

static void chip_isr(void)
{
	if (gpfIsr) {
		gpfIsr();
	}
}

/*
 *	@fn		init_chip_pins
 *	@brief	Initialize reset, chip enable and wake pin
 *	@note 	TY: changes according to Atmel Example
 */
static void init_chip_pins(void)
{
#if 0
	struct port_config pin_conf;

	port_get_config_defaults(&pin_conf);

	/* Configure control pins as output. */
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(CONF_WINC_PIN_RESET, &pin_conf);
	port_pin_set_config(CONF_WINC_PIN_CHIP_ENABLE, &pin_conf);
	port_pin_set_config(CONF_WINC_PIN_WAKE, &pin_conf);
	port_pin_set_output_level(CONF_WINC_PIN_CHIP_ENABLE, false);
	port_pin_set_output_level(CONF_WINC_PIN_RESET, false);
#endif

	nrf_gpio_cfg_output(AT_RESET_PIN);
	nrf_gpio_cfg_output(AT_CHIP_EN_PIN);
	nrf_gpio_cfg_output(AT_WAKE_PIN);
}

/*
 *	@fn		nm_bsp_init
 *	@brief	Initialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_init(void)
{
#if 0
	gpfIsr = NULL;

	/* Initialize chip IOs. */
	init_chip_pins();

    /* Make sure a 1ms Systick is configured. */
    if (!(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk && SysTick->CTRL & SysTick_CTRL_TICKINT_Msk)) {
	    delay_init();
    }

	/* Perform chip reset. */
	nm_bsp_reset();

	system_interrupt_enable_global();
#endif

	gpfIsr = NULL;

	/* Initialize chip IOs. */
	init_chip_pins();

	/* Perform chip reset. */
	nm_bsp_reset();

	//system_interrupt_enable_global();

	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_deinit
 *	@brief	De-iInitialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_deinit(void)
{
#if 0
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	/* Configure control pins as input no pull up. */
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	pin_conf.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_output_level(CONF_WINC_PIN_CHIP_ENABLE, false);
	port_pin_set_output_level(CONF_WINC_PIN_RESET, false);
	port_pin_set_config(CONF_WINC_SPI_INT_PIN, &pin_conf);
#endif

	nrf_gpio_pin_clear(AT_WAKE_PIN);
	nrf_gpio_input_disconnect(AT_MISO_PIN);
	nrf_gpio_pin_clear(AT_MOSI_PIN);
	nrf_gpio_pin_clear(AT_SS_PIN);
	nrf_gpio_pin_clear(AT_CLK_PIN);
	nrf_gpio_pin_clear(AT_RESET_PIN);
	nrf_gpio_input_disconnect(AT_IQRN_PIN);
	nrf_gpio_pin_clear(AT_CHIP_EN_PIN);

	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_reset
 *	@brief	Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_bsp_reset(void)
{
#if 0
	port_pin_set_output_level(CONF_WINC_PIN_CHIP_ENABLE, false);
	port_pin_set_output_level(CONF_WINC_PIN_RESET, false);
	nm_bsp_sleep(100);
	port_pin_set_output_level(CONF_WINC_PIN_CHIP_ENABLE, true);
	nm_bsp_sleep(100);
	port_pin_set_output_level(CONF_WINC_PIN_RESET, true);
	nm_bsp_sleep(100);
#endif

	/* Enable Pin */
	nrf_gpio_pin_clear(AT_CHIP_EN_PIN);
	/* Reset Pin */
	nrf_gpio_pin_clear(AT_RESET_PIN);

	nrf_delay_ms(100);
	nrf_gpio_pin_set(AT_CHIP_EN_PIN);

	nrf_delay_ms(10);
	nrf_gpio_pin_set(AT_RESET_PIN);
	nrf_delay_ms(10);
}

/*
 *	@fn		nm_bsp_sleep
 *	@brief	Sleep in units of mSec
 *	@param[IN]	u32TimeMsec
 *				Time in milliseconds
 *	@todo this delay is blocking at the moment
 */
void nm_bsp_sleep(uint32 u32TimeMsec)
{
        vTaskDelay(u32TimeMsec);

//	while (u32TimeMsec--) {
//		nrf_delay_ms(1);
//	}
}

extern SemaphoreHandle_t m_winc_int_semaphore; /**< Semaphore set in RTC event */

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    BaseType_t yield_req = pdFALSE;
    chip_isr();
    NRF_LOG_DEBUG("WINC INT\n\r")

    UNUSED_VARIABLE(xSemaphoreGiveFromISR(m_winc_int_semaphore, &yield_req));
    portYIELD_FROM_ISR(yield_req);

}

/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(AT_IQRN_PIN, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(AT_IQRN_PIN, true);
}


/*
 *	@fn		nm_bsp_register_isr
 *	@brief	Register interrupt service routine
 *	@param[IN]	pfIsr
 *				Pointer to ISR handler
 *	@todo	since we're not using interrupts this is empty. Should be changed
 */
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
#if 0
	struct extint_chan_conf config_extint_chan;

	gpfIsr = pfIsr;

	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin = CONF_WINC_SPI_INT_PIN;
	config_extint_chan.gpio_pin_mux = CONF_WINC_SPI_INT_MUX;
	config_extint_chan.gpio_pin_pull = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_FALLING;

	extint_chan_set_config(CONF_WINC_SPI_INT_EIC, &config_extint_chan);
	extint_register_callback(chip_isr, CONF_WINC_SPI_INT_EIC,
			EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(CONF_WINC_SPI_INT_EIC,
			EXTINT_CALLBACK_TYPE_DETECT);
#endif

  gpio_init();
  gpfIsr = pfIsr;

}

/*
 *	@fn		nm_bsp_interrupt_ctrl
 *	@brief	Enable/Disable interrupts
 *	@param[IN]	u8Enable
 *				'0' disable interrupts. '1' enable interrupts
 *	@todo	since we're not using interrupts this is empty. Should be changed
 */
void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
#if 0
	if (u8Enable) {
		extint_chan_enable_callback(CONF_WINC_SPI_INT_EIC,
				EXTINT_CALLBACK_TYPE_DETECT);
	} else {
		extint_chan_disable_callback(CONF_WINC_SPI_INT_EIC,
				EXTINT_CALLBACK_TYPE_DETECT);
	}
#endif
#if 1
	if (u8Enable) {
		 nrf_drv_gpiote_in_event_enable(AT_IQRN_PIN, true);
	} else {
		nrf_drv_gpiote_in_event_disable(AT_IQRN_PIN);
	}
#endif
}
