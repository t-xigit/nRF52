
#ifndef GPIO_NELLO_ONE_H
#define GPIO_NELLO_ONE_H
#include "nrf_drv_gpiote.h"

/**
 * @brief Function for reconfiguring NFC pins to GPIO pins.
 *
 */


#define LED_1_PIN				12
#define LED_1_MASK (0x0001 << LED_1_PIN)
#define SUPER_CAP_VOLTAGE_PIN	28

/**Defines the output pin for Transistor No. 1.*/
#define BUS_RECEIVE_PIN  	 2
#define BUS_TX_PIN_1		14
#define BUS_TX_PIN_2		15
#define BUS_TX_PIN_3		16
#define BUS_TX_PIN_4		17
#define BUS_TX_PIN_5		18
#define BUS_TX_PIN_6		19
#define BUS_TX_PIN_7		20
#define BUS_TX_PIN_8		13

#define SSRELAY_PIN			24
#define PHOTO_SENSOR_PIN	31
#define RESET_Button_PIN	21
#define BELL_RECEIVE_PIN	 3

/*------- 1+N PINS ----------*/
#define OpN_TX_HI		BUS_TX_PIN_1
#define OpN_TX_MED		BUS_TX_PIN_2
#define OpN_TX_LOW		BUS_TX_PIN_3

/*------- WiFi PINS ---------*/

#define AT_IQRN_PIN			10
#define AT_WAKE_PIN			11
#define AT_CLK_PIN			 7
#define AT_MISO_PIN			 5
#define AT_MOSI_PIN			 6
#define AT_SS_PIN			 4
#define AT_RESET_PIN		 8
#define AT_CHIP_EN_PIN		 9
#define POWER_BOOST_1_PIN	 30
#define POWER_BOOST_2_PIN	 23
#define CHARGE_EN_PIN		29
#define SIEDLE_N_1_PIN		25
#define SIEDLE_N_2_PIN		26
#define SIEDLE_N_3_PIN		27
#define TEST_PIN_1 			22


/*---------------------------*/

#define BUS_TX_PIN_1_PIN_Msk (0x0001 << BUS_TX_PIN_1) /*!< Bit mask of PIN */
#define BUS_TX_PIN_2_PIN_Msk (0x0001 << BUS_TX_PIN_2) /*!< Bit mask of PIN */
#define BUS_TX_PIN_3_PIN_Msk (0x0001 << BUS_TX_PIN_3) /*!< Bit mask of PIN */
#define BUS_TX_PIN_4_PIN_Msk (0x0001 << BUS_TX_PIN_4) /*!< Bit mask of PIN */
#define BUS_TX_PIN_5_PIN_Msk (0x0001 << BUS_TX_PIN_5) /*!< Bit mask of PIN */
#define BUS_TX_PIN_6_PIN_Msk (0x0001 << BUS_TX_PIN_6) /*!< Bit mask of PIN */
#define BUS_TX_PIN_7_PIN_Msk (0x0001 << BUS_TX_PIN_7) /*!< Bit mask of PIN */
#define BUS_TX_PIN_8_PIN_Msk (0x0001 << BUS_TX_PIN_8) /*!< Bit mask of PIN */
#define BUS_TX_PIN_ALL_PIN_Msk 	BUS_TX_PIN_1_PIN_Msk|BUS_TX_PIN_2_PIN_Msk|BUS_TX_PIN_3_PIN_Msk|BUS_TX_PIN_4_PIN_Msk|BUS_TX_PIN_5_PIN_Msk|BUS_TX_PIN_6_PIN_Msk|BUS_TX_PIN_7_PIN_Msk|BUS_TX_PIN_8_PIN_Msk /*!< Bit mask of PIN */

void vInit_GPIO_Pins(void);
void ButtonWatchFunction(void);
void vDeactivate_Door_Open_Relay(void);
void vActivate_Door_Open_Relay(void);
void BellCheckFunction(void);
void gpio_interrupt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

#endif
