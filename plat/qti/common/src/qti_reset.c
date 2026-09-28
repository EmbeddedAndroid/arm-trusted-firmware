/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <qti_reset.h>
#include <spmi_arb.h>

#if QTI_RESET2_COOKIE_STORE_ENABLE

#define QTI_RESET2_COOKIE_BUS_ID	0U
#define QTI_RESET2_COOKIE_SLAVE_ID	0U
#define QTI_RESET2_COOKIE_REG		0x7148U

#define SPMI_BUS_ID_PRIMARY		0U
#define SPMI_SLAVE_ID_SHIFT		16U
#define SPMI_SLAVE_ID_MASK		0xfU

static int qti_sdam_write_byte(uint8_t bus_id, uint32_t slave_id,
			       uint16_t addr, uint8_t data)
{
	uint32_t spmi_addr;

	if ((bus_id != SPMI_BUS_ID_PRIMARY) ||
	    ((slave_id & ~SPMI_SLAVE_ID_MASK) != 0U)) {
		return -1;
	}

	spmi_addr = (slave_id << SPMI_SLAVE_ID_SHIFT) | addr;
	return spmi_arb_write8(spmi_addr, data);
}

void qti_reset_write_sdam_cookie(uint8_t cookie)
{
	int ret;

	ret = qti_sdam_write_byte(QTI_RESET2_COOKIE_BUS_ID,
				  QTI_RESET2_COOKIE_SLAVE_ID,
				  QTI_RESET2_COOKIE_REG, cookie);
	if (ret != 0) {
		ERROR("Failed to write reset2 cookie 0x%x: %d\n", cookie, ret);
	}
}

void qti_reset_write_gpio_tcsr(uint32_t val)
{
	if (QTI_RESET2_BOOT_MISC_DETECT_REG != 0U) {
		mmio_write_32(QTI_RESET2_BOOT_MISC_DETECT_REG, val);
	}
}

#endif /* QTI_RESET2_COOKIE_STORE_ENABLE */
