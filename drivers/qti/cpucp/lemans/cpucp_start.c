/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <cpucp.h>
#include <cpucp_hwio.h>
#include <cpucp_shared.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <lib/cassert.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

#define SHARED(member)	(CPUCP_SHARED_DATA_BASE + \
			 offsetof(struct cpucp_shared_data, member))

/* Offsets the firmware expects for the interface version 2 host fields. */
CASSERT(offsetof(struct cpucp_shared_data, soc_info.chip_version) == 0x23cU,
	assert_cpucp_chip_version_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.foundry_id) == 0x240U,
	assert_cpucp_foundry_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.speed_bin) == 0x244U,
	assert_cpucp_speed_bin_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.feature_id) == 0x248U,
	assert_cpucp_feature_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.jtag_id) == 0x24cU,
	assert_cpucp_jtag_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.vp_id) == 0x250U,
	assert_cpucp_vp_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.soft_sku_lval) == 0x254U,
	assert_cpucp_soft_sku_lval_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.local_rev) == 0x264U,
	assert_cpucp_local_rev_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.aging) == 0x265U,
	assert_cpucp_aging_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.targ_volt) == 0x2f6U,
	assert_cpucp_targ_volt_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_offset) == 0x3f6U,
	assert_cpucp_quot_offset_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_vmin) == 0x4f6U,
	assert_cpucp_quot_vmin_offset);
CASSERT(offsetof(struct cpucp_shared_data, acc_info) == 0x5f6U,
	assert_cpucp_acc_info_offset);
CASSERT(sizeof(struct cpucp_acc_info) == 9U, assert_cpucp_acc_info_size);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.addr) == 0x61cU,
	assert_cpucp_mx_cmd_db_addr_offset);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.len) == 0x620U,
	assert_cpucp_mx_cmd_db_len_offset);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.data) == 0x622U,
	assert_cpucp_mx_cmd_db_data_offset);
CASSERT(offsetof(struct cpucp_shared_data, is_slt_plat) == 0x644U,
	assert_cpucp_is_slt_plat_offset);
CASSERT(sizeof(struct cpucp_shared_data) == 0x648U,
	assert_cpucp_shared_data_size);

/* About 1 ms of the 19.2 MHz reference clock. */
#define CPUCP_HANG_THRESHOLD		0x4b1eU

#define GFMUX_SRC_SEL_MASK		GENMASK_32(1, 0)
#define GFMUX_SRC_GPLL0			1U

/* An APSS clock the CPUCP subsystem runs from: GPLL0 divided by div + 1. */
struct apss_clk {
	uintptr_t cdiv;
	unsigned int cdiv_lsb;
	unsigned int cdiv_width;
	uintptr_t gfmux;
	unsigned int div;
};

static const struct apss_clk apss_clks[] = {
	{ APSS_AHB_CLK_CFG,           8U, 4U, APSS_AHB_CLK_CFG,             5U },
	{ APSS_LMH_CDIV_COUNT,        0U, 3U, APSS_LMH_GFMUX_CFG,           5U },
	{ APSS_OSM_CDIV_COUNT,        0U, 3U, APSS_OSM_GFMUX_CFG,           1U },
	{ APSS_PERIPH_CDIV_COUNT,     0U, 4U, APSS_PERIPHCLK_GFMUX_CFG,     1U },
	{ APSS_CL1_PERIPH_CDIV_COUNT, 0U, 4U, APSS_CL1_PERIPHCLK_GFMUX_CFG, 1U },
	{ APSS_IPM_CDIV_COUNT,        0U, 4U, APSS_IPM_GFMUX_CFG,           3U },
};

struct fuse_part {
	uintptr_t reg;
	unsigned int lsb;
	unsigned int width;
};

/* A fused value split over up to two fields, least significant part first. */
struct fuse_field {
	struct fuse_part lo;
	struct fuse_part hi;
};

#define ROW_LSB(n)	QFPROM_CORR_CALIBRATION_ROW_LSB(n)
#define ROW_MSB(n)	QFPROM_CORR_CALIBRATION_ROW_MSB(n)

