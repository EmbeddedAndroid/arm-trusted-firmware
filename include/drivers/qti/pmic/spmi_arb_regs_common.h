/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_ARB_REGS_COMMON_H
#define SPMI_ARB_REGS_COMMON_H

#include <lib/utils_def.h>

/*
 * Register layout shared by every PMIC ARB revision. Revision-specific base
 * offsets, the REG_ADDRp offset (PMIC_ARB_REG_ADDRP_OFFS) and the
 * per-channel stride (PMIC_ARB_CHNLN_STRIDE) are defined by the including
 * <pmicarbN/spmi_arb_regs.h> before this header is pulled in.
 */

/* Per-channel peripheral identity (SID + PPID) and IRQ owner. */
#define PMIC_ARB_REG_ADDRP_ADDR(base, p) \
	((base) + PMIC_ARB_CORE_REG_BASE_OFFS + PMIC_ARB_REG_ADDRP_OFFS + (0x4UL * (p)))
#define PMIC_ARB_REG_ADDRP_IRQ_OWN_BMSK	0x1000000UL
#define PMIC_ARB_REG_ADDRP_IRQ_OWN_SHFT	24U
#define PMIC_ARB_REG_ADDRP_SID_BMSK		0xf0000UL
#define PMIC_ARB_REG_ADDRP_SID_SHFT		16U
#define PMIC_ARB_REG_ADDRP_ADDRESS_BMSK	0xff00UL
#define PMIC_ARB_REG_ADDRP_ADDRESS_SHFT	8U

/* Per-channel command/status/data registers used to issue a transaction. */
#define PMIC_ARB_CHNLN_CMD_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + (PMIC_ARB_CHNLN_STRIDE * (n)))
#define PMIC_ARB_CHNLN_STATUS_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x8UL + (PMIC_ARB_CHNLN_STRIDE * (n)))
#define PMIC_ARB_CHNLN_WDATA0_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x10UL + (PMIC_ARB_CHNLN_STRIDE * (n)))
#define PMIC_ARB_CHNLN_RDATA0_ADDR(base, n) \
	((base) + PMIC_ARB_CORE_REGISTERS_REG_BASE_OFFS + 0x18UL + (PMIC_ARB_CHNLN_STRIDE * (n)))

#define PMIC_ARB_CHNLN_CMD_OPCODE_SHFT		27U
#define PMIC_ARB_CHNLN_CMD_PRIORITY_SHFT	26U
#define PMIC_ARB_CHNLN_CMD_ADDRESS_OFFSET_BMSK	0xff0UL
#define PMIC_ARB_CHNLN_CMD_ADDRESS_OFFSET_SHFT	4U
#define PMIC_ARB_CHNLN_CMD_BYTE_CNT_BMSK	0x7UL

#define PMIC_ARB_CHNLN_STATUS_DROPPED_BMSK	0x8UL
#define PMIC_ARB_CHNLN_STATUS_DENIED_BMSK	0x4UL
#define PMIC_ARB_CHNLN_STATUS_FAILURE_BMSK	0x2UL
#define PMIC_ARB_CHNLN_STATUS_DONE_BMSK		0x1UL

/* Opcodes for this ARB version's extended-register read/write transactions. */
#define PMIC_ARB_CMD_EXTENDED_REG_WRITE_LONG	0x0U
#define PMIC_ARB_CMD_EXTENDED_REG_READ_LONG	0x1U

/* Peripheral-to-owner table: which EE-id (owner) a periph is assigned to. */
#define SPMI_PERIPHM_2OWNER_TABLE_REG_ADDR(base, m) \
	((base) + SPMI_CFG_REG_BASE_OFFS + (0x4UL * (m)))
#define SPMI_PERIPHM_2OWNER_TABLE_REG_PERIPH2OWNER_BMSK	0xfUL
#define SPMI_PERIPHM_2OWNER_TABLE_REG_PERIPH2OWNER_SHFT	0U

/*
 * TZ's raw EE-id value for owner/EE-ID-aware channel resolution — NOT the
 * SpmiBusCfg_OwnerMask bitmask. Shared across every chipset on every ARB
 * version (all SpmiBlock.c configs use SPMI_BARE_OWNER_NUMBER = 1).
 */
#define TZ_OWNER_ID				1U
/* The HLOS's EE-id in the same table */
#define HLOS_OWNER_ID				0U

/* 1us poll interval; SPMI_TIMEOUT_USEC downstream is 400us. */
#define SPMI_ARB_TIMEOUT_ITER			400U

#endif /* SPMI_ARB_REGS_COMMON_H */
