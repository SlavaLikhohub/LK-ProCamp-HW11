/*
 * gpio_helpers.c
 *
 *  Created on: Dec 4, 2020
 *      Author: vlykhohub
 */

#include "gpio_helpers.h"

static int button_init(unsigned gpio)
{
	int rc;

	rc = gpio_request(gpio, "Onboard user button");
	if (rc)
		goto err_register;

	rc = gpio_direction_input(gpio);
	if (rc)
		goto err_set_dir;

	return 0;
err_set_dir:
	gpio_free(gpio);
err_register:
	return rc;
}

static void button_deinit(unsigned gpio)
{
	gpio_free(gpio);
}

static int led_init(unsigned gpio)
{
	int rc = gpio_direction_output(gpio, 0);
	return rc;
}

/**
 * init_gpios - initialize button and LED and set LED state to HIGH
 * @button:	GPIO number of a button
 * @led:	GPIO number if a LED
 *
 * Return: Error code (0 for OK)
 */
int init_gpios(unsigned button, unsigned led)
{
	int rc;

	rc = button_init(button);
	if (unlikely(rc)) {
		pr_err("Cannot init GPIO%d as an input\n", button);
		goto err_button_init;
	}
	rc = led_init(led);
	if (rc) {
		pr_err("Cannot init GPIO%d as an output \n", led);
		goto err_led_init;
	}

	gpio_set_value(led, 1);

	return 0;
err_led_init:
	button_deinit(button);
err_button_init:
	return rc;
}

/**
 * deinit_gpios - free button and set LED state to LOW
 * @button:	GPIO number of a button
 * @led:	GPIO number if a LED
 */
void deinit_gpios(unsigned button, unsigned led)
{
	button_deinit(button);
	gpio_set_value(led, 0);
}
