# Copyright (c) 2025 emtrion GmbH
# SPDX-License-Identifier: Apache-2.0

zephyr_sources_ifdef(CONFIG_NET_L2_ETHERNET board_eth.c)
