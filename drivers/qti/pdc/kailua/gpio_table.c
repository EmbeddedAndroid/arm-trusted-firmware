/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/* SM8550 wires its wakeup GPIOs to dedicated PDC inputs, not to the mux. */
struct pdc_gpio_mapping g_pdc_gpio_mapping[] = {};

const uint32_t g_pdc_gpio_mapping_size = ARRAY_SIZE(g_pdc_gpio_mapping);