#define F(reg, lsb, width)	{ { (reg), (lsb), (width) }, { 0U, 0U, 0U } }
#define F2(reg0, lsb0, width0, reg1, lsb1, width1) \
	{ { (reg0), (lsb0), (width0) }, { (reg1), (lsb1), (width1) } }

/*
 * CPR instances 12 to 15 serve, in order, the cluster 0 L3 and gold domains on
 * the APC0 rail and the cluster 1 L3 and gold domains on APC1.
 */
#define FIRST_APSS_CPR		12U
#define NUM_APSS_CPR		4U

/* Open-loop target voltage codes: 8 mV steps in bits [4:0], sign in bit 5. */
static const struct fuse_field
targ_volt_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_LSB(7), 19U, 6U), F(ROW_LSB(7), 13U, 6U),
		F(ROW_LSB(7), 7U, 6U), F(ROW_LSB(7), 1U, 6U),
	}, {
		F(ROW_MSB(7), 11U, 6U), F(ROW_MSB(7), 5U, 6U),
		F2(ROW_LSB(7), 31U, 1U, ROW_MSB(7), 0U, 5U),
		F(ROW_LSB(7), 25U, 6U),
	}, {
		F(ROW_LSB(8), 3U, 6U),
		F2(ROW_MSB(7), 29U, 3U, ROW_LSB(8), 0U, 3U),
		F(ROW_MSB(7), 23U, 6U), F(ROW_MSB(7), 17U, 6U),
	}, {
		F2(ROW_LSB(8), 27U, 5U, ROW_MSB(8), 0U, 1U),
		F(ROW_LSB(8), 21U, 6U), F(ROW_LSB(8), 15U, 6U),
		F(ROW_LSB(8), 9U, 6U),
	},
};

/* Closed-loop quotient offsets, fused in units of 5. */
#define QUOT_OFFSET_STEP	5U

static const struct fuse_field
quot_offset_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_MSB(11), 24U, 7U), F(ROW_MSB(11), 17U, 7U),
		F(ROW_MSB(11), 9U, 8U), F(ROW_MSB(11), 1U, 8U),
	}, {
		F(ROW_LSB(12), 22U, 7U), F(ROW_LSB(12), 15U, 7U),
		F(ROW_LSB(12), 7U, 8U),
		F2(ROW_MSB(11), 31U, 1U, ROW_LSB(12), 0U, 7U),
	}, {
		F(ROW_MSB(12), 20U, 7U), F(ROW_MSB(12), 13U, 7U),
		F(ROW_MSB(12), 5U, 8U),
		F2(ROW_LSB(12), 29U, 3U, ROW_MSB(12), 0U, 5U),
	}, {
		F(ROW_LSB(13), 18U, 7U), F(ROW_LSB(13), 11U, 7U),
		F(ROW_LSB(13), 3U, 8U),
		F2(ROW_MSB(12), 27U, 5U, ROW_LSB(13), 0U, 3U),
	},
};

static const struct fuse_field
quot_vmin_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_LSB(9), 5U, 12U),
		F2(ROW_MSB(8), 25U, 7U, ROW_LSB(9), 0U, 5U),
		F(ROW_MSB(8), 13U, 12U), F(ROW_MSB(8), 1U, 12U),
	}, {
		F2(ROW_MSB(9), 21U, 11U, ROW_LSB(10), 0U, 1U),
		F(ROW_MSB(9), 9U, 12U),
		F2(ROW_LSB(9), 29U, 3U, ROW_MSB(9), 0U, 9U),
		F(ROW_LSB(9), 17U, 12U),
	}, {
		F(ROW_MSB(10), 5U, 12U),
		F2(ROW_LSB(10), 25U, 7U, ROW_MSB(10), 0U, 5U),
		F(ROW_LSB(10), 13U, 12U), F(ROW_LSB(10), 1U, 12U),
	}, {
		F2(ROW_LSB(11), 21U, 11U, ROW_MSB(11), 0U, 1U),
		F(ROW_LSB(11), 9U, 12U),
		F2(ROW_MSB(10), 29U, 3U, ROW_LSB(11), 0U, 9U),
		F(ROW_MSB(10), 17U, 12U),
	},
};

