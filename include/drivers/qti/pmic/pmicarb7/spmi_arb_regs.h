/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_ARB_REGS_H
#define SPMI_ARB_REGS_H

/*
 * PMIC ARB7 (v7p3) register layout. Offsets below are relative to
 * QTI_PMIC_ARB_BASE, defined in the platform's platform_def.h.
 */

#define PMIC_ARB_CORE_REG_BASE_OFFS		0x1400000UL
#define PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS	0x1500000UL
#define SPMI_CFG_REG_BASE_OFFS			0x142D000UL
#define PMIC_ARB_REG_ADDRP_OFFS			0x2000UL
#define PMIC_ARB_CHNLN_STRIDE			0x1000UL

#include <spmi_arb_regs_common.h>

#endif /* SPMI_ARB_REGS_H */
