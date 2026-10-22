/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGATTI_DEF_H
#define AGATTI_DEF_H

#include <common_def.h>
#include <qti_board_def.h>

/* One cluster of four Kryo 2xx Silver cores */
#define MPIDR_PRIMARY_CPU			0x0000

#define PLAT_CLUSTER_COUNT			1
#define PLAT_CLUSTER0_CORE_COUNT		4
#define PLATFORM_CORE_COUNT			PLAT_CLUSTER0_CORE_COUNT

/* System, cluster and core power domains */
#define PLAT_NUM_PWR_DOMAINS			(1 + PLAT_CLUSTER_COUNT + \
						 PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL			2

#define QTI_PWR_LVL0				MPIDR_AFFLVL0
#define QTI_PWR_LVL1				MPIDR_AFFLVL1
#define QTI_PWR_LVL2				MPIDR_AFFLVL2
#define QTI_PWR_LVL3				MPIDR_AFFLVL3

#define QTI_LOCAL_STATE_RUN			0
#define QTI_LOCAL_STATE_STB			1
#define QTI_LOCAL_STATE_RET			2
#define QTI_LOCAL_STATE_OFF			3
#define QTI_LOCAL_STATE_DEEPOFF			4

#define PLAT_MAX_RET_STATE			QTI_LOCAL_STATE_RET
#define PLAT_MAX_OFF_STATE			QTI_LOCAL_STATE_DEEPOFF

#define MAX_MMAP_REGIONS			(PLAT_QTI_MMAP_ENTRIES)
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ull << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ull << 36)

#define ARM_CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE			(1 << ARM_CACHE_WRITEBACK_SHIFT)
#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)

/* GIC-500 */
#define QTI_GICD_BASE				0x0F200000
#define QTI_GICR_BASE				0x0F300000
#define QTI_GICC_BASE				0x0

#define PLAT_INT_ID_CPU_WAKEUP_SGI		0x8
#define PLAT_INT_ID_RESET_SGI			0xF

/* APSS QTimer control base (CNTCTLBase) */
#define QTI_QTIMER_BASE				0x0F120000

/* APSS per-core power controllers, as on MSM8916 */
#define AGATTI_APCS_ALIAS_ACS(core)		(0x0F188000 + (core) * 0x10000)

/* Debug UART: QUPv3 wrap 0, serial engine 4 */
#define PLAT_QTI_UART_BASE			0x04A90000

#define GENI4_CFG				0x0
#define GENI4_IMAGE_REGS			0x100
#define GENI4_DATA				0x600

#define GENI_STATUS_REG				(GENI4_CFG + 0x00000040)
#define GENI_STATUS_M_GENI_CMD_ACTIVE_MASK	(0x1)
#define UART_TX_TRANS_LEN_REG			(GENI4_IMAGE_REGS + 0x00000170)

#define GENI_M_CMD0_REG				(GENI4_DATA + 0x00000000)
#define GENI_TX_FIFOn_REG			(GENI4_DATA + 0x00000100)

#define GENI_M_CMD_TX				(0x08000000)

/* PS_HOLD and the PM4125 PON registers behind the SPMI arbiter */
#define QTI_PS_HOLD_REG				0x0440B000
#define PMIC_ARB_BASE				0x01C00000
#define PON_PS_HOLD_RESET_CTL			0x85A
#define PON_PS_HOLD_RESET_CTL2			0x85B

/* Memory map */
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x10000000 - QTI_DEVICE_BASE)

#define QTI_PIMEM_BASE				0x10000000
#define QTI_PIMEM_LIMIT				0x10500000

#define QTI_SMEM_BASE				0x46000000
#define QTI_SMEM_SIZE				0x00200000

/* TCSR SOC_HW_VERSION */
#define QTI_SOC_VERSION_MASK			0xFFFF
#define QTI_SOC_REVISION_REG			0x003C8000
#define QTI_SOC_REVISION_MASK			0xFFFF

#endif /* AGATTI_DEF_H */
