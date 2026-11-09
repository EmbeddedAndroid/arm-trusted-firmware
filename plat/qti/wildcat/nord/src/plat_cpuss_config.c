/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <qti_plat.h>

/* INTU base = HWIO_APSS_HM_BASE(0x17000000) + 0x840000. */
#define APSS_INTU_BASE			0x17840000U

#define APSS_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x3000U)
#define APSS_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x3400U)
#define APSS_MRG_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x4000U)
#define APSS_MRG_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x4400U)
#define APSS_MRG_DIRECT_SPI_CFG		(APSS_INTU_BASE + 0x5C00U)

/*
 * TCSR_PCIE_LINK_CONFIG_MODE selects how the four PCIe controllers share
 * the 16 lanes of the PCIe PHY. The kernel PHY driver only reads it, and
 * it is not writable from the normal world. Nord boards wire A x8, B x4,
 * C x2, D x2.
 */
#define TCSR_PCIE_LINK_CONFIG_MODE	0x01FD1050U
#define PCIE_LINK_MODE_X8_X4_X2_X2	3U

#define NUM_SPI_WORDS		57U
#define NUM_MERGER_SPI_WORDS	6U
#define NUM_DIRECT_SPI_WORDS	6U

static const uint32_t nordv2_spi_type[NUM_SPI_WORDS] = {
	0x001F8000U, 0x0000A000U, 0x00000000U, 0x0C000220U,
	0x88000000U, 0x1004300CU, 0x00000890U, 0x00140000U,
	0x20608000U, 0x00FC080AU, 0x02000000U, 0x03000000U,
	0x20180000U, 0x00009C13U, 0xE000801FU, 0xE0060000U,
	0x00010000U, 0x00000000U, 0xEA000000U, 0x00000001U,
	0x01823000U, 0x0C000001U, 0x00000100U, 0x00000006U,
	0x08000000U, 0x0C000000U, 0x00048001U, 0x00000000U,
	0x000007F6U, 0x8FC80000U, 0xFFFFFFEFU, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000080U, 0x00000000U, 0x00000000U, 0xDA000500U,
	0x00000002U,
};

static const uint32_t nord_seca_merger_type[NUM_MERGER_SPI_WORDS] = {
	0x00000024U, 0x7F000000U, 0x0FFF0000U, 0x00000000U,
	0x1A000010U, 0xE025B400U,
};

static const uint32_t nord_seca_direct_cfg[NUM_DIRECT_SPI_WORDS] = {
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
};

static void plat_irq_config(void)
{
	configure_irq_type(APSS_CLR_EDGE_BASE, APSS_SET_LEVEL_BASE,
			   nordv2_spi_type, NUM_SPI_WORDS);
	configure_irq_type(APSS_MRG_CLR_EDGE_BASE, APSS_MRG_SET_LEVEL_BASE,
			   nord_seca_merger_type, NUM_MERGER_SPI_WORDS);
	configure_irq_array(APSS_MRG_DIRECT_SPI_CFG, nord_seca_direct_cfg,
			    NUM_DIRECT_SPI_WORDS);
}

static void plat_pcie_link_mode_config(void)
{
	mmio_write_32(TCSR_PCIE_LINK_CONFIG_MODE, PCIE_LINK_MODE_X8_X4_X2_X2);
	INFO("PCIe link mode %u\n", mmio_read_32(TCSR_PCIE_LINK_CONFIG_MODE));
}

void plat_cpuss_config(void)
{
	plat_irq_config();
	plat_pcie_link_mode_config();
}