/* Each rail has one aging fuse shared by its CPR instances. */
static const struct fuse_field aging_fuses[NUM_APSS_CPR] = {
	F(ROW_MSB(22), 0U, 8U), F(ROW_MSB(22), 0U, 8U),
	F(ROW_MSB(22), 8U, 8U), F(ROW_MSB(22), 8U, 8U),
};

static uint32_t read_bits(uintptr_t reg, unsigned int lsb, unsigned int width)
{
	return (mmio_read_32(reg) >> lsb) & (BIT_32(width) - 1U);
}

static uint32_t fuse_read(const struct fuse_field *f)
{
	uint32_t val = read_bits(f->lo.reg, f->lo.lsb, f->lo.width);

	if (f->hi.width != 0U) {
		val |= read_bits(f->hi.reg, f->hi.lsb, f->hi.width) <<
		       f->lo.width;
	}

	return val;
}

static int16_t targ_volt_mv(uint32_t code)
{
	int16_t mv = (int16_t)((code & 0x1fU) * 8U);

	return ((code & BIT_32(5)) != 0U) ? (int16_t)-mv : mv;
}

/*
 * A fused frequency cap is passed as the complement of its L value, or as
 * 0xfff when the part has none.
 */
static uint32_t soft_sku_lval(uint32_t enabled, uint32_t lval)
{
	return (enabled != 0U) ? (~lval | 0xffffff00U) : 0xfffU;
}

static uint32_t soc_major_version(void)
{
	return read_bits(QTI_SOC_REVISION_REG, 8U, 8U);
}

static void cpucp_set_clocks(void)
{
	for (unsigned int i = 0U; i < ARRAY_SIZE(apss_clks); i++) {
		const struct apss_clk *clk = &apss_clks[i];

		mmio_clrsetbits_32(clk->cdiv,
				   (BIT_32(clk->cdiv_width) - 1U) <<
				   clk->cdiv_lsb,
				   clk->div << clk->cdiv_lsb);
		mmio_clrsetbits_32(clk->gfmux, GFMUX_SRC_SEL_MASK,
				   GFMUX_SRC_GPLL0);
	}
}

static void cpucp_fill_soc_info(void)
{
	mmio_write_32(SHARED(soc_info.chip_version),
		      (soc_major_version() << 16) |
		      read_bits(QTI_SOC_REVISION_REG, 0U, 8U));
	mmio_write_32(SHARED(soc_info.foundry_id),
		      read_bits(QFPROM_CORR_FEATURE_CONFIG_NM_ROW2_LSB, 6U, 4U));
	mmio_write_32(SHARED(soc_info.speed_bin),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 29U, 3U));
	mmio_write_32(SHARED(soc_info.feature_id),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 20U, 8U));
	mmio_write_32(SHARED(soc_info.jtag_id),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 0U, 20U));
	mmio_write_32(SHARED(soc_info.vp_id),
		      read_bits(QFPROM_CORR_QC_SPARE_20_MSB, 4U, 4U));

	/* The L3 domains have no frequency cap fuse. */
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL0_L3]), 0U);
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL1_L3]), 0U);
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL0_GOLD]),
		      soft_sku_lval(read_bits(SOFT_SKU_APC0_FREQ, 8U, 1U),
				    read_bits(SOFT_SKU_APC0_FREQ, 0U, 8U)));
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL1_GOLD]),
		      soft_sku_lval(read_bits(SOFT_SKU_APC1_FREQ, 8U, 1U),
				    read_bits(SOFT_SKU_APC1_FREQ, 0U, 8U)));
}

