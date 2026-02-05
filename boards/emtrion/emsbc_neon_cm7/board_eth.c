/*
 * Copyright (c) 2025 emtrion GmbH
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_MODULE_NAME eth_emsbc_neon
#define LOG_LEVEL       CONFIG_ETHERNET_LOG_LEVEL

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <zephyr/drivers/gpio.h>
#include "stm32f7xx_hal.h"

#ifdef CONFIG_BOARD_REVISION_R3
#include <zephyr/drivers/eeprom.h>
#endif

/* The devicetree node identifier for the "eth_phy_reset" alias. */
#define ETH_PHY_RESET_NODE DT_ALIAS(eth_phy_reset)

static const struct gpio_dt_spec phy_reset = GPIO_DT_SPEC_GET(ETH_PHY_RESET_NODE, gpios);

#ifdef CONFIG_BOARD_REVISION_R3
/*
 * Get a device structure from a devicetree node with alias eeprom-0
 */
static const struct device *get_eeprom_device(void)
{
	const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(eeprom_0));

	if (!device_is_ready(dev)) {
		LOG_ERR("Error: EEPROM Device \"%s\" is not ready; "
			"check the driver initialization logs for errors.",
			dev->name);
		return NULL;
	}

	LOG_INF("Found EEPROM device \"%s\"\n", dev->name);
	return dev;
}
#endif

void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
	int ret;

#ifdef CONFIG_BOARD_REVISION_R3
	const struct device *eeprom = get_eeprom_device();
	uint8_t ethaddr[6];
	int rc;

	if (eeprom != NULL) {
		rc = eeprom_read(eeprom, 0xFA, &ethaddr[0], sizeof(ethaddr));
		if (rc < 0) {
			LOG_ERR("Cannot read ethernet MAC address from EEPROM");
		} else {
			LOG_DBG("Setting ethernet MAC to %02x:%02x:%02x:%02x:%02x:%02x", ethaddr[0],
				ethaddr[1], ethaddr[2], ethaddr[3], ethaddr[4], ethaddr[5]);
			memcpy(heth->Init.MACAddr, ethaddr, sizeof(ethaddr));
		}
	}
#endif
	LOG_INF("Using ethernet MAC %02x:%02x:%02x:%02x:%02x:%02x", heth->Init.MACAddr[0],
		heth->Init.MACAddr[1], heth->Init.MACAddr[2], heth->Init.MACAddr[3],
		heth->Init.MACAddr[4], heth->Init.MACAddr[5]);

	/* The ethernet phy is enabled by settig the GPIO pin PC0 to 1.
	 * This is done only if the network has been explicitly configured
	 */

	if (!gpio_is_ready_dt(&phy_reset)) {
		return;
	}

	ret = gpio_pin_configure_dt(&phy_reset, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Ethernet Phy reset not available");
		return;
	}
}
