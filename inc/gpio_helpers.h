/*
 * gpio_helpers.h
 *
 *  Created on: Dec 4, 2020
 *      Author: vlykhohub
 */

#ifndef GPIO_HELPERS_H_
#define GPIO_HELPERS_H_

#include <linux/gpio.h>

/* On-board LESs
 *  0: D2	GPIO1_21	heartbeat
 *  1: D3	GPIO1_22	uSD access
 *  2: D4	GPIO1_23	active
 *  3: D5	GPIO1_24	MMC access
 *
 * uSD and MMC access LEDs are not used in nfs boot mode, but they are already
 * requested
 * So that, we don't use gpio_request()/gpio_free() here.
 */

/* On-board button.IRQ_HANDLED
 *
 * HDMI interface must be disabled.
 */
#define GPIO_NUMBER(port, bit) (32 * (port) + (bit))
#define BUTTON  GPIO_NUMBER(2, 8)

#define LED_SD  GPIO_NUMBER(1, 22)
#define LED_MMC GPIO_NUMBER(1, 24)
#define LED LED_SD

int init_gpios(unsigned button, unsigned led);
void deinit_gpios(unsigned button, unsigned led);

#endif /* GPIO_HELPERS_H_ */
