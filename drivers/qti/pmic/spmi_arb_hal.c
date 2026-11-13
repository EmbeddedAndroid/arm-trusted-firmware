/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <spmi_arb.h>
#include <spmi_arb_regs.h>

#include <platform_def.h>

/*
 * addr is a 32-bit value combining (U)SID, PPID and register address:
 *   bits[19:16] = SID, bits[15:8] = PPID, bits[7:0] = register offset,
 * matching the PMIC_ARB_REG_ADDRP_ADDR SID/ADDRESS field layout.
 */
#define ADDR_SID(addr)			(((addr) >> 16) & 0xfU)
#define ADDR_PPID(addr)			(((addr) >> 8) & 0xffU)
#define ADDR_REG_OFFSET(addr)		((addr) & 0xffU)

/*
 * Owner/EE-ID-aware channel resolution. Peripherals may appear on more than
 * one channel (e.g. shared across owners); a channel whose SID/PPID matches
 * is only a provisional match: it must also match want_owner to be
 * accepted. If no SID/PPID-matching channel is also owned by want_owner,
 * this is a hard failure, even if a SID/PPID-only match existed on another
 * owner's channel.
 */
static int addr_to_channel_owner(uint32_t addr, uint8_t want_owner)
{
	uint8_t target_sid = ADDR_SID(addr);
	uint8_t target_ppid = ADDR_PPID(addr);
	int found = -1;
	unsigned int chan;

	for (chan = 0U; chan < SPMI_ARB_NUM_CHANNELS; chan++) {
		uint32_t reg_addr = PMIC_ARB_REG_ADDRP_ADDR(QTI_PMIC_ARB_BASE, chan);
		uint32_t reg = mmio_read_32(reg_addr);
		uint8_t sid, ppid;

		if (reg == 0U) {
			continue;
		}

		sid = (reg & PMIC_ARB_REG_ADDRP_SID_BMSK) >>
			PMIC_ARB_REG_ADDRP_SID_SHFT;
		ppid = (reg & PMIC_ARB_REG_ADDRP_ADDRESS_BMSK) >>
			PMIC_ARB_REG_ADDRP_ADDRESS_SHFT;

		if (sid != target_sid || ppid != target_ppid) {
			continue;
		}

		found = (int)chan;

		uint32_t owner_addr = SPMI_PERIPHM_2OWNER_TABLE_REG_ADDR(QTI_PMIC_ARB_BASE, chan);
		uint32_t owner_reg = mmio_read_32(owner_addr);
		uint8_t owner = (owner_reg &
				 SPMI_PERIPHM_2OWNER_TABLE_REG_PERIPH2OWNER_BMSK) >>
				SPMI_PERIPHM_2OWNER_TABLE_REG_PERIPH2OWNER_SHFT;

		if (owner == want_owner) {
			break;
		}
	}

	if (found < 0) {
		VERBOSE("SPMI_ARB: no SID/PPID match, addr=0x%x chans=%u\n",
			addr, SPMI_ARB_NUM_CHANNELS);
	} else if (chan == SPMI_ARB_NUM_CHANNELS) {
		VERBOSE("SPMI_ARB: no owner match, addr=0x%x chan=%d\n",
			addr, found);
		found = -1;
	}

	return found;
}

static int addr_to_channel(uint32_t addr)
{
	return addr_to_channel_owner(addr, TZ_OWNER_ID);
}

static int wait_for_done(unsigned int chan)
{
	unsigned int timeout = SPMI_ARB_TIMEOUT_ITER;
	uint32_t status_addr = PMIC_ARB_CHNLN_STATUS_ADDR(QTI_PMIC_ARB_BASE, chan);

	while (timeout-- != 0U) {
		uint32_t status = mmio_read_32(status_addr);

		if ((status & PMIC_ARB_CHNLN_STATUS_DONE_BMSK) != 0U) {
			if ((status & (PMIC_ARB_CHNLN_STATUS_FAILURE_BMSK |
				       PMIC_ARB_CHNLN_STATUS_DENIED_BMSK |
				       PMIC_ARB_CHNLN_STATUS_DROPPED_BMSK)) != 0U) {
				return -(int)status;
			}
			return 0;
		}
		udelay(1);
	}

	ERROR("SPMI_ARB timeout!\n");
	return -1;
}

static void issue_command(unsigned int chan, uint8_t opcode, uint32_t addr)
{
	uint32_t cmd_addr = PMIC_ARB_CHNLN_CMD_ADDR(QTI_PMIC_ARB_BASE, chan);
	uint32_t cmd = ((uint32_t)opcode << PMIC_ARB_CHNLN_CMD_OPCODE_SHFT) |
			((ADDR_REG_OFFSET(addr) << PMIC_ARB_CHNLN_CMD_ADDRESS_OFFSET_SHFT) &
			 PMIC_ARB_CHNLN_CMD_ADDRESS_OFFSET_BMSK) |
			0U; /* byte_cnt = bytes - 1 = 0 (1-byte transfer) */

	mmio_write_32(cmd_addr, cmd);
}

int spmi_arb_read8(uint32_t addr)
{
	int chan;
	int ret;
	uint8_t val;
	uint32_t rdata_addr;

	chan = addr_to_channel(addr);
	if (chan < 0) {
		ERROR("SPMI_ARB read: no channel found for addr=0x%x\n", addr);
		return chan;
	}

	issue_command((unsigned int)chan, PMIC_ARB_CMD_EXTENDED_REG_READ_LONG, addr);

	ret = wait_for_done((unsigned int)chan);
	if (ret != 0) {
		ERROR("SPMI_ARB read error [0x%x]: %d\n", addr, ret);
		return ret;
	}

	rdata_addr = PMIC_ARB_CHNLN_RDATA0_ADDR(QTI_PMIC_ARB_BASE, chan);
	val = mmio_read_32(rdata_addr) & 0xffU;

	return (int)val;
}

static int write8_chan(int chan, uint32_t addr, uint8_t data)
{
	int ret;

	mmio_write_32(PMIC_ARB_CHNLN_WDATA0_ADDR(QTI_PMIC_ARB_BASE, chan), data);

	issue_command((unsigned int)chan, PMIC_ARB_CMD_EXTENDED_REG_WRITE_LONG, addr);

	ret = wait_for_done((unsigned int)chan);
	if (ret != 0) {
		ERROR("SPMI_ARB write error [0x%x] = 0x%x: %d\n", addr, data, ret);
	}

	return ret;
}

int spmi_arb_write8(uint32_t addr, uint8_t data)
{
	int chan;

	chan = addr_to_channel(addr);
	if (chan < 0) {
		ERROR("SPMI_ARB write: no channel found for addr=0x%x\n", addr);
		return chan;
	}

	return write8_chan(chan, addr, data);
}

/*
 * Only for the SYSTEM_RESET2 path: PSCI takes SYSTEM_RESET2 only from the
 * normal world, which has stopped its other CPUs, so the HLOS does not use
 * its channel at the same time. Never use a channel of another EE this way;
 * on SM8750 an access through a channel owned by AOP stalls BL31.
 */
int spmi_arb_write8_hlos(uint32_t addr, uint8_t data)
{
	int chan;

	chan = addr_to_channel_owner(addr, HLOS_OWNER_ID);
	if (chan < 0) {
		ERROR("SPMI_ARB write: no HLOS channel for addr=0x%x\n", addr);
		return chan;
	}

	return write8_chan(chan, addr, data);
}
