/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/qti/pdc/pdc.h>
#include <drivers/qti/pdc/pdc_internal.h>
#include <drivers/qti/pdc/pdc_seq.h>
#include <drivers/qti/pdc/pdc_tcs.h>
#include <drivers/qti/pdc/pdc_types.h>

#include "pdc_regs.h"

extern struct pdc_interrupt_mapping	g_pdc_interrupt_mapping[];
extern const uint32_t			g_pdc_interrupt_table_size;

extern struct pdc_gpio_mapping		g_pdc_gpio_mapping[];
extern const uint32_t			g_pdc_gpio_mapping_size;

static void pdc_set_owner(uint32_t pdc_bit_num, uint32_t num_int,
			  enum pdc_pin_type pin_type, enum pdc_drv_type owner)
{
	uint32_t bit_num = pdc_bit_num;

	if (pin_type == PDC_GPIO) {
		bit_num += num_int;
	}

#if QTI_PDC_IRQ_OWNER_BANKED
	uintptr_t bank = PDC_REG(PDC_BASE, PDC_IRQ_OWNER_BANK_OFF(bit_num / 32U));
	uint32_t mask = BIT_32(bit_num % 32U);

	if (owner == PDC_DRV0) {
		mmio_clrbits_32(bank, mask);
	} else {
		mmio_setbits_32(bank, mask);
		mmio_write_32(PDC_REG(PDC_BASE, PDC_IRQ_NONZERO_DRV_CFG_OFF),
			      (uint32_t)owner);
	}
#else
	PDC_IRQ_OWNER_WRITE(bit_num, (uint32_t)owner);
#endif
}

static void pdc_target_init(void)
{
	uint32_t i;

	for (i = 0U; i < g_pdc_interrupt_table_size; i++) {
		pdc_set_owner(i, g_pdc_interrupt_table_size, PDC_IRQ,
			      g_pdc_interrupt_mapping[i].trig_config.drv_num);
	}

	for (i = 0U; i < g_pdc_gpio_mapping_size; i++) {
		pdc_set_owner(i, g_pdc_interrupt_table_size, PDC_GPIO,
			      g_pdc_gpio_mapping[i].trig_config.drv_num);
	}
}

void qti_pdc_init(void)
{
	pdc_seq_sys_init();
#if QTI_PDC_TCS
	pdc_tcs_initialize();
#endif
	pdc_target_init();
}