static void cpucp_fill_cpr_info(void)
{
	uintptr_t targ_volt = SHARED(cpr_info.targ_volt);
	uintptr_t quot_offset = SHARED(cpr_info.quot_offset);
	uintptr_t quot_vmin = SHARED(cpr_info.quot_vmin);

	mmio_write_8(SHARED(cpr_info.local_rev),
		     (uint8_t)read_bits(ROW_MSB(22), 27U, 3U));

	for (unsigned int i = 0U; i < NUM_APSS_CPR; i++) {
		unsigned int cpr = FIRST_APSS_CPR + i;

		mmio_write_8(SHARED(cpr_info.aging) + cpr,
			     (uint8_t)fuse_read(&aging_fuses[i]));

		for (unsigned int c = 0U; c < CPUCP_NUM_FUSED_CORNERS; c++) {
			uintptr_t idx = ((cpr * CPUCP_NUM_CORNERS) + c) *
					sizeof(uint16_t);

			mmio_write_16(targ_volt + idx, (uint16_t)targ_volt_mv(
				fuse_read(&targ_volt_fuses[i][c])));
			mmio_write_16(quot_offset + idx, (uint16_t)(
				fuse_read(&quot_offset_fuses[i][c]) *
				QUOT_OFFSET_STEP));
			mmio_write_16(quot_vmin + idx,
				      (uint16_t)fuse_read(&quot_vmin_fuses[i][c]));
		}
	}

	for (unsigned int d = CD_CL0_L3; d < CD_MAX; d++) {
		mmio_write_8(SHARED(acc_info[0].acc_fuse) +
			     (d * sizeof(struct cpucp_acc_info)), 0U);
	}
}

/* The firmware votes the MX rail for L3 from the levels in the command DB. */
static int cpucp_fill_mx_levels(void)
{
	uint8_t data[sizeof(((struct cpucp_cmd_db *)NULL)->data)];
	uint8_t len = (uint8_t)sizeof(data);
	uint32_t addr = cmd_db_query_addr("mx.lvl");

	if ((addr == 0U) || (cmd_db_query_aux_data("mx.lvl", &len, data) != 0)) {
		return -1;
	}

	mmio_write_32(SHARED(mx_cmd_db.addr), addr);
	mmio_write_16(SHARED(mx_cmd_db.len), len);
	for (unsigned int i = 0U; i < len; i++) {
		mmio_write_8(SHARED(mx_cmd_db.data) + i, data[i]);
	}

	return 0;
}

void qti_cpucp_init(void)
{
	uint32_t version = mmio_read_32(SHARED(version));

	if (version != CPUCP_SHARED_VERSION) {
		ERROR("CPUCP: unsupported firmware interface %u\n", version);
		return;
	}

	cpucp_set_clocks();

	mmio_write_32(EPSSTOP_MUC_HANG_COUNT_THRESHOLD, CPUCP_HANG_THRESHOLD);
	mmio_setbits_32(EPSSTOP_MUC_HANG_DET_CTRL,
			EPSSTOP_MUC_HANG_DET_CTRL_IRQ_EN);

	cpucp_fill_soc_info();
	cpucp_fill_cpr_info();
	if (cpucp_fill_mx_levels() != 0) {
		ERROR("CPUCP: no MX levels in the command DB\n");
		return;
	}

	/* SLT status unknown: there is no platform info service. */
	mmio_write_32(SHARED(is_slt_plat), (uint32_t)-1);

	/*
	 * The boot firmware opens EPSS to non-secure writes to load CPUCP.
	 * Only version 1 silicon keeps that for kernel L3 voting; later parts
	 * grant it through a dedicated L3 voting permission instead.
	 */
	mmio_clrsetbits_32(EPSSTOP_SECURE_ACCESS_OVERRIDE,
			   EPSSTOP_SECURE_ACCESS_OVERRIDE_EN,
			   (soc_major_version() < 2U) ?
			   EPSSTOP_SECURE_ACCESS_OVERRIDE_EN : 0U);

	mmio_write_32(EPSSTOP_L3_VOTING_EN, 1U);
	mmio_setbits_32(EPSSTOP_GLOBAL_ENABLE, 1U);
	mmio_setbits_32(EPSSFAST_EPSS_MUC_CLK_CTRL,
			EPSS_MUC_CLK_CTRL_CORE_CLK_EN);
}
