/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_ARB_REGS_H
#define SPMI_ARB_REGS_H

/*
 * PMIC ARB5 (v5_1) register layout. Offsets below are relative to
 * QTI_PMIC_ARB_BASE, defined in the platform's platform_def.h.
 */

#define PMIC_ARB_CORE_REG_BASE_OFFS		0x00040000UL
#define PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS	0x00200000UL
#define SPMI_CFG_REG_BASE_OFFS			0x0000a700UL
#define PMIC_ARB_REG_ADDRP_OFFS			0x00000900UL
#define PMIC_ARB_CHNLN_STRIDE			0x10000UL

#include <spmi_arb_regs_common.h>

#endif /* SPMI_ARB_REGS_H */
